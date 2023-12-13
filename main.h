/**
 * \author Emin Muradov
 */

#ifndef _MAIN_H_
#define _MAIN_H_

#include "config.h"
#include "datamgr.h"
#include "sensor_db.h"
#include "connmgr.h"
#include "sbuffer.h"

int main(int argc, char *argv[]);
void main_process(int port);
void log_process(void);

#endif //__MAIN_H__
