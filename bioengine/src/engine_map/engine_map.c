#include "engine_map.h"

void init_timings(timings* t){
    t->spark[0] = 0;
    t->spark[1] = 0;

    t->fuel[0] = 0;
    t->fuel[1] = 0;

    t->o = NULL;
    t->is_valid = false;
}

int set_operating_point(operating_point* o, unsigned int target_speed){
    for(size_t i = 0; i < MAP_SIZE; i++){
        if(target_speed == operating_map[i].speed){
            o = &(operating_map[i]);
            return 0;
        }
    }

    return 1;
}

int set_engine_timings(timings* t, const operating_point* o, const engine* e){
    if(!o || !e){
        t->is_valid = false;
        return 1;
    }

    t->o = o;

    t->spark[1] = 360 - o->spark_btdc;
    t->spark[0] = t->spark[1] - (e->speed * DWELL_TIME);

    t->fuel[0] = MIN_FUEL_START_ANGLE;
    t->fuel[1] = o->inj_duration;

    if(t->fuel[1] > MAX_FUEL_END_ANGLE){
        t->is_valid = false;
        return 1;
    }

    t->is_valid = true;

    return 0;
}