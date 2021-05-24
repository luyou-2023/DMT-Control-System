#ifndef ENGINE_SIMULATOR_MESSAGES_H
    #define ENGINE_SIMULATOR_MESSAGES_H

    #include <Arduino.h>
    #include <stdlib.h>
    #include <stdio.h>

    #define TEMP_FLAG           "--TEMP"
    #define SPEED_FLAG          "--SPEED"

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
        char type;
        int speed, temp;
    } instr;

    #define INVALID_INSTR ((instr) {INVALID_CODE, -1, -1})

    instr get_instruction(const char* message);

    void get_instruction_message(instr* i, char message[150]);

    #ifdef __cplusplus
    }
    #endif

#endif