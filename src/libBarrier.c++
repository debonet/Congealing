#include "libBarrier.h"
#include "libUtility.h"

#if PTHREAD_BARRIER

int pthread_barrier_init(
	pthread_barrier_t* pbarrier,  
	const pthread_barrierattr_t *pattr, 
	unsigned cRequired
)
{
	// pattr is unused at this point
	pbarrier->m_cRequired = cRequired;
	pbarrier->m_cReached = 0;
	pthread_mutex_init(&pbarrier->m_mutex,NULL);
	pthread_cond_init(&pbarrier->m_cond,NULL);
	return 0;
}

int pthread_barrier_destroy(pthread_barrier_t* pbarrier)
{
	pthread_mutex_destroy(&pbarrier->m_mutex);
	pthread_cond_destroy(&pbarrier->m_cond);
	return 0;
}

int pthread_barrier_wait(pthread_barrier_t* pbarrier)
{
	pthread_mutex_lock(&pbarrier->m_mutex);
	pbarrier->m_cReached++;
	if (pbarrier->m_cReached == pbarrier->m_cRequired) {
		pbarrier->m_cReached = 0;
		pthread_cond_broadcast(&pbarrier->m_cond);
	} else {
		pthread_cond_wait(&pbarrier->m_cond,&pbarrier->m_mutex);
	}
	pthread_mutex_unlock(&pbarrier->m_mutex);
	return 0;
}

#endif
