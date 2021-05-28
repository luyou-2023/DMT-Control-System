#include <Arduino.h>
#include <SoftwareSerial.h>

#include "src/messages/messages.h"

#define IPG_HIGH_ANGLE 15

#define REGISTER PORTB

#define TEMP_ADDRESS 2
#define CPG_ADDRESS 4
#define IPG_ADDRESS 5

#define TEMP_PIN 10
#define CPG_PIN 12
#define IPG_PIN 13

#define THERMISTOR_VOLTAGE(T) \
    - (2 * pow(10, -12) * pow(T, 6)) \
    + (6 * pow(10, -10) * pow(T, 5)) \
    - (3 * pow(10, -8) * pow(T, 4)) \
    - (6 * pow(10, -6) * pow(T, 3)) \
    + (4 * pow(10, -4) * pow(T, 2)) \
    + (0.0462 * T) + 1.1977

#define SUPPLY ((double) 5.0)

const int cpg_pulse_angles[3] = {0, 60, 360};

size_t buffer = 0;
char message[MESSAGE_SIZE];
bool message_available = false;

unsigned int temp = 0;
unsigned int speed = 0;
unsigned int angle = 0;

unsigned long pulse_width = 0;

bool is_running = false;

unsigned long counter;

unsigned long last_pulse = micros();

char pin_state(int address){
    return (REGISTER >> address) & 1;
}

void open_circuit(int address){
    REGISTER &= ~(1 << address);
}

void close_circuit(int address){
    REGISTER |= 1 << address;
}

void set_temperature_pwm(void){
    double temp_supply_ratio = (THERMISTOR_VOLTAGE((double) temp)) / 5.0;
    int temp_pwm = temp_supply_ratio * 255;

    analogWrite(TEMP_PIN, temp_pwm);
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

    open_circuit(TEMP_ADDRESS);
    open_circuit(CPG_ADDRESS);
    open_circuit(IPG_ADDRESS);

    is_running = false;
}

void set_simulation(instr* i){
    if(i->speed > 0){
        speed = i->speed;
        uint32_t pulses_per_second = (speed / 60) * (360 / IPG_HIGH_ANGLE);
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
    float voltage = THERMISTOR_VOLTAGE((double) temp);
    int voltage_integer = (int) voltage;
    int voltage_decimal = (voltage - voltage_integer) * 10;

    sprintf(message, 
        "simulator info:\n    is running: %s\n    temp: %u deg C (~%i.%i V)\n    speed: %u RPM\n    pulse width: %lu us\n",
        is_running ? "true" : "false", temp, voltage_integer, voltage_decimal, speed, pulse_width);
}

void setup(void){
    Serial.begin(9600);

    pinMode(TEMP_PIN, OUTPUT);

    pinMode(CPG_PIN, OUTPUT);
    pinMode(IPG_PIN, OUTPUT);

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

        int cpg_pulse = 0;

        if(pin_state(CPG_ADDRESS)){
            open_circuit(CPG_ADDRESS);
        } else {
            for(size_t i = 0; i < 3; i++){
                if(angle == cpg_pulse_angles[i]){
                    cpg_pulse = 1;
                }
            }
        }

        if(angle % (2 * IPG_HIGH_ANGLE) == 0){
            REGISTER = (1 << IPG_ADDRESS) + (cpg_pulse << CPG_ADDRESS);

        } else {
            open_circuit(IPG_ADDRESS);
        }
    }
}