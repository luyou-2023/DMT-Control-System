#include "messages.h"

void get_message_keywords(const char* message, keywords kws){
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
    } else if(!strcmp(k[0], GET_KEYWORD)){
        return GET_CODE;
    } else if(!strcmp(k[0], SET_KEYWORD)){
        return SET_CODE;
    } 

    return INVALID_CODE;
}

int get_flag_index(keywords k, char* flag){
    for(size_t i = 1; i < MAX_KEYWORDS - 1; i++){
        if(!strcmp(k[i], flag)) return i;
    }

    return -1;
}

pin* get_target(keywords k, engine* e){
    int i = get_flag_index(k, TARGET_FLAG);
    
    if(i == -1) return NULL;

    if(!strcmp(k[i + 1], THERMISTOR_KEYWORD)){
        return &(e->thermistor);
    } else if(!strcmp(k[i + 1], CRANKSHAFT_KEYWORD)){
        return &(e->ipg);
    } else if(!strcmp(k[i + 1], CAMSHAFT_KEYWORD)){
        return &(e->cpg);
    }

    pin* p = NULL;

    if(!strcmp(k[i + 1], INJECTOR_KEYWORD)){
        p = e->injs;
    } else if(!strcmp(k[i + 1], COIL_KEYWORD)){
        p = e->coils;
    }

    if(p && i < MAX_KEYWORDS - 2){
        long c = strtol(k[i + 2], NULL, 0);
        if(c >= 1 && c <= 4) return &(p[c - 1]);
    }

    return NULL;
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
        .target = get_target(kws, e),
        .speed = get_speed(kws),
    };
}

void get_instruction_message(instr* i, char message[150]){
    char type_name[10] = INVALID_KEYWORD;
    char target_string[100] = "not given";
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
        case GET_CODE:
            sprintf(type_name, GET_KEYWORD);
    }

    if((i->type == STOP_CODE || i->type == SET_CODE || i->type == GET_CODE) && i->target){
        sprintf(target_string, "\n        name: %s\n        pin: %i\n        address: %p", 
            i->target->name, i->target->pin, i->target->reg + i->target->num);
    }

    if(i->type == SET_CODE && i->speed != -1){
        sprintf(speed_string, "%i rpm", i->speed);
    }

    sprintf(message, "\nnew instruction:\n    type: %s\n    target: %s\n    speed: %s\n", 
        type_name, target_string, speed_string);
}

void print_target_value(instr* i, engine* e, char* message){
    char value[20];

    if(!strcmp(i->target->name, THERMISTOR_KEYWORD)){
        sprintf(value, "%i degC", (int) get_internal_temp(e));
    } else {
        sprintf(value, "%s", pin_state(i->target) ? "HIGH" : "LOW");
    }

    sprintf(message, "%s: %s\n", i->target->name, value);
}