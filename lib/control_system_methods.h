#ifndef CONTROL_SYSTEM_METHODS_H
    #define CONTROL_SYSTEM_METHODS_H

    #include <Arduino.h>

    /*
        Macros defining the pins on the Arduino each 
        sensor/actuator corresponds to.
    */

    #define CRANKSHAFT A0
    #define CAMSHAFT A1
    #define THERMISTOR A5

    #define INJECTOR_1 5
    #define INJECTOR_2 6
    #define INJECTOR_3 7
    #define INJECTOR_4 8

    #define COIL_1 12
    #define COIL_2 11
    #define COIL_3 10
    #define COIL_4 9

    // The change in crankshaft angle between IPG pulses
    #define IPG_PULSE_ANGLE 30

    /*
        The number of IPG pulses between the last and current 
        CPG pulses, based on the camshaft angle.
    */

    #define START_OF_CYCLE_PULSES 12
    #define REFERENCE_PULSES 2
    #define MID_CYCLE_PULSES 10

    // Maximum internal temperature of control system allowed, in degC
    #define MAX_INTERNAL_TEMP 100

    /*
        Definition for engine type. This is a struct that contains important 
        details about the state of the engine, including:

        - The crank angle in degrees.
        - The last state of the CPG and IPG signals.
        - The shaft speed of the engine.
        - The internal temperature of the control system.
        - An array of pointers to the pins controlling each coil and injector.
        - A flag determine whether all engine data is valid, allowing the 
          engine to run fully.
    */
    typedef struct engine {
        int crank, cpg, ipg;
        float speed, cs_temp;

        int coils[4], injs[4];
        int is_valid;
    } engine;

    /* Function prototypes, some of which are yet to be defined */

    void charge_coil(engine* e, int i);
    void discharge_coil(engine* e, int i);

    void open_injector(engine* e, int i);
    void close_injector(engine* e, int i);

    void shutdown(engine* e);

    bool cpg_pulsed(engine* e);
    bool ipg_pulsed(engine* e);

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
    int get_true_crank_angle(int pulses);

    /*
        Method to update the shaft speed of the engine, given the time
        between successive IPG pulses in microseconds. 
        
        This method currently only approximates across a single pulse. 
        However there will be another implementation where the shaft 
        speed is approximated over several pulses.
    */
    float update_velocity(engine* e, unsigned long pulse_width);

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
    float estimate_angle(engine* e, unsigned long last_pulse);
    float internal_temp(engine* e);

#endif
