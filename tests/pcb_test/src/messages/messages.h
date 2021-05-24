#ifndef PCB_TEST_MESSAGES_H
    #define PCB_TEST_MESSAGES_H

    #include <Arduino.h>
    #include <stdlib.h>

    #include "../control_system/control_system.h"

    #define TARGET_FLAG         "--TARGET"
    #define SPEED_FLAG          "--SPEED"

    #define INJECTOR_KEYWORD    "INJECTOR"
    #define COIL_KEYWORD        "COIL"
    #define THERMISTOR_KEYWORD  "THERMISTOR"
    #define CAMSHAFT_KEYWORD    "CAMSHAFT"
    #define CRANKSHAFT_KEYWORD  "CRANKSHAFT"

    #define INVALID_KEYWORD     "INVALID"
    #define START_KEYWORD       "START"
    #define STOP_KEYWORD        "STOP"
    #define SET_KEYWORD         "SET"
    #define GET_KEYWORD         "GET"

    #define INVALID_CODE        0x00
    #define START_CODE          0x01
    #define STOP_CODE           0x02
    #define SET_CODE            0x03
    #define GET_CODE            0x04

    #ifdef __cplusplus
    extern "C" {
    #endif

    #define MAX_KEYWORDS        10
    #define MAX_KEYWORD_LENGTH  15

    #define MESSAGE_SIZE (MAX_KEYWORDS * MAX_KEYWORD_LENGTH)

    typedef char keywords[MAX_KEYWORDS][MAX_KEYWORD_LENGTH + 1];

    typedef struct instr {
        char type;
        pin* target;
        int speed;
    } instr;

    #define INVALID_INSTR ((instr) {INVALID_CODE, NULL, -1})

    instr get_instruction(const char* message, engine* e);

    void get_instruction_message(instr* i, char message[150]);

    void print_target_value(instr* i, engine* e, char* message);

    #ifdef __cplusplus
    }
    #endif

#endif