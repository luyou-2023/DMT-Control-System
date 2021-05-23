#include "messages.h"

void get_message_keywords(const char message[], keywords kws){
    if(!message || !kws) return;

    size_t i = 0, j = 0, k = 0;

    while(message[i] != '\n' && k != MAX_KEYWORDS){
        if(message[i] == ' '){
            if(i > 0 && j > 0){
                kws[k][j] = '\0';
                j = 0; k++;
            }
        } else if(j < MAX_KEYWORD_LENGTH){
            kws[k][j] = message[i];
            j++;
        }

        i++;
    }
    kws[k][j] = '\0';
}

char get_type(keywords k){
    if(!strcmp(k[0], START_KEYWORD)){
        return START_CODE;
    } else if(!strcmp(k[0], STOP_KEYWORD)){
        return STOP_CODE;
    } else if(!strcmp(k[0], SET_KEYWORD)){
        return SET_CODE;
    } else if(!strcmp(k[0], STATUS_KEYWORD)){
        return STATUS_CODE;
    }

    return INVALID_CODE;
}

int get_flag_index(keywords k, char* flag){
    for(size_t i = 1; i < MAX_KEYWORDS - 1; i++){
        if(!strcmp(k[i], flag)) return i;
    }

    return -1;
}

long get_speed(keywords k){
    int i = get_flag_index(k, SPEED_FLAG);
    return i != -1 ? strtol(k[i + 1], NULL, 0) : -1;
}

instr get_instruction(const char* message, engine* e){
    if(!message) return INVALID_INSTR;

    keywords kws;

    get_message_keywords(message, kws);

    return (instr) {
        .type = get_type(kws),
        .speed = get_speed(kws),
    };
}

void get_instruction_message(instr* i, char message[150]){
    char type_name[10] = INVALID_KEYWORD;
    char speed_string[50] = "not given";

    switch(i->type){
        case START_CODE:
            sprintf(type_name, START_KEYWORD);
            break;
        case STOP_CODE:
            sprintf(type_name, STOP_KEYWORD);
            break;
        case SET_CODE:
            sprintf(type_name, SET_KEYWORD);
            break;
        case STATUS_CODE:
            sprintf(type_name, STATUS_KEYWORD);
    }

    if(i->type == SET_CODE && i->speed != -1){
        sprintf(speed_string, "%i rpm", i->speed);
    }

    sprintf(message, "\nnew instruction:\n    type: %s\n    speed: %s\n", 
        type_name, speed_string);
}

void get_engine_info(engine* e, char message[150]){
    int speed_rpm = (int) (pow(10, 6) * 60 * e->speed) / 360;
    sprintf(message, "engine status:\n    crank angle: %i deg\n    speed: %i RPM\n   temp: %i deg C\n    is running: %b\n", 
        e->crank, speed_rpm, e->temp, e->is_running);
}

void get_timing_info(timings* t, char message[150]){
    sprintf(message, "timings:\n    target RPM: %i\n    spark: ~%i to ~%i deg\n    fuel: ~%i to ~%i deg\n    is valid: %b\n",
        t->o->speed, (int) t->spark[0], (int) t->spark[1], (int) t->fuel[0], (int) t->fuel[1], t->is_valid);
}