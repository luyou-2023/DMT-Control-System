#include "src/control_system/control_system.h"
#include "src/messages/messages.h"

#include <Arduino.h>
#include <SoftwareSerial.h>

#define MESSAGE_SIZE (MAX_KEYWORDS * MAX_KEYWORD_LENGTH)

typedef struct circuit {
    unsigned long pulse_width;
    unsigned long speed;
    pin* target;
    bool is_pulsing;
} circuit;

circuit c = {
    .pulse_width = 0,
    .speed = 0,
    .target = NULL
};

engine e;

size_t buffer = 0;
char message[MESSAGE_SIZE] = {0};
bool message_available = false;

unsigned long counter = 0;

void start_circuit(circuit* c){
    if(c->target && c->speed != -1){
        c->is_pulsing = true;
    }
}

void stop_circuit(circuit* c){
    open_circuit(c->target);
    c->is_pulsing = false;
}

void set_circuit(instr* i, circuit* c){
    if(i->speed != -1){
        c->speed = i->speed;
        c->pulse_width = pow(10, 6) * 2 * 60 / c->speed;
    }

    if(i->target){
        c->target = i->target;
    }
}

void get_circuit_value(instr* i, engine* e, char* message){
    char value[20];

    if(!strcmp(i->target->name, THERMISTOR_KEYWORD)){
        sprintf(value, "%i degC", (int) get_internal_temp(e));
    } else {
        sprintf(value, "%s", pin_state(i->target) ? "HIGH" : "LOW");
    }

    sprintf(message, "%s: %s\n", i->target->name, value);
}

void setup(void){
    Serial.begin(9600);
    init_engine(&e);
    e.is_running = true;

    Serial.println("Setup successful");
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

        char readback[150];

        get_instruction_message(&i, readback);
        Serial.println(readback);

        switch(i.type){
            case START_CODE:
                start_circuit(&c);
                break;
            case STOP_CODE:
                stop_circuit(&c);
                break;
            case SET_CODE:
                set_circuit(&i, &c);
                break;
            case GET_CODE:
                get_circuit_value(&i, &e, readback);
                Serial.println(readback);
                break;
            default:
                Serial.println("No change from instruction.");
        }

        counter = micros();
        message_available = false;
    }
    
    if((micros() - counter > c.pulse_width) && c.is_pulsing){
        if(pin_state(c.target) == 1){
            open_circuit(c.target);
        } else {
            close_circuit(c.target);
        }

        counter = micros();
    }

}
