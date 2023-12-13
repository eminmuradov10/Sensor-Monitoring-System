/**
 * \author Emin Muradov
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_
#define _GNU_SOURCE


#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>


#include "errors.h"


#include "lib/tcpsock.h"
#include "lib/dplist.h"


#define RESET   "\x1b[0m"
#define BLACK   "\x1b[0;30m"
#define RED     "\x1b[0;31m"
#define GREEN   "\x1b[0;32m"
#define YELLOW  "\x1b[0;33m"
#define BLUE    "\x1b[0;34m"
#define PURPLE  "\x1b[0;35m"
#define CYAN    "\x1b[0;36m"


#define FIFONAME "logFifo"
#define GATEWAYPATH "gateway.log"


typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;


#define REAL_TO_STRING(s) #s
#define TO_STRING(s) REAL_TO_STRING(s)    

#ifndef DB_NAME
#define DB_NAME Sensor.db
#endif

#ifndef TABLE_NAME
#define TABLE_NAME SensorData
#endif

typedef int (*callback_t)(void *, int, char **, char **);


struct server_node{
    tcpsock_t* socket_nr;
    sensor_ts_t last_mod;
};
typedef struct server_node server_node_t;

struct sensor_node{
    tcpsock_t* socket_nr;
    sensor_ts_t last_mod;
    
    sensor_id_t sensor_id;
    sensor_value_t sensor_temp;
    sensor_ts_t sensor_timestp;
};
typedef struct sensor_node sensor_node_t;


#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1
#define SBUFFER_NULL_BUFF 2
#define SBUFFER_EMPTY_BUFF 3
#define SBUFFER_NO_NEW_DATA 4

typedef enum {DATAMGR,SQLMGR}type;

struct sbuffer_node {
    bool has_seen_by_DATAMGR;
    bool has_seen_by_SQLMGR;
    struct sbuffer_node *next;  
    sensor_data_t data;         
};
typedef struct sbuffer_node sbuffer_node_t;

typedef struct sbuffer sbuffer_t;


#define DBCONN sqlite3

struct thread_args{
    int port;
    sbuffer_t** buffer;
    pthread_mutex_t* lock;
    pthread_mutex_t* fifolock;
    pthread_cond_t* condition;
    FILE* fifofile;
    bool terminated_by_CONNMGR;
    DBCONN* conn;
    bool terminated_by_INIT_CONNECTION;
};
typedef struct thread_args thread_args_t;

#define MAX 1000

void log_this(char* fifo_buffer,thread_args_t* arguments);

#endif /* _CONFIG_H_ */
