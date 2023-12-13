/**
 * \author Emin Muradov
 */

#define _GNU_SOURCE
#include "main.h" 

void *conn_mgr(void *arg){
    thread_args_t *arguments = arg;
    connmgr_thread(arguments);
    return NULL;
}

void *sql_mgr(void *arg){
    thread_args_t *arguments = arg;
    init_connection(1,arguments); 
    sqlmgr_thread(arguments);
    disconnect(arguments);
    return NULL;
}

void *data_mgr(void *arg){
    thread_args_t* arguments = arg;
    datamgr_thread(arguments);
    return NULL;
}

int main(int argc, char *argv[])
{
    int port = atoi(argv[1]);
    pid_t child_pid;
    child_pid = fork();
    SYSCALL_ERROR(child_pid);
    
    //runs the log process from parent
    if (child_pid != 0){
        printf("\n Parent Process(MAIN PROCESS) (pid=%d)", getpid());
        main_process(port);
    }else{
        printf("\n\n Child Process(LOG PROCESS) (pid=%d)", getpid());
        log_process();
    }

    int child_exit;
    child_pid = wait(&child_exit);
    SYSCALL_ERROR( child_pid );
    printf("\n ========== END  OF  MAIN  PROCESS ========== (pid=%d) \n\n",getpid());
    //printf("\n parent process(the main process) (pid=%d) waited on child process(the log_process) (pid=%d)\n", getpid(), child_pid);

    exit(EXIT_SUCCESS);
}

void main_process(int port)//from lectures
{
    printf("\n ========== START of MAIN PROCESS =========== (pid=%d) \n",getpid());

    pthread_t data_thrd, sql_thrd, conn_thrd;
    void *datamgr_result, *sqlmgr_result, *connmgr_result;
    
    sbuffer_t* buffer;
    sbuffer_init(&buffer);

    thread_args_t *thread_arguments = malloc(sizeof(thread_args_t));        
    
    thread_arguments->port=port;
    
    thread_arguments->buffer=&buffer;

    thread_arguments->lock=malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(thread_arguments->lock , NULL);
    
    thread_arguments->fifolock=malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(thread_arguments->fifolock,NULL);
    
    thread_arguments->condition=malloc(sizeof(pthread_cond_t));
    pthread_cond_init(thread_arguments->condition,NULL);

    thread_arguments->terminated_by_CONNMGR=false;
    thread_arguments->terminated_by_INIT_CONNECTION=false;
    thread_arguments->conn=NULL;
    
    CHECK_MKFIFO(mkfifo(FIFONAME, 0666));
    thread_arguments->fifofile= fopen(FIFONAME, "w");
    FILE_OPEN_ERROR(thread_arguments->fifofile);
    
    char* fifo_buffer;
    ASPRINTF_ERROR(asprintf(&(fifo_buffer),"********** START of LOG PROCESS ********** \n"));
    log_this(fifo_buffer,thread_arguments);
    free(fifo_buffer);
    fifo_buffer=NULL;
    
    pthread_create(&conn_thrd, NULL, &conn_mgr, thread_arguments);
    pthread_create(&data_thrd, NULL, &data_mgr, thread_arguments);
    pthread_create(&sql_thrd, NULL, &sql_mgr, thread_arguments);

    pthread_join(conn_thrd, &connmgr_result);
    pthread_join(data_thrd, &datamgr_result);
    pthread_join(sql_thrd, &sqlmgr_result); 
    
    ASPRINTF_ERROR(asprintf(&(fifo_buffer),"********** END of LOG PROCESS **********"));
    log_this(fifo_buffer,thread_arguments);
    free(fifo_buffer);
    fifo_buffer=NULL;
    
    fclose(thread_arguments->fifofile);
    unlink(FIFONAME);

    pthread_cond_destroy(thread_arguments->condition);
    free(thread_arguments->condition);
    
    pthread_mutex_destroy(thread_arguments->fifolock);
    free(thread_arguments->fifolock);

    pthread_mutex_destroy(thread_arguments->lock);
    free(thread_arguments->lock);
    
    free(thread_arguments);

    sbuffer_free(&buffer);
    return;
}

void log_this(char* fifo_buffer,thread_args_t* arguments){
    pthread_mutex_lock(arguments->fifolock);

    char* writer;
    ASPRINTF_ERROR(asprintf( &writer, "%s",fifo_buffer));
    if(fputs( writer, arguments->fifofile )==EOF){
        //printf("error in placing in fifofile\n");
        exit( EXIT_FAILURE );
    }
    FFLUSH_ERROR(fflush(arguments->fifofile));
    free(writer);
    writer=NULL;

    pthread_mutex_unlock(arguments->fifolock);
}

// log process from lecture codes(fifo_reader.c)
void log_process(){
    printf("\n ========== START of LOG PROCESS ========== (pid=%d) \n\n",getpid());
    
    FILE* fifofile_read = fopen(FIFONAME, "r");
    FILE* logfile_write = fopen(GATEWAYPATH, "a");

    char recbuf[MAX];
    int sequence=0;
    int result;
    char* str_result;
    do{
        str_result=NULL;
        str_result = fgets(recbuf, MAX, fifofile_read);
        if( str_result != NULL ){
            fprintf(logfile_write, "%d %ld %s",sequence,time(NULL), recbuf);
            printf("LOG_PROCESS %d %ld %s\n\n",sequence,time(NULL),recbuf);
            sequence++;
        }
    }while(strcmp(recbuf,"********** END of LOG PROCESS **********")!=0);

    result = fclose(logfile_write);
    FILE_CLOSE_ERROR(result);
    
    result=fclose(fifofile_read);
    FILE_CLOSE_ERROR(result);

    printf("\n ========== END of LOG PROCESS ========== (pid=%d) \n",getpid());
    exit(EXIT_SUCCESS);
}

