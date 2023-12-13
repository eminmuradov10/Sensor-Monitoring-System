/**
 * \author Emin Muradov
 */

#ifndef __CONNMGR_H__
#define __CONNMGR_H__
#ifndef TIMEOUT
#error TIMEOUT not set!
#endif

#include "config.h"
#include "sbuffer.h"

void connmgr_thread(thread_args_t* arguments);
void connmgr_free_list(dplist_t* connections, bool free_element);

#endif //__CONNMGR_H__
