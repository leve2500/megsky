#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "sgdev_mutex.h"

pthread_mutex_t m_mutex;

void sg_mutex_init(void)
{
    pthread_mutexattr_t tMutexAttr;
    (void)pthread_mutexattr_init(&tMutexAttr); 
    (void)pthread_mutexattr_settype(&tMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    (void)pthread_mutex_init(&m_mutex, &tMutexAttr);
    (void)pthread_mutexattr_destroy(&tMutexAttr);
}

void sg_mutex_exit(void)
{
    (void)pthread_mutex_destroy(&m_mutex);
}

bool sg_try_lock(void)
{
    return (pthread_mutex_trylock(&m_mutex) == 0);
}

void sg_lock(void)
{
    pthread_mutex_lock(&m_mutex);
}

void sg_unlock(void)
{
    pthread_mutex_unlock(&m_mutex);
}
