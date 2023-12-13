/**
 * \author Emin Muradov
 */

#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include "config.h"
#include "sbuffer.h"



void sqlmgr_thread(thread_args_t* arguments);

/**
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME  
 * \param clear_up_flag if the table existed, clear up the existing data when clear_up_flag is set to 1
 * \return the connection for success, NULL if an error occurs
 */
DBCONN *init_connection(char clear_up_flag,thread_args_t* arguments);

/**
 * Disconnect from the database server
 * \param conn pointer to the current connection
 */
void disconnect(thread_args_t* arguments);

/**
 * Write an INSERT query to insert a single sensor measurement
 * \param conn pointer to the current connection
 * \param id the sensor id
 * \param value the measurement value
 * \param ts the measurement timestamp
 * \return zero for success, and non-zero if an error occurs
 */
int insert_sensor(thread_args_t* arguments, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);

#endif /* _SENSOR_DB_H_ */
