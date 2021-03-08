#include "control_system_methods.h"

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
int get_true_crank_angle(int pulses){
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

/*
    Method to update the shaft speed of the engine, given the time
    between successive IPG pulses in microseconds. 
    
    This method currently only approximates across a single pulse. 
    However there will be another implementation where the shaft 
    speed is approximated over several pulses.
*/
float update_velocity(engine* e, unsigned long pulse_width){
    return (e->speed = IPG_PULSE_ANGLE / pulse_width);
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
float estimate_angle(engine* e, unsigned long last_pulse){
    float angle_change = e->speed * (micros() - last_pulse);

    return angle_change < IPG_PULSE_ANGLE
        ? e->crank + angle_change
        : e->crank + IPG_PULSE_ANGLE;
}