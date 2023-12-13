/**
 * \author Emin Muradov
 */

#include "sbuffer.h"

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;

    (*buffer)->last_update = time(NULL);
    pthread_rwlock_init(&(*buffer)->lock, NULL);

    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    
    }
    
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    pthread_rwlock_wrlock(&buffer->lock);
    
    sbuffer_node_t *dummy;
    if (buffer == NULL){
        pthread_rwlock_unlock(&buffer->lock); 
        return SBUFFER_NULL_BUFF;

    }
   
    if (buffer->head == NULL){
        pthread_rwlock_unlock(&buffer->lock); 
        return SBUFFER_EMPTY_BUFF;
    }
    
    *data = buffer->head->data;
    dummy = buffer->head;
    
    if (buffer->head == buffer->tail) // buffer has only one node
    {
        buffer->head = buffer->tail = NULL;
    }else  // buffer has many nodes empty
    {
        buffer->head = buffer->head->next;
    }
    
    free(dummy);

    pthread_rwlock_unlock(&buffer->lock);
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    pthread_rwlock_wrlock(&buffer->lock);
    
    sbuffer_node_t *dummy;
    if (buffer == NULL){
        pthread_rwlock_unlock(&buffer->lock);
        return SBUFFER_NULL_BUFF;
    } 

    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) {
        pthread_rwlock_unlock(&buffer->lock);
        return SBUFFER_FAILURE;
    }

    dummy->data = *data;
    dummy->next = NULL;

    if (buffer->tail == NULL) 
    {
        buffer->head = buffer->tail = dummy;
    } else 
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    
    dummy->has_seen_by_DATAMGR=false;
    dummy->has_seen_by_SQLMGR=false;
    buffer->last_update=time(NULL);

    pthread_rwlock_unlock(&buffer->lock);
    return SBUFFER_SUCCESS;
}

int sbuffer_read(sbuffer_t *buffer,sensor_data_t* sensor_data,type type,bool terminated_by_CONNMGR){
    pthread_rwlock_wrlock(&buffer->lock);

    if(buffer == NULL){
        pthread_rwlock_unlock(&buffer->lock);
        return SBUFFER_NULL_BUFF;
    }

    if (buffer->head == NULL){
        pthread_rwlock_unlock(&buffer->lock);
        return SBUFFER_EMPTY_BUFF;
    }

    if(buffer->head != NULL){
        sbuffer_node_t* current_node=buffer->head;
        while(current_node != NULL){

            if(current_node->has_seen_by_SQLMGR==true && current_node->has_seen_by_DATAMGR==true){
                
                buffer->last_update=time(NULL);
                
                //printf("SBUFFER: removing sensor_id=%d temperature=%f°C at time=%ld,it has seen by both\n",sensor_data->id,sensor_data->value,sensor_data->ts);
                
                if (buffer->head == buffer->tail){
                    buffer->head = buffer->tail = NULL;
                } else{
                    buffer->head = buffer->head->next;
                    free(current_node);
                    current_node=buffer->head;
                }
                
                if(terminated_by_CONNMGR==true){
                    free(current_node);
                    pthread_rwlock_unlock(&buffer->lock);
                    return SBUFFER_NO_NEW_DATA;
                }

            }

            if(current_node->has_seen_by_DATAMGR==false && type==DATAMGR){
                
                current_node->has_seen_by_DATAMGR=true;
                *sensor_data=current_node->data;
                buffer->last_update=time(NULL);
                
                //printf("SBUFFER: sensor_id=%d temperature=%f°C at time=%ld is sent to DATAMGR\n",sensor_data->id,sensor_data->value,sensor_data->ts);
                
                pthread_rwlock_unlock(&buffer->lock);
                return SBUFFER_SUCCESS;
            }

            if(current_node->has_seen_by_SQLMGR==false && type==SQLMGR){
                
                current_node->has_seen_by_SQLMGR=true;
                *sensor_data=current_node->data;
                buffer->last_update=time(NULL);
                
                //printf("SBUFFER: sensor_id=%d temperature=%f°C at time=%ld is sent to SQLMGR\n",sensor_data->id,sensor_data->value,sensor_data->ts);

                pthread_rwlock_unlock(&buffer->lock);
                return SBUFFER_SUCCESS;
            }
            
            current_node = current_node->next;
        }
    }

    pthread_rwlock_unlock(&buffer->lock);
    return SBUFFER_NO_NEW_DATA;
}

int getinfo(sbuffer_t *buffer,type type){    
    pthread_rwlock_rdlock(&buffer->lock);

    if(buffer == NULL){
        pthread_rwlock_unlock(&buffer->lock);
        return SBUFFER_NULL_BUFF;  
    }

    if (buffer->head == NULL){
        pthread_rwlock_unlock(&buffer->lock);
        return SBUFFER_EMPTY_BUFF;
    }

    if(buffer->head != NULL){
        sbuffer_node_t* current_node = buffer->head;
        while(current_node != NULL){
            
            if(current_node->has_seen_by_DATAMGR==false && type==DATAMGR){
                
                pthread_rwlock_unlock(&buffer->lock);
                return SBUFFER_SUCCESS;
            }

            if(current_node->has_seen_by_SQLMGR==false && type==SQLMGR){
                
                pthread_rwlock_unlock(&buffer->lock);
                return SBUFFER_SUCCESS;
            }
            
            current_node = current_node->next;
        }
    }

    pthread_rwlock_unlock(&buffer->lock);
    return SBUFFER_NO_NEW_DATA;  
}
