#include <Arduino.h>
#include <SoftwareSerial.h>

// Library containing basic methods for opening/closing circuits or measuring pulses
#include "src/control_system/control_system.h"
// Library containg methods for evaluating the correct spark/fuel timings
#include "src/engine_map/engine_map.h"
// Library containing methods for determining instructions from the computer
#include "src/messages/messages.h"

// Maximum internal temperature of control system allowed, in deg C
#define MAX_TEMP        80
#define TARGET_RPM      1000

#define TACHO_MODULO    1000
#define TIMINGS_TACHO   10
#define TEMP_TACHO      50

#define SPEED_TEST

// Comment
operating_point o;
// Comment
timings t;
// Struct containing information about the state of the engine
engine e;

// A variable counting the microseconds since the last observed pulse
// Used to determine the shaft speed and the crankshaft angle between pulses
unsigned long tacho = 0;

// Counter for the number of IPG pulses between successive CPG pulses
volatile char pulses = 1;
volatile int saved_pulses, saved_crank;

volatile unsigned long current_pulse;
volatile unsigned long last_pulse;

volatile bool cpg_pulsed = false;
volatile bool ipg_pulsed = false;

bool update_temperature = false;
bool update_timings = false;

int prev_estimated_rpm = 0;

size_t buffer = 0;

char message[MESSAGE_SIZE];
bool message_available = false;

bool user_run = false;

void ipg_pulse(void){
    increment_crank(&e, IPG_PULSE_ANGLE);
    pulses++;

    if(pin_state(&(e.cpg))){
        saved_pulses = pulses;
        pulses = 0;

        saved_crank = e.crank;
        cpg_pulsed = true;
    }

    last_pulse = current_pulse;
    current_pulse = micros();

    ipg_pulsed = true;
}

void handle_new_instruction(instr* i){
    switch(i->type){
        case START_CODE:
            if(!e.is_running) user_run = true;
            break;
        case STOP_CODE:
            shutdown_and_print("User-prompted shutdown.\n");
            break;
        case STATUS_CODE:
            get_engine_info(&e, message);
            Serial.println(message);
            get_timing_info(&t, message);
            Serial.println(message);
            break;
        case SET_CODE:
            new_operating_point(i->speed, &o, &t, &e, message);
            Serial.println(message);
    }
}

void shutdown_and_print(char* cause){
    shutdown(&e);

    Serial.println(cause);
    Serial.println("Shutting Down...\n");

    get_engine_info(&e, message);
    Serial.println(message);
}

#ifdef SPEED_TEST
    #define SPEED_TEST_TACHO 20
    float prev_estimated_crank = 0;
    bool update_speed_test = false;

    void print_angle(char* m, float d){
        int diff_integer = (int) d;
        int diff_decimal = (int) 10 * (d - diff_integer);

        sprintf(message, "%s ~%i.%i deg", m, diff_integer, diff_decimal);

        Serial.println(message);
    }
#endif

void setup(void){
    // Open Serial Communication
    Serial.begin(9600);

    // Set the engine parameters
    init_engine(&e);
    init_timings(&t);

    new_operating_point(TARGET_RPM, &o, &t, &e, message);

    attachInterrupt(digitalPinToInterrupt(e.ipg.pin), ipg_pulse, RISING);

    Serial.println("Setup successful.\n");
}

void loop(void){
    if(Serial.available() > 0){
        message[buffer] = Serial.read();
        if(message[buffer] == '\n'){
            message[buffer + 1] = '\0';
            message_available = true;
            buffer = 0;
        } else if(buffer == MESSAGE_SIZE - 1){
            Serial.println("Message is too long.\n");
            buffer = 0;
        } else {
            buffer++;
        }
    } else if(message_available){
        Serial.print(message);
        instr i = get_instruction(message);
        get_instruction_message(&i, message);
        Serial.println(message);
        handle_new_instruction(&i);
        message_available = false;
    }

    if(cpg_pulsed){
        int true_crank = get_true_crank_angle(saved_pulses);

        if(true_crank != -1 && saved_crank != true_crank){
            if(e.is_running){
                shutdown_and_print("CPG and IPG signals don't match.\n");
            } else {
                Serial.println("Correcting crankshaft angle.\n");
                set_crank(&e, true_crank);
            }
        } else if(user_run && t.is_valid){
            Serial.println("Control System is running.\n");
            e.is_running = true;
            user_run = false;
        }

        if(e.crank == 0){
            tacho = (tacho + 1) % TACHO_MODULO;
            if(!(tacho % TIMINGS_TACHO)) update_timings = true;
            if(!(tacho % TEMP_TACHO)) update_temperature = true;

            #ifdef SPEED_TEST
            if(!(tacho % SPEED_TEST_TACHO)){
                update_speed_test = true;
            }
            #endif
        }

        cpg_pulsed = false;
        ipg_pulsed = false;

    } else if(ipg_pulsed){
        unsigned long pulse_width = current_pulse - last_pulse;
        update_velocity(&e, pulse_width);
        ipg_pulsed = false;
    }

    // Set the operating point for the correct timings every revolution of the crank
    if(update_timings){
        update_timings = false;
        int err = set_engine_timings(&t, &o, &e);
    } else if(update_temperature){
        update_temperature = false;
        get_internal_temp(&e);

        if(e.is_running && e.temp > MAX_TEMP){
            shutdown_and_print("Internal temperature exceeded maximum.\n");
        }
    }

    // Estimate the crankshaft angle between pulses using linear interpolation
    float estimated_crank = estimate_angle(&e, current_pulse);

    #ifdef SPEED_TEST
    float angle_difference = estimated_crank - prev_estimated_crank;
    if(angle_difference < 0) angle_difference += 720;

    if(update_speed_test){
        print_angle("Esimated crank:", estimated_crank);
        print_angle("Previous estimated crank:", prev_estimated_crank);
        print_angle("   Loop angle difference:", angle_difference);
        Serial.println();
        update_speed_test = false;
    }

    prev_estimated_crank = estimated_crank;
    #endif
    
    if(e.is_running){
        for(int c = 0; c < 4; c++){
            float a = fmod(estimated_crank + cylinder_phases[c], 720);

            if(should_open_circuit(a, t.spark, &(e.coils[c]))){
                open_circuit(&(e.coils[c]));
            } else if(should_close_circuit(a, t.spark, &(e.coils[c]))){
                close_circuit(&(e.coils[c]));
            }

            if(should_open_circuit(a, t.fuel, &(e.injs[c]))){
                open_circuit(&(e.injs[c]));
            } else if(should_close_circuit(a, t.fuel, &(e.injs[c]))){
                close_circuit(&(e.injs[c]));
            }
        }
    }
}