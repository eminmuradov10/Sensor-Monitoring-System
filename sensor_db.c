/**
 * \author Emin Muradov
 */

#define _GNU_SOURCE
#include "sensor_db.h"

void sqlmgr_thread(thread_args_t* arguments){

    char* fifo_buffer;
    ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the SQLMGR has started\n"));
    log_this(fifo_buffer,arguments);
    free(fifo_buffer);
    fifo_buffer=NULL;
    
    sensor_data_t* sensor_data = malloc(sizeof(sensor_data_t));
    bool quit=false;

    pthread_mutex_lock(arguments->lock);
    while((arguments->terminated_by_INIT_CONNECTION==false)&&(arguments->conn==NULL)){
        //printf("waiting for database in SQLMGR\n");
        pthread_cond_wait(arguments->condition,arguments->lock);
    }
    if((arguments->terminated_by_INIT_CONNECTION==true)||(arguments->conn==NULL)){

        ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the SQLMGR is terminated by INIT_CONNECTION\n"));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;

        quit=true;
    }
    pthread_mutex_unlock(arguments->lock);

    while (!quit){

        if(arguments->terminated_by_CONNMGR==true){

            ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the SQLMGR is terminated by CONNMGR since it does not have correct port\n"));
            log_this(fifo_buffer,arguments);
            free(fifo_buffer);
            fifo_buffer=NULL;

            quit=true;
        }

        pthread_mutex_lock(arguments->lock);
        while(((getinfo(*(arguments->buffer),SQLMGR)!=SBUFFER_SUCCESS))&&(arguments->terminated_by_CONNMGR==false)){
            pthread_cond_wait(arguments->condition,arguments->lock);
            if(arguments->terminated_by_CONNMGR==true){

                ASPRINTF_ERROR(asprintf(&(fifo_buffer),"the SQLMGR is terminated by CONNMGR since there is no server left\n"));
                log_this(fifo_buffer,arguments);
                free(fifo_buffer);
                fifo_buffer=NULL; 

                quit=true; 
            }
        }
        pthread_mutex_unlock(arguments->lock);
        
        int result = sbuffer_read(*(arguments->buffer), sensor_data, SQLMGR,arguments->terminated_by_CONNMGR);
        if(result==SBUFFER_SUCCESS){
            if((insert_sensor(arguments,sensor_data->id, sensor_data->value, sensor_data->ts)==SQLITE_OK)){

                ASPRINTF_ERROR(asprintf(&(fifo_buffer), "SQLMGR: sensor_id=%d temperature=%f°C at time=%ld is inserted\n", sensor_data->id, sensor_data->value, sensor_data->ts));
                log_this(fifo_buffer,arguments);
                free(fifo_buffer);
                fifo_buffer=NULL;
            }else{
                
                ASPRINTF_ERROR(asprintf(&(fifo_buffer), "SQLMGR :connection to sql server lost\n"));
                log_this(fifo_buffer,arguments);
                free(fifo_buffer);
                fifo_buffer=NULL;
            }
        }
    }

    free(sensor_data);
    sensor_data=NULL;
}

/**
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME
 * \param clear_up_flag if the table existed, clear up the existing data when clear_up_flag is set to 1
 * \return the connection for success, NULL if an error occurs
 */

DBCONN *init_connection(char clear_up_flag,thread_args_t* arguments){
    //https://zetcode.com/db/sqlitec/
    sqlite3* db;
    char* err_msg;
    int rc;

    char* fifo_buffer;

    //sqlite3_stmt* res;
    int counter=0;
    do{
        rc=sqlite3_open(TO_STRING(DS_NAME),&db);
        if(rc!=SQLITE_OK){

            ASPRINTF_ERROR(asprintf(&(fifo_buffer), "INIT_CONNECTION: Unable to connect to sql server\n"));
            log_this(fifo_buffer,arguments);
            free(fifo_buffer);
            fifo_buffer=NULL;
            counter++;
        }
    }while((rc!=SQLITE_OK)&&counter<3);

    if(rc!=SQLITE_OK){

        ASPRINTF_ERROR(asprintf(&(fifo_buffer), "INIT_CONNECTION: Cannot open database: %s\n", sqlite3_errmsg(db)));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;

        arguments->terminated_by_INIT_CONNECTION=true;
        arguments->conn=db;
        pthread_cond_broadcast(arguments->condition);
        
    }else{

        ASPRINTF_ERROR(asprintf(&(fifo_buffer), "INIT_CONNECTION: Connection to sql server established\n"));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;

        arguments->terminated_by_INIT_CONNECTION=false;
        arguments->conn=db;
        pthread_cond_broadcast(arguments->condition);
        
        char* sequel=0;
        ASPRINTF_ERROR(asprintf(&sequel,"CREATE TABLE IF NOT EXISTS %s(Id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_id INT, sensor_value DECIMAL(4,2), timestamp TIMESTAMP);",TO_STRING(TABLE_NAME)));
        rc = sqlite3_exec(db, sequel, 0, 0, &err_msg);
        if (clear_up_flag==1){
            char* sequel_1;
            ASPRINTF_ERROR(asprintf(&sequel_1,"DROP TABLE IF EXISTS %s; CREATE TABLE %s(Id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_id INT, sensor_value DECIMAL(4,2), timestamp TIMESTAMP);",
                        TO_STRING(TABLE_NAME),TO_STRING(TABLE_NAME)));
            rc = sqlite3_exec(db, sequel_1, 0, 0, &err_msg);
            free(sequel_1);
            sequel_1=NULL;
        }

        if (rc != SQLITE_OK ) {
            fprintf(stderr, "INIT_CONNECTION: SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            free(sequel);
            sequel=NULL;
            return NULL;
        }

        ASPRINTF_ERROR(asprintf(&(fifo_buffer), "INIT_CONNECTION: new table %s created\n", TO_STRING(TABLE_NAME)));
        log_this(fifo_buffer,arguments);
        free(fifo_buffer);
        fifo_buffer=NULL;
        
        free(sequel);
        sequel=NULL;
        
        arguments->terminated_by_INIT_CONNECTION=false;
        arguments->conn=db;
        pthread_cond_broadcast(arguments->condition);
    }
    return db;
}

/**
 * Disconnect from the database server
 * \param conn pointer to the current connection
 */
void disconnect(thread_args_t* arguments){
        sqlite3_close(arguments->conn);
}

/**
 * Write an INSERT query to insert a single sensor measurement
 * \param conn pointer to the current connection
 * \param id the sensor id
 * \param value the measurement value
 * \param ts the measurement timestamp
 * \return zero for success, and non-zero if an error occurs
 */
int insert_sensor(thread_args_t* arguments, sensor_id_t id, sensor_value_t value, sensor_ts_t ts){

    char* sequel=0;
    char* err_msg=0;
    asprintf(&sequel,"INSERT INTO %s(sensor_id, sensor_value, timestamp) VALUES(%d, %lf, %ld);", TO_STRING(TABLE_NAME),id,value,ts);
    int rc = sqlite3_exec(arguments->conn, sequel, 0, 0, &err_msg);

    //case if error occurs-> returns 1
    if (rc != SQLITE_OK ) {

        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        free(sequel);
        sequel=NULL;
        return rc;
    }

    free(sequel);
    sequel=NULL;
    return rc;

}


