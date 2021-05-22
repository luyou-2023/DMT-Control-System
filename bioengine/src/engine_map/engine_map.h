#ifndef ENGINE_MAP_H
    #define ENGINE_MAP_H

    #include <Arduino.h>

    // Library containing basic methods for opening/closing circuits or measuring pulses
    #include "../control_system/control_system.h"

    // The duration a coil should charge for in crank angle degrees
    // This needs to be replaced with a timer interrupt for greater accuracy
    #define DWELL_TIME_ANGLE 5
    // The crank angle at which fuel injectors will open
    #define FUEL_START_ANGLE 0
    // The size of the fuel/ignition map
    #define MAP_SIZE 7
    // The firing order of the engine cylinders (1-4-2-3)
    #define FIRING_ORDER ((int)[4] {1, 4, 2, 3})

    /*
        Definition of an operating-point type. This contains:
        - The target shaft speed the timings are optimised for
        - The spark angle BTDC at which the coil should discharge
        - The duration the injectors should be open for in crank angle degrees
          (This should be replaced with a timer interrupt for greater accuracy)
    */

    typedef struct operating_point {
        int speed;
        float spark_btdc;
        float inj_duration;
    } operating_point;

    typedef struct timings {
        float spark[4][2];
        float fuel[4][2];
        operating_point o;
    } timings;

    /*
        Global variable defining the map of all the known operating points
        for the CBR600f4i that are optimised for ethanol.
    */

    const operating_point operating_map[] {
        {1000,  7.50,  4.15},
        {2000, 15.00,  9.57},
        {3000, 20.00, 23.45},
        {4000, 23.13, 38.46},
        {5000, 26.25, 44.64},
        {6000, 29.38, 51.24},
        {6250, 32.50, 57.90}
    };

    /* 
        Method for selecting the optimal spark/fuel timing based on the 
        operating map above.

        Given a target RPM, this function will interpolate the values of 
        the operating map to determine the best spark timings. 
        
        These values are saved to p, a pointer to an operating point.
    */
    operating_point select_operating_point(unsigned int target_speed);

    timings get_engine_timings(unsigned int engine_speed, unsigned int target_speed);

    /*
        Method to charge/discharge coils or open/close injectors depending 
        on the current crankshaft angle and the operating point 
        set for the engine.
    */
    void check_engine_timings(float crank, timings* t);

#endif
