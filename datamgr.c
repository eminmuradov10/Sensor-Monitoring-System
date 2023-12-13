/**
 * \author Emin Muradov
 */

#define _GNU_SOURCE
#include "datamgr.h"

void* element_copy2(void* src_element){
    sensor_t* element_ptr_t=(sensor_t*)src_element;

    sensor_t* sensor_node=malloc(sizeof(sensor_t));
    assert(sensor_node != NULL);
    
    sensor_node->sensor_id=element_ptr_t->sensor_id;
    sensor_node->room_id=element_ptr_t->room_id;
    sensor_node->running_avg=element_ptr_t->running_avg;
    sensor_node->last_mod=element_ptr_t->last_mod;
    for(int c=0;c<RUN_AVG_LENGTH;c++){
        sensor_node->datamgr_avg[c]=element_ptr_t->datamgr_avg[c];
    }

    return (void*) sensor_node;
}

void element_free2(void ** element) {

    free(*element);
    *element = NULL;
}

int element_compare2(void* x, void* y){

    if(((sensor_t*)x)->sensor_id>((sensor_t*)y)->sensor_id){

        return 1;
    }else if (((sensor_t*)x)->sensor_id==((sensor_t*)y)->sensor_id){
        
        return 0;
    }else{
        
        return -1;
    }
}

int get_index_of_sensor_id(int sensor_id,dplist_t* list_ptr_t,thread_args_t* arguments){

    for(int i=0;i<dpl_size(list_ptr_t);i++){
        sensor_t* sensor=dpl_get_element_at_index(list_ptr_t,i);
        
        if(sensor->sensor_id==sensor_id){
            return i;
        }
    }

    return -1;
}

void add_new_temp(sensor_t* sensor, sensor_value_t temp_buffer,thread_args_t* arguments){
    
    sensor_value_t sum_of_temp=0;
    char counter=RUN_AVG_LENGTH;

    for(int i=1; i<=(RUN_AVG_LENGTH-1);i++){
        sensor->datamgr_avg[i-1]=sensor->datamgr_avg[i];
        sum_of_temp+=sensor->datamgr_avg[i-1];
        
        if(sensor->datamgr_avg[i-1]==0){
            counter--;
        }
    }

    sum_of_temp+=temp_buffer;
    sensor->datamgr_avg[RUN_AVG_LENGTH-1]=temp_buffer;
    sensor->running_avg=sum_of_temp/counter;

    char* fifo_buffer;
    if(sensor->running_avg>SET_MAX_TEMP){

        ASPRINTF_ERROR(asprintf(&(fifo_buffer), "DATAMGR:the sensor node with %hu reports it's too hot (running avg temperature = %f)\n", sensor->sensor_id, sensor->running_avg));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;
    }

    if(sensor->running_avg<SET_MIN_TEMP){

        ASPRINTF_ERROR(asprintf(&(fifo_buffer), "DATAMGR:the sensor node with %hu reports it's too cold (running avg temperature = %f)\n", sensor->sensor_id, sensor->running_avg));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;
    }
}

void datamgr_thread(thread_args_t* arguments){
    
    char* fifo_buffer;
    ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the DATAMGR has started\n"));
    log_this(fifo_buffer,arguments);
    free(fifo_buffer);
    fifo_buffer=NULL;

    dplist_t* list_ptr_t=dpl_create(element_copy2,element_free2,element_compare2);
    sensor_id_t room_id_buffer;
    sensor_id_t sensor_id_buffer;

    //to test invalid sensors, put comment on correct one, uncomment wrong one
    FILE* fp_sensor_map = fopen("room_sensor.map","r");
    //FILE* fp_sensor_map = fopen("room_sensor(test_invalid).map","r");

    while(fscanf(fp_sensor_map,"%hd %hd",&room_id_buffer,&sensor_id_buffer)==2){
        sensor_t* new_sensor=malloc(sizeof(sensor_t));
        new_sensor->sensor_id=sensor_id_buffer;
        new_sensor->room_id=room_id_buffer;
        for(int i=0; i<RUN_AVG_LENGTH;i++){
            new_sensor->datamgr_avg[i]=0;
            
        }
        dpl_insert_at_index(list_ptr_t,new_sensor,dpl_size(list_ptr_t),false);
    }
    
    ASPRINTF_ERROR(asprintf(&(fifo_buffer),"DATAMGR: room_sensor.map has been parsed\n"));
    log_this(fifo_buffer,arguments);
    free(fifo_buffer);
    fifo_buffer=NULL;

    sensor_data_t* sensor_data = malloc(sizeof(sensor_data_t));
    bool quit=false;
    
    pthread_mutex_lock(arguments->lock);
    while((arguments->terminated_by_INIT_CONNECTION==false)&&(arguments->conn==NULL)){
        //printf("waiting for database in DATAMGR\n");
        pthread_cond_wait(arguments->condition,arguments->lock);
    }
    if((arguments->terminated_by_INIT_CONNECTION==true)||(arguments->conn==NULL)){

        ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the DATAMGR is terminated by INIT_CONNECTION\n"));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;

        quit=true;
    }
    pthread_mutex_unlock(arguments->lock);

    while (!quit){
        
        if(arguments->terminated_by_CONNMGR==true){

            ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the DATAMGR is terminated by CONNMGR since it does not have correct port\n"));
            log_this(fifo_buffer,arguments);
            free(fifo_buffer);
            fifo_buffer=NULL;

            quit=true;
        }

        pthread_mutex_lock(arguments->lock);
        while(((getinfo(*(arguments->buffer),DATAMGR)!=SBUFFER_SUCCESS))&&(arguments->terminated_by_CONNMGR==false)){
            pthread_cond_wait(arguments->condition,arguments->lock);
            if(arguments->terminated_by_CONNMGR==true){

                ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the DATAMGR is terminated by CONNMGR since there is no server left\n"));
                log_this(fifo_buffer,arguments);
                free(fifo_buffer);
                fifo_buffer=NULL;

                quit=true;
            }
        }
        pthread_mutex_unlock(arguments->lock);

        int result = sbuffer_read(*(arguments->buffer), sensor_data, DATAMGR,arguments->terminated_by_CONNMGR);
        if(result==SBUFFER_SUCCESS){
            
            if(get_index_of_sensor_id(sensor_data->id, list_ptr_t,arguments)!=-1){
                
                sensor_t* sensor=dpl_get_element_at_index(list_ptr_t,get_index_of_sensor_id(sensor_data->id, list_ptr_t,arguments));
                sensor->last_mod = sensor_data->ts;
                add_new_temp(sensor,sensor_data->value,arguments);
            }else{

                char* fifo_buffer;
                ASPRINTF_ERROR(asprintf(&(fifo_buffer), "DATAMGR:invalid sensor_id=%d\n", sensor_data->id));
                log_this(fifo_buffer,arguments);
                free(fifo_buffer);
                fifo_buffer=NULL;

            }
        }
    }

    free(sensor_data);
    sensor_data=NULL;
    fclose(fp_sensor_map);
    dpl_free(&list_ptr_t, true);
}

