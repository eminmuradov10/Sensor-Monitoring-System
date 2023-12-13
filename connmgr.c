/**
 * \author Emin Muradov
 */

#define _GNU_SOURCE
#include "connmgr.h"

void* conn_copy(void * element) {
    return NULL;
}

void conn_free(void** element) {
    sensor_node_t* lmnt = (sensor_node_t*)*element;
    tcp_close(&(lmnt->socket_nr));
    free(*element);
    *element = NULL;
}

int conn_compare(void * x, void * y) {
    if( ((sensor_node_t*)x)->socket_nr->sd == ((sensor_node_t*)y)->socket_nr->sd ){
        return 0; 
    }
    return 1;
}

void connmgr_thread(thread_args_t* arguments){
    
    char* fifo_buffer;
    ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the CONNMGR has started\n"));
    log_this(fifo_buffer,arguments);
    free(fifo_buffer);
    fifo_buffer=NULL;

    FILE* fp=fopen("sensor_data_recv.txt","w");
    
    dplist_t* connections=dpl_create(conn_copy,conn_free,conn_compare);

    server_node_t* server_node=malloc(sizeof(server_node_t));
    server_node->socket_nr=NULL;
    server_node->last_mod=time(NULL);
    
    struct pollfd* poll_list=NULL;
    sensor_data_t* sensor_data=malloc(sizeof(sensor_data_t));

    bool quit=false;
    int counter=TIMEOUT;
    
    if(tcp_passive_open(&(server_node->socket_nr),arguments->port)==TCP_NO_ERROR){
        
        ASPRINTF_ERROR(asprintf( &(fifo_buffer), "CONNMGR: server is present\n"));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;
        
        server_node->last_mod=time(NULL);
        connections=dpl_insert_at_index(connections,server_node,0,false);

    }else{
        tcp_close(&(server_node->socket_nr));

        ASPRINTF_ERROR(asprintf( &(fifo_buffer), "CONNMGR: server is closed no having port :/\n"));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;

        arguments->terminated_by_CONNMGR=true;
        pthread_cond_broadcast(arguments->condition); 
        quit=true;
    }

    if (quit!=true){

        pthread_mutex_lock(arguments->lock);
        while((arguments->terminated_by_INIT_CONNECTION==false)&&(arguments->conn==NULL)){
            //printf("waiting for database in CONNMGR\n");
            pthread_cond_wait(arguments->condition,arguments->lock);
        }
        if((arguments->terminated_by_INIT_CONNECTION==true)||(arguments->conn==NULL)){

            tcp_close(&(server_node->socket_nr));

            ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the CONNMGR is terminated by INIT_CONNECTION\n"));
            log_this(fifo_buffer,arguments);
            free(fifo_buffer);
            fifo_buffer=NULL;

            quit=true;
        }
        pthread_mutex_unlock(arguments->lock);
    }

    while(!quit){

        poll_list=(struct pollfd*)calloc(dpl_size(connections)+1,sizeof(struct pollfd));
        poll_list[0].fd=-1;
        if(tcp_get_sd(server_node->socket_nr,&(poll_list[0].fd))==TCP_NO_ERROR){
            poll_list[0].events=POLLIN|POLLRDHUP;   
            poll_list[0].revents=0;
        }

        for(int i=1;i<dpl_size(connections);i++){
            poll_list[i].fd=((sensor_node_t*)dpl_get_element_at_index(connections,i))->socket_nr->sd;
            poll_list[i].events=POLLIN|POLLRDHUP;
            poll_list[i].revents=0;
        }

        poll(poll_list,dpl_size(connections),TIMEOUT*1000);
        for(int i=0; i<dpl_size(connections);i++){
            sensor_node_t* searched_sensor=(sensor_node_t*)(dpl_get_element_at_index(connections,i));
            
            if(i!=0 && ((difftime(time(NULL),searched_sensor->last_mod))>TIMEOUT)){
                int port=searched_sensor->socket_nr->port;
                int id=searched_sensor->sensor_id;
                
                TCP_CLOSE_ERROR(tcp_close(&(searched_sensor->socket_nr)));
                connections=dpl_remove_at_index(connections,i,true);
                
                if(dpl_size(connections)>1){

                    ASPRINTF_ERROR(asprintf( &(fifo_buffer), "CONNMGR: sensor_id=%d from port=%d is timed out,%d connections left(including server)\n",id,port,dpl_size(connections)));
                    log_this(fifo_buffer,arguments);
                    free(fifo_buffer);
                    fifo_buffer=NULL;
                    
                }else{
                    
                    ASPRINTF_ERROR(asprintf( &(fifo_buffer), "CONNMGR: sensor_id=%d from port=%d is timed out,only the server is left\n",id,port));
                    log_this(fifo_buffer,arguments);
                    free(fifo_buffer);
                    fifo_buffer=NULL;
                    
                }
                
                server_node->last_mod=time(NULL);
                break;
            }

            if(poll_list[i].revents&POLLRDHUP){ 
                int port=searched_sensor->socket_nr->port;
                int id=searched_sensor->sensor_id;
                
                
                ASPRINTF_ERROR(asprintf(&(fifo_buffer), "CONNMGR: sensor_id=%d from port=%d is hanged up and %d connection(s) left(including server)\n",id,port,dpl_size(connections)-1));
                log_this(fifo_buffer,arguments);
                free(fifo_buffer);
                fifo_buffer=NULL;
                
                
                tcp_close(&(searched_sensor->socket_nr));
                connections=dpl_remove_at_index(connections,i,true);
                
                server_node->last_mod=time(NULL);
                break;
            }
            
            if((i==0)&&(poll_list[i].revents&POLLIN)){
                sensor_node_t* new_sensor_node=malloc(sizeof(sensor_node_t));
                
                if(tcp_wait_for_connection(server_node->socket_nr,&(new_sensor_node->socket_nr))==TCP_NO_ERROR){
                    
                    new_sensor_node->last_mod=time(NULL);
                    new_sensor_node->sensor_id=0;
                    new_sensor_node->sensor_temp=0;
                    new_sensor_node->sensor_timestp=0;

                    connections=dpl_insert_at_index(connections,new_sensor_node,dpl_size(connections),false);
                }

                server_node->last_mod=time(NULL);
            }

            if((i!=0)&&(poll_list[i].revents&POLLIN)){
                sensor_node_t* searched_sensor=(sensor_node_t*)(dpl_get_element_at_index(connections,i));
                searched_sensor->last_mod=time(NULL);

                int size_buffer=sizeof(sensor_data->id);
                tcp_receive(searched_sensor->socket_nr,(void*)&(sensor_data->id),&size_buffer);
                searched_sensor->sensor_id=sensor_data->id;

                size_buffer=sizeof(sensor_data->value);
                tcp_receive(searched_sensor->socket_nr,(void*)&(sensor_data->value),&size_buffer);
                searched_sensor->sensor_temp=sensor_data->value;

                size_buffer=sizeof(sensor_data->ts);
                tcp_receive(searched_sensor->socket_nr,(void*)&(sensor_data->ts),&size_buffer);
                searched_sensor->sensor_timestp=sensor_data->ts;
                
                ASPRINTF_ERROR(asprintf( &(fifo_buffer), "CONNMGR: a sensor node with sensor_id=%d has opened a new connection\n", searched_sensor->sensor_id));
                log_this(fifo_buffer,arguments);
                free(fifo_buffer);
                fifo_buffer=NULL;
                
                fprintf(fp, "%d %lf %ld\n", sensor_data->id, sensor_data->value, sensor_data->ts);
                
                sbuffer_insert(*(arguments->buffer),sensor_data);
                pthread_cond_broadcast(arguments->condition);
            }

            if((dpl_size(connections)==1)){
                if((difftime(time(NULL),server_node->last_mod)>TIMEOUT)){
                    tcp_close(&(server_node->socket_nr));

                    ASPRINTF_ERROR(asprintf( &(fifo_buffer), "CONNMGR: no activity, server is closed\n"));
                    log_this(fifo_buffer,arguments);
                    free(fifo_buffer);
                    fifo_buffer=NULL;
                    
                    quit=true;
                }else{
                    ASPRINTF_ERROR(asprintf( &(fifo_buffer), "CONNMGR: no activity, server will close in %d second(s)\n",counter));
                    log_this(fifo_buffer,arguments);
                    free(fifo_buffer);
                    fifo_buffer=NULL;
                    counter--;
                }
            }
        }
        
        free(poll_list);
        poll_list=NULL;
    }
        
    connmgr_free_list(connections,false);
    free(server_node);
    server_node=NULL;
    free(sensor_data);
    sensor_data=NULL;
    FILE_CLOSE_ERROR(fclose(fp));

    if((arguments->terminated_by_CONNMGR==false)&&(arguments->terminated_by_INIT_CONNECTION==false)){
        ASPRINTF_ERROR(asprintf( &(fifo_buffer), "the CONNMGR has ended due to no server\n"));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;

        arguments->terminated_by_CONNMGR=true;
        pthread_cond_broadcast(arguments->condition); 
    }
    
}

void connmgr_free_list(dplist_t* connections, bool free_element){
    dpl_free(&connections,free_element);
}
