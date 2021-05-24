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

int get_flag_value(keywords k, char* flag){
    int i = get_flag_index(k, flag);

    if(i == -1) return -1;
    
    long v = strtol(k[i + 1], NULL, 0);
    return v > 0 && v < INT16_MAX ? (int) v : -1;
}

instr get_instruction(const char* message){
    if(!message) return INVALID_INSTR;

    keywords kws;

    get_message_keywords(message, kws);

    return (instr) {
        .type = get_type(kws),
        .speed = get_flag_value(kws, SPEED_FLAG),
        .temp = get_flag_value(kws, TEMP_FLAG)
    };
}

void get_instruction_message(instr* i, char message[150]){
    char type_name[10] = INVALID_KEYWORD;
    char speed_string[50] = "not given";
    char temp_string[50] = "not given";

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
        sprintf(speed_string, "%i RPM", i->speed);
    }

    if(i->type == SET_CODE && i->temp != -1){
        sprintf(temp_string, "%i deg C", i->temp);
    }

    sprintf(message, "\nnew instruction:\n    type: %s\n    speed: %s\n    temp: %s\n", 
        type_name, speed_string, temp_string);
}