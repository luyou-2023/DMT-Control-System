#ifndef ENGINE_MAP_H
    #define ENGINE_MAP_H

    #include <Arduino.h>
    #include <stdio.h>

    // Library containing basic methods for opening/closing circuits or measuring pulses
    #include "../control_system/control_system.h"

    // The duration a coil should charge for in microseconds
    // This needs to be replaced with a timer interrupt for greater accuracy
    #define DWELL_TIME 2000

    // The crank angle at which fuel injectors will open
    #define MIN_FUEL_START_ANGLE 10
    #define MAX_FUEL_END_ANGLE 90

    // The size of the fuel/ignition map
    #define MAP_SIZE 7

    #ifdef __cplusplus
    extern "C" {
    #endif

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
        float spark[2];
        float fuel[2];
        bool is_valid;
        operating_point* o;
    } timings;

    /*
        Global variable defining the map of all the known operating points
        for the CBR600f4i that are optimised for ethanol.
    */

   // The firing order of the engine cylinders (1-4-2-3)
    static const int cylinder_phases[4] = {0, 180, 270, 90};

    static const operating_point operating_map[7] = {
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

    void init_timings(timings* t);

    int set_operating_point(operating_point* o, unsigned int target_speed);

    int set_engine_timings(timings* t, const operating_point* o, const engine* e);

    void get_timing_info(timings* t, char message[150]);

    #ifdef __cplusplus
    }
    #endif

#endif
