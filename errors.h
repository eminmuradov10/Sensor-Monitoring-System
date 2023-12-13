/**
 * \author Emin Muradov
 */

#ifndef __errors_h__
#define __errors_h__

#include <errno.h>

#define SYSCALL_ERROR(err) 									\
		do {												\
			if ( (err) == -1 )								\
			{												\
				perror("Error executing syscall");			\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)

#define CHECK_MKFIFO(err) 									\
		do {												\
			if ( (err) == -1 )								\
			{												\
				if ( errno != EEXIST )						\
				{											\
					perror("Error executing mkfifo");		\
					exit( EXIT_FAILURE );					\
				}											\
			}												\
		} while(0)

#define FILE_OPEN_ERROR(fp) 								\
		do {												\
			if ( (fp) == NULL )								\
			{												\
				perror("File open failed");					\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)

#define FILE_CLOSE_ERROR(err) 								\
		do {												\
			if ( (err) == -1 )								\
			{												\
				perror("File close failed");				\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)

#define ASPRINTF_ERROR(err) 								\
		do {												\
			if ( (err) == -1 )								\
			{												\
				perror("asprintf failed");					\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)

#define FFLUSH_ERROR(err) 									\
		do {												\
			if ( (err) == EOF )								\
			{												\
				perror("fflush failed");					\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)

#define TCP_CLOSE_ERROR(err) 								\
		do {												\
			if ( (err) == 1 )								\
			{												\
				perror("tcp_close failed");					\
				exit( EXIT_FAILURE );						\
			}												\
		} while(0)
#endif

