#include <Arduino.h>
#include <SoftwareSerial.h>

#include "src/messages/messages.h"

#define IPG_HIGH_ANGLE 15

#define THERMISTOR_VOLTAGE(T) ((float) T * 0.05)

#define SUPPLY 5

const int cpg_pulse_angles[3] = {0, 60, 360};

typedef struct pin {
    char* reg;
    int num, pin;
} pin;

pin temp_pin = {&PORTB, 2, 10};

pin cpg_pin = {&PORTB, 4, 12};
pin ipg_pin = {&PORTB, 5, 13};

size_t buffer = 0;
char message[MESSAGE_SIZE];
bool message_available = false;

unsigned int temp = 0;
unsigned int speed = 0;
unsigned int angle = 0;

unsigned long pulse_width = 0;

bool is_running = false;

unsigned long counter;

char pin_state(pin* target){
    return (*(target->reg) >> target->num) & 1;
}

void open_circuit(pin* target){
    *(target->reg) &= ~(1 << target->num);
}

void close_circuit(pin* target){
    *(target->reg) |= 1 << target->num;
}

void set_temperature_pwm(void){
    int temp_pwm = 255 * THERMISTOR_VOLTAGE(temp) / SUPPLY;
    analogWrite(temp_pin.pin, temp_pwm);
}

void start_simulation(void){
    Serial.println("Starting simulation.\n");
    if(temp > 0 && speed > 0){
        angle = 0;
        is_running = true;
        counter = micros();
        set_temperature_pwm();
    } else {
        Serial.println("Temperature and/or speed have not been set!\n");
        Serial.println("Stopping simulation.\n");
        is_running = false;
    }
}

void stop_simulation(void){
    Serial.println("Stopping simulation.\n");

    open_circuit(&cpg_pin);
    open_circuit(&ipg_pin);
    open_circuit(&temp_pin);

    is_running = false;
}

void set_simulation(instr* i){
    if(i->speed > 0){
        speed = i->speed;
        float pulses_per_second = ((360 / IPG_HIGH_ANGLE) * speed / 60);
        pulse_width = pow(10, 6) / pulses_per_second;
    }

    if(i->temp > 0){
        temp = i->temp;

        if(is_running){
            set_temperature_pwm();
        }
    }
}

void print_simulator_info(char* message){
    float voltage = THERMISTOR_VOLTAGE((float) temp);
    int voltage_integer = (int) voltage;
    int voltage_decimal = (voltage - voltage_integer) * 10;

    sprintf(message, 
        "simulator info:\n    is running: %s\n    temp: %u deg C (~%i.%i V)\n    speed: %u RPM\n    pulse width: %lu us\n",
        is_running ? "true" : "false", temp, voltage_integer, voltage_decimal, speed, pulse_width);
}

void setup(void){
    Serial.begin(9600);

    pinMode(temp_pin.pin, OUTPUT);
    pinMode(cpg_pin.pin, OUTPUT);
    pinMode(ipg_pin.pin, OUTPUT);

    Serial.println("Setup successful.\n");
}

void loop(void){
    if(Serial.available() > 0){
        message[buffer] = Serial.read();
        if(message[buffer] == '\n'){
            message[buffer + 1] = '\0';
            message_available = true;
            buffer = 0;
        } else if(buffer == MESSAGE_SIZE - 1){
            Serial.println("Message is too long.\nTry again.\n");
            buffer = 0;
        } else {
            buffer++;
        }
    } else if(message_available){
        Serial.print(message);

        instr i = get_instruction(message);

        get_instruction_message(&i, message);
        Serial.println(message);

        switch(i.type){
            case START_CODE:
                start_simulation();
                break;
            case STOP_CODE:
                stop_simulation();
                break;
            case STATUS_CODE:
                print_simulator_info(message);
                Serial.println(message);
                break;
            case SET_CODE:
                set_simulation(&i);
        }

        message_available = false;
    }

    if(is_running && micros() - counter > pulse_width){
        counter = micros();

        angle = (angle + IPG_HIGH_ANGLE) % 720;

        if(angle % (2 * IPG_HIGH_ANGLE) == 0){
            close_circuit(&ipg_pin);
        } else {
            open_circuit(&ipg_pin);
        }

        if(pin_state(&cpg_pin)){
            open_circuit(&cpg_pin);
        } else {
            for(size_t i = 0; i < 3; i++){
                if(angle == cpg_pulse_angles[i]){
                    close_circuit(&cpg_pin);
                }
            }
        }
    }
}