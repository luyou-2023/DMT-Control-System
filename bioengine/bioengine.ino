#include <Arduino.h>
// Library containing basic methods for opening/closing circuits or measuring pulses
#include "libraries/control_system/control_system.h"
// Library containg methods for evaluating the correct spark/fuel timings
#include "libraries/engine_map/engine_map.h"

// Maximum internal temperature of control system allowed, in deg C
#define MAX_INTERNAL_TEMP 100
#define TARGET_RPM 1000

// Struct containing information about the state of the engine
engine e;
// Counter for the number of IPG pulses between successive CPG pulses
unsigned int pulses;
// A variable counting the microseconds since the last observed pulse
// Used to determine the shaft speed and the crankshaft angle between pulses
unsigned long last_pulse;

void setup(void){
    // Set the e parameters
    init_engine(&e);
}

void loop(void){
    update_engine_signals(&e);

    // If the internal temperature exceeds the maximum
    if(e.temp > MAX_INTERNAL_TEMP){
        shutdown(&e);
    }

    if(cpg_pulsed(&e)){
        int true_crank = get_true_crank_angle(pulses);

        if(true_crank != -1 && e.crank != true_crank){
            // If the engine was running, it should shutdown
            if(e.is_running) shutdown(&e);
            // If the engine was setting up, now it can start running
            else set_crank(&e, true_crank);
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
    }

    // Set the operating point for the correct timings
    timings t = get_engine_timings(&e, TARGET_RPM);
    // Estimate the crankshaft angle between pulses using linear interpolation
    float crank = estimate_angle(&e, last_pulse);
    check_engine_timings(crank, &t);
}