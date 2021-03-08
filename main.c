#include <Arduino.h>

// Library containing basic methods for opening/closing circuits or measuring pulses
#include "lib/control_system_methods.h"
// Library containg methods for evaluating the correct spark/fuel timings
#include "lib/cbr600f4i_operating_map.h"

// Struct containing information about the state of the engine
engine cbr;
// Struct containing the spark/fuel timings needed to achieve a specific RPM
operating_point p;

// Counter for the number of IPG pulses between successive CPG pulses
unsigned int pulses;
// A variable counting the microseconds since the last observed pulse
// Used to determine the shaft speed and the crankshaft angle between pulses
unsigned long last_pulse;

void setup(void){
    // Set up the pins...

    // Set the operating point for the correct timings
    select_operating_point(1000, &p);
}

void loop(void){
    // If the internal temperature exceeds the maximum
    if(internal_temp(&cbr) >= MAX_INTERNAL_TEMP){
        // Shutdown the engine and exit the program
        shutdown(&cbr);
        exit(0);
    }

    // If the CAM Pulse Generator has returned a new pulse
    if(cpg_pulsed(&cbr)){
        // Use the number of IPG pulses counted since last 
        // CPG pulse to determine what the crank angle should be
        int true_crank = get_true_crank_angle(pulses);
        // If the true crank angle is valid and does not equal the stored value
        if(true_crank != -1 && cbr.crank != true_crank){
            if(e->is_valid){
                // If the engine was fully running, 
                // it should stop as a pulse has been missed
                shutdown();
                exit(0);
            } else {
                // If the system was just setting up, 
                // it now knows the crank angle and can begin running
                e->crank = true_crank;
                e->is_valid = true;
            }
        }
        // Reset the number of IPG pulses since last CPG pulse
        pulses = 0;
    }

    // If the Ignition Pulse Generator has returned a new pulse
    if(ipg_pulsed(&cbr)){
        // Determine the time since the last IPG pulse
        unsigned long pulse_width = micros() - last_pulse;
        // Use this to determine the shaft speed
        update_velocity(&cbr, pulse_width);
        // Increment the crankshaft angle
        cbr.crank = (cbr.crank + IPG_PULSE_ANGLE) % 720;
        // Reset the time since the last pulse
        last_pulse = current_pulse;
        // Increment the pulse counter for CPG
        pulses++;
    }
    // Estimate the crankshaft angle between pulses using 
    // linear interpolation of the time and shaft speed
    float crank = estimate_angle(&cbr, last_pulse);
    // Use this estimation to determine if any of the spark/injection
    // circuits should be changed at this point in the engine cycle
    check_operating_point(crank, p);
}