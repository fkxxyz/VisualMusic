#pragma once
#include "stdafx.h"


#ifdef linux

#include <pthread.h>
#include <semaphore.h>

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int value;
	int max_value;
} semaphore_t_arg;

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool setted;
	bool manual_reset;
} event_t_arg;

#define SIZEOF_THREAD_T  sizeof(pthread_t)
#define SIZEOF_MUTEX_T  sizeof(pthread_mutex_t)
#define SIZEOF_COND_T   sizeof(pthread_cond_t)
#define SIZEOF_SEMAPHORE_T  sizeof(semaphore_t_arg)
#define SIZEOF_EVENT_T  sizeof(event_t_arg)

#elif defined(_WIN32) || defined(__WIN32__)



#else

#error Unknow platform.

#endif
