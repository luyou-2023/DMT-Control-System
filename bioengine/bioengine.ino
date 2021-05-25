#include <Arduino.h>
#include <SoftwareSerial.h>

// Library containing basic methods for opening/closing circuits or measuring pulses
#include "src/control_system/control_system.h"
// Library containg methods for evaluating the correct spark/fuel timings
#include "src/engine_map/engine_map.h"
// Library containing methods for determining instructions from the computer
#include "src/messages/messages.h"

// Maximum internal temperature of control system allowed, in deg C
#define MAX_TEMP 80
#define INITIAL_TARGET_RPM 1000

#define UPDATE_TIMINGS_CYCLES 1
#define UPDATE_TEMP_CYCLES 50

//#define SPEED_TEST

#ifdef SPEED_TEST
    #define SPEED_TEST_CYL 0
#endif

// Comment
operating_point o;
// Comment
timings t;
// Struct containing information about the state of the engine
engine e;
// Counter for the number of IPG pulses between successive CPG pulses
unsigned int pulses;
// A variable counting the microseconds since the last observed pulse
// Used to determine the shaft speed and the crankshaft angle between pulses
unsigned long last_pulse;
unsigned long tacho = 0;

size_t buffer = 0;
char message[MESSAGE_SIZE];
bool message_available = false;

bool user_run = false;

void print_circuit_change(char* event_name, int cylinder_number, float a, char* message){
    int angle_integer = (int) a;
    int angle_decimal = (int) 10 * (a - angle_integer);

    sprintf(message, "%s %i at %i.%i deg\n", 
        event_name, cylinder_number, angle_integer, angle_decimal);
}

void setup(void){
    // Open Serial Communication
    Serial.begin(9600);

    // Set the engine parameters
    init_engine(&e);
    init_timings(&t);

    set_operating_point(&o, INITIAL_TARGET_RPM);
}

void loop(void){
    if(Serial.available() > 0){
        message[buffer] = Serial.read();
        if(message[buffer] == '\n'){
            message[buffer + 1] = '\0';
            message_available = true;
            buffer = 0;
        } else if(buffer == MESSAGE_SIZE - 1){
            Serial.println("Message is too long.");
            buffer = 0;
        } else {
            buffer++;
        }
    } else if(message_available){
        Serial.print(message);

        instr i = get_instruction(message);

        switch(i.type){
            case START_CODE:
                user_run = true;
                break;
            case STOP_CODE:
                shutdown(&e);
                user_run = false;
                Serial.println("Shutting Down...");
                break;
            case STATUS_CODE:
                get_engine_info(&e, message);
                Serial.println(message);
                get_timing_info(&t, message);
                Serial.println(message);
                break;
            case SET_CODE:
                int err = set_operating_point(&o, i.speed);
                if(err) Serial.println("Could not find target RPM");
                else {
                    err = set_engine_timings(&t, &o, &e);
                    if(err){
                        shutdown(&e);
                        user_run = false;
                        Serial.println("Timings are invalid.");
                        Serial.println("Shutting Down...");
                    }
                }
        }

        get_instruction_message(&i, message);
        Serial.println(message);

        message_available = false;
    }

    update_engine_signals(&e);

    if(cpg_pulsed(&e)){
        int true_crank = get_true_crank_angle(pulses);
        if(true_crank != -1 && e.crank != true_crank){
            // If the engine was running, it should shutdown
            if(e.is_running){
                shutdown(&e);
                Serial.println("Mismatch between CPG and IPG signals.");
                Serial.println("Shutting Down...");
            } else {
                set_crank(&e, true_crank);
                if(user_run && t.is_valid){
                    e.is_running = true;
                    Serial.println("Control System is running.");
                }
            }
        }

        pulses = 0;
    }

    // If the Ignition Pulse Generator has returned a new pulse
    if(ipg_pulsed(&e)){
        unsigned long current_pulse = micros();
        unsigned long pulse_width = current_pulse - last_pulse;

        update_velocity(&e, pulse_width);

        last_pulse = current_pulse;
        pulses++;

        // Increment the crankshaft angle
        increment_crank(&e, IPG_PULSE_ANGLE);
        if(e.crank == 0) tacho++;
    }

    // Set the operating point for the correct timings every revolution of the crank
    if(tacho % UPDATE_TIMINGS_CYCLES == 0){
        set_engine_timings(&t, &o, &e);
    }

    if(tacho % UPDATE_TEMP_CYCLES == 0){
        if(get_internal_temp(&e) > MAX_TEMP){
            shutdown(&e);
            Serial.println("Internal temperature exceeded maximum.");
            Serial.println("Shutting Down...");
        }
    }

    // Estimate the crankshaft angle between pulses using linear interpolation
    float crank = estimate_angle(&e, last_pulse);
    
    if(e.is_running){
        for(int c = 0; c < 4; c++){
            float a = fmod(crank + cylinder_phases[c], 720);
            if(should_open_circuit(a, t.spark, &(e.coils[c]))){
                open_circuit(&(e.coils[c]));
                #ifdef SPEED_TEST
                if(c == SPEED_TEST_CYL){
                    print_circuit_change("DISCHARGE COIL", c, a, message);
                }
                #endif
            } else if(should_close_circuit(a, t.spark, &(e.coils[c]))){
                close_circuit(&(e.coils[c]));
                #ifdef SPEED_TEST
                if(c == SPEED_TEST_CYL){
                    print_circuit_change("CHARGE COIL", c, a, message);
                }
                #endif
            }

            if(should_open_circuit(a, t.fuel, &(e.injs[c]))){
                open_circuit(&(e.injs[c]));
                #ifdef SPEED_TEST
                if(c == SPEED_TEST_CYL){
                    print_circuit_change("CLOSE INJECTOR", c, a, message);
                }
                #endif
            } else if(should_close_circuit(a, t.fuel, &(e.injs[c]))){
                close_circuit(&(e.injs[c]));
                #ifdef SPEED_TEST
                if(c == SPEED_TEST_CYL){
                    print_circuit_change("OPEN INJECTOR", c, a, message);
                }
                #endif
            }
        }
    }
}