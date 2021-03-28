#ifndef CONTROL_SYSTEM_METHODS_H
    #define CONTROL_SYSTEM_METHODS_H

    #include <Arduino.h>

    /*
        Macros defining the pins on the Arduino each 
        sensor/actuator corresponds to.
    */

   typedef struct pin {
       char* reg;
       char num, pin;
   } pin;

    /* Sensor pins: Read-only therefore PIN register */

    #define CRANKSHAFT ((pin) {&PINF, 7, A0})   // Pin A0 (PF7)
    #define CAMSHAFT   ((pin) {&PINF, 6, A1})   // Pin A1 (PF6)

    #define THERMISTOR ((pin) {&PINF, 0, A5})   // Pin A5 (PF0)

    /* Actuator pins: Write-only therefore PORT register */

    #define INJECTOR_1 ((pin) {&PORTC, 6, 5})   // Pin D5 (PC6)
    #define INJECTOR_2 ((pin) {&PORTD, 7, 6})   // Pin D6 (PD7)
    #define INJECTOR_3 ((pin) {&PORTE, 6, 7})   // Pin D7 (PE6)
    #define INJECTOR_4 ((pin) {&PORTB, 4, 8})   // Pin D8 (PB4)

    #define COIL_1 ((pin) {&PORTD, 6, 12})      // Pin D12 (PD6)
    #define COIL_2 ((pin) {&PORTB, 7, 11})      // Pin D11 (PB7)
    #define COIL_3 ((pin) {&PORTB, 6, 10})      // Pin D10 (PB6)
    #define COIL_4 ((pin) {&PORTB, 5, 9})       // Pin D9 (PB5)

    // The change in crankshaft angle between IPG pulses
    #define IPG_PULSE_ANGLE 30

    /*
        The number of IPG pulses between the last and current 
        CPG pulses, based on the camshaft angle.
    */

    #define START_OF_CYCLE_PULSES 12
    #define REFERENCE_PULSES 2
    #define MID_CYCLE_PULSES 10

    #define SUPPLY 5
    #define ADC_MAX 1024

    #define TEMPERATURE(V) ((0.3721 * pow(R, 6)) - )


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
        int crank;
        float speed, temp;
        bool is_running;

        pin coils[4], injs[4];
        pin thermistor, cpg, ipg;

        char _cpg_state;
        char _ipg_state;

        char _prev_cpg_state;
        char _prev_ipg_state;
    } engine;

    /* Function prototypes, some of which are yet to be defined */

    void init_engine(engine* e);

    void shutdown(engine* e);

    void charge_coil(engine* e, char i);
    void discharge_coil(engine* e, char i);

    void open_injector(engine* e, char i);
    void close_injector(engine* e, char i);

    char get_cpg_state(engine* e);
    char get_ipg_state(engine* e);

    float get_internal_temp(engine* e);

    void update_engine_signals(engine* e);

    bool cpg_pulsed(engine* e);
    bool ipg_pulsed(engine* e);

    void set_crank(engine* e, int angle);
    void increment_crank(engine* e, int angle);

    /*
        Method to update the shaft speed of the engine, given the time
        between successive IPG pulses in microseconds. 
        
        This method currently only approximates across a single pulse. 
        However there will be another implementation where the shaft 
        speed is approximated over several pulses.
    */
    void update_velocity(engine* e, unsigned long pulse_width);

    /*
        Method to estimate the crankshaft angle between pulses using 
        linear interpolation of the last known angle, the shaft speed 
        and the time since the last known angle.

        Such that: theta(t) ~ theta_i + (w * (t - t_i))
        
        Where last_pulse is the time recorded of the last known angle in
        microseconds.

        Note that angle change cannot exceed IPG_PULSE_ANGLE, as a new 
        pulse will have been observed.
    */
    float estimate_angle(engine* e, unsigned long last_pulse);

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
    int get_true_crank_angle(char pulses);

#endif
