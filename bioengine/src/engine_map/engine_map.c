#include "engine_map.h"

void init_timings(timings* t){
    t->spark[0] = 0;
    t->spark[1] = 0;

    t->fuel[0] = 0;
    t->fuel[1] = 0;

    t->o = NULL;
    t->is_valid = false;
}

operating_point get_operating_point(unsigned int target_speed){
    for(size_t i = 0; i < MAP_SIZE; i++){
        if(target_speed == operating_map[i].speed){
            return operating_map[i];
        }
    }

    return INVALID_OPERATING_POINT;
}

int set_engine_timings(timings* t, const operating_point* o, const engine* e){
    if(!o || !e || e->rpm == 0){
        t->is_valid = false;
        return 1;
    }

    t->o = o;

    t->spark[1] = 360 - o->spark_btdc;
    t->spark[0] = t->spark[1] - (e->speed * DWELL_TIME);

    t->fuel[0] = MIN_FUEL_START_ANGLE;
    t->fuel[1] = MIN_FUEL_START_ANGLE + o->inj_duration;

    if(t->fuel[1] > MAX_FUEL_END_ANGLE || t->spark[0] < MIN_CHARGE_ANGLE){
        t->is_valid = false;
        return 1;
    }

    t->is_valid = true;

    return 0;
}

void new_operating_point(unsigned int rpm, operating_point* o, timings* t, engine* e, char* message){
    *o = get_operating_point(rpm);

    if(o->speed == -1){
        sprintf(message, "Could not find target RPM.\n");
    }

    int err = set_engine_timings(t, o, e);

    if(err){
        sprintf(message, "Timings are invalid.\n");
    } else {
        sprintf(message, "Engine operating point and timings updated.\n");
    }
}

void get_timing_info(timings* t, char message[150]){
    if(!t->is_valid){
        sprintf(message, "timings not valid.\n");
    } else {
        sprintf(message, "timings:\n    target RPM: %i\n    spark: ~%i to ~%i deg\n    fuel: ~%i to ~%i deg\n    is valid: %s\n",
            t->o->speed, (int) t->spark[0], (int) t->spark[1], (int) t->fuel[0], (int) t->fuel[1], t->is_valid ? "true" : "false");
    }
}