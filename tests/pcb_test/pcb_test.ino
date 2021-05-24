#include "src/control_system/control_system.h"
#include "src/messages/messages.h"

#include <Arduino.h>
#include <SoftwareSerial.h>

engine e;

size_t buffer = 0;
char message[MESSAGE_SIZE];
bool message_available = false;

unsigned long pulse_width = 0;

unsigned int speed = 0;
bool is_pulsing = false;

pin* target = NULL;

unsigned long counter;

void start_circuit(void){
    Serial.println("Starting pulses.\n");
    if(target && speed != 0){
        is_pulsing = true;
        counter = micros();
    } else {
        Serial.println("Target or speed have not been set!\n");
        Serial.println("Stopping pulses.\n");
        is_pulsing = false;
    }
}

void stop_circuit(void){
    Serial.println("Stopping pulses.\n");

    open_circuit(target);
    is_pulsing = false;
}

void set_circuit(instr* i){
    Serial.println("Setting parameters.\n");
    if(i->speed != -1){
        speed = i->speed;
        pulse_width = pow(10, 6) * 2 * 60 / speed;
    }

    if(i->target){
        target = i->target;
    }
}

void setup(void){
    Serial.begin(9600);

    init_engine(&e);
    e.is_running = true;

    Serial.println("Setup successful\n");
}

void loop(void){
    if(Serial.available() > 0){
        message[buffer] = Serial.read();
        if(message[buffer] == '\n'){
            message[buffer + 1] = '\0';
            message_available = true;
            buffer = 0;
        } else {
            buffer++;
        }
    } else if(message_available){
        Serial.print(message);

        instr i = get_instruction(message, &e);

        get_instruction_message(&i, message);
        Serial.println(message);

        switch(i.type){
            case START_CODE:
                start_circuit();
                break;
            case STOP_CODE:
                stop_circuit();
                break;
            case SET_CODE:
                stop_circuit();
                set_circuit(&i);
                start_circuit();
                break;
            case GET_CODE:
                print_target_value(&i, &e, message);
                Serial.println(message);
        }

        message_available = false;
    }
    
    if(is_pulsing && micros() - counter > pulse_width){
        if(pin_state(target)){
            open_circuit(target);
        } else {
            close_circuit(target);
        }

        counter = micros();
    }
}
