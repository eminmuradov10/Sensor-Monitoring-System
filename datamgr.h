/**
 * \author Emin Muradov
 */

#ifndef DATAMGR_H_
#define DATAMGR_H_

#include "config.h"
#include "sbuffer.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
#error SET_MAX_TEMP not set
#endif

#ifndef SET_MIN_TEMP
#error SET_MIN_TEMP not set
#endif

struct sensor{
    sensor_id_t sensor_id;
    sensor_id_t room_id;
    sensor_value_t running_avg;
    sensor_ts_t last_mod;
    sensor_value_t datamgr_avg[RUN_AVG_LENGTH];
};
typedef struct sensor sensor_t;

void datamgr_thread(thread_args_t* arguments);
void add_new_temp(sensor_t* sensor, sensor_value_t temp_buffer,thread_args_t* arguments);
int get_index_of_sensor_id(int sensor_id,dplist_t* list_ptr_t,thread_args_t* arguments);

#endif  //DATAMGR_H_
