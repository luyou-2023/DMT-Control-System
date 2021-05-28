#include "control_system.h"

void init_pin_modes(engine* e){
    for(size_t i = 0; i < 4; i++){
        pinMode(e->coils[i].pin, OUTPUT);
        pinMode(e->injs[i].pin, OUTPUT);
    }

    pinMode(e->thermistor.pin, INPUT);
    pinMode(e->cpg.pin, INPUT);
    pinMode(e->ipg.pin, INPUT);
}


void init_engine(engine* e){
    if(!e) return;

    e->coils[0] = COIL_1;
    e->coils[1] = COIL_2;
    e->coils[2] = COIL_3;
    e->coils[3] = COIL_4;

    e->injs[0] = INJECTOR_1;
    e->injs[1] = INJECTOR_2;
    e->injs[2] = INJECTOR_3;
    e->injs[3] = INJECTOR_4;

    e->thermistor = THERMISTOR;

    e->cpg = CAMSHAFT;
    e->ipg = CRANKSHAFT;

    init_pin_modes(e);

    e->crank = 0;

    e->speed = 0;
    e->rpm = 0;

    e->temp = get_internal_temp(e);

    e->is_running = false;
}

void shutdown(engine* e){
    for(size_t i = 0; i < 4; i++){
        open_circuit(&(e->injs[i]));
        open_circuit(&(e->coils[i]));
    }

    e->is_running = false;
}

char pin_state(pin* target){
    return (*(target->reg) >> target->num) & 1;
}

void open_circuit(pin* target){
    *(target->reg) &= ~(1 << target->num);
}

void close_circuit(pin* target){
    *(target->reg) |= 1 << target->num;
}


int get_internal_temp(engine* e){
    float pin_voltage = (float) SUPPLY * analogRead(e->thermistor.pin) / ADC_MAX;
    e->temp = (int) TEMPERATURE((float) pin_voltage);
    return e->temp;
}

void set_crank(engine* e, int angle){
    e->crank = angle % 720;
}

void increment_crank(engine* e, int angle){
    e->crank = (e->crank + angle) % 720;
}

/*
    Method to update the shaft speed of the engine, given the time
    between successive IPG pulses in microseconds. 
    
    This method currently only approximates across a single pulse. 
    However there will be another implementation where the shaft 
    speed is approximated over several pulses.
*/
void update_velocity(engine* e, unsigned long pulse_width){
    e->speed = (float) IPG_PULSE_ANGLE / pulse_width;
    e->rpm = pow(10, 6) * e->speed * 60 / 360;
}

/*
    Method to estimate the crankshaft angle between pulses using 
    linear interpolation of the last known angle, the shaft speed 
    and the time since the last known angle.

    Such that: theta(t) ~= theta_i + (w * (t - t_i))
    
    Where last_pulse is the time recorded of the last known angle in
    microseconds.

    Note that angle change cannot exceed IPG_PULSE_ANGLE, as a new 
    pulse will have been observed.
*/
float estimate_angle(engine* e, unsigned long current_pulse){
    if(!e) return -1;

    return e->crank + (e->speed * (micros() - current_pulse));
    /*
    return angle_change < IPG_PULSE_ANGLE
        ? e->crank + angle_change
        : e->crank + IPG_PULSE_ANGLE;
    */
}

/*
    Method to return the true crankshaft angle based on the number 
    of IPG pulses between successive CPG pulses.
    
    The CPG sends three pulses per cycle:
    - At the start of the cycle, when cylinder 1 is at TDC 
      before the intake stroke.
    - A reference pulse shortly after the start of the cycle.
    - At the midpoint of the cycle, when cylinder 1 reaches
      TDC before the power stroke.

    If the number of pulses found does not match any of these, 
    the function will return -1.
*/
int get_true_crank_angle(char pulses){
    switch(pulses){
        case START_OF_CYCLE_PULSES:
            return 0;
        case REFERENCE_PULSES:
            return 60;
        case MID_CYCLE_PULSES:
            return 360;
        default:
            return -1;
    }
}

bool within_interval(float angle, float bounds[2]){
    return angle < bounds[1] && angle > bounds[0];
}

bool should_open_circuit(float angle, float bounds[2], pin* p){
    return pin_state(p) && !within_interval(angle, bounds);
}

bool should_close_circuit(float angle, float bounds[2], pin* p){
    return !pin_state(p) && within_interval(angle, bounds);
}

void get_engine_info(engine* e, char message[150]){
    sprintf(message, "engine status:\n    crank angle: %i deg\n    speed: %i RPM\n    temp: %i deg C\n    is running: %s\n", 
        e->crank, e->rpm, e->temp, e->is_running ? "true" : "false");
}