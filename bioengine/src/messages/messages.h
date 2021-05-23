#ifndef PCB_MESSAGES_H
    #define PCB_MESSAGES_H

    #include <Arduino.h>
    #include <stdlib.h>

    #include "../control_system/control_system.h"
    //#include "../engine_map/engine_map.h"

    #define SPEED_FLAG          "--RPM"

    #define INVALID_KEYWORD     "INVALID"
    #define START_KEYWORD       "START"
    #define STOP_KEYWORD        "STOP"
    #define SET_KEYWORD         "SET"
    #define STATUS_KEYWORD      "STATUS"

    #define INVALID_CODE        0x00
    #define START_CODE          0x01
    #define STOP_CODE           0x02
    #define SET_CODE            0x03
    #define STATUS_CODE         0x04

    #ifdef __cplusplus
    extern "C" {
    #endif

    #define MAX_KEYWORDS        10
    #define MAX_KEYWORD_LENGTH  15

    #define MESSAGE_SIZE (MAX_KEYWORDS * MAX_KEYWORD_LENGTH)

    typedef char keywords[MAX_KEYWORDS][MAX_KEYWORD_LENGTH + 1];

    typedef struct instr {
        int type;
        int speed;
    } instr;

    #define INVALID_INSTR ((instr) {INVALID_CODE, -1})

    instr get_instruction(const char* message, engine* e);

    void get_instruction_message(instr* i, char message[150]);

    void get_engine_info(engine* e, char message[150]);

    void get_timing_info(timings* t, char message[150]);

    #ifdef __cplusplus
    }
    #endif

#endif