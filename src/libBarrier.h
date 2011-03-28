#ifndef __LIBBARRIER_H__
#define __LIBBARRIER_H__

#if PTHREAD_BARRIER
// TODO: figure out how to know if pthreads.h has 
// already defined these functions

#include <pthread.h>

typedef struct {
	int m_cRequired;
	int m_cReached;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
} pthread_barrier_t;

typedef void pthread_barrierattr_t ;

int pthread_barrier_destroy(
	pthread_barrier_t* pbarrier
);

int pthread_barrier_init(
	pthread_barrier_t* pbarrier,  
	const pthread_barrierattr_t *pattr, 
	unsigned count
);


int pthread_barrier_wait(
	pthread_barrier_t* pbarrier
);

#endif

#endif
