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
    pthread_mutexattr_init(&tMutexAttr); //判断返回值 123
    pthread_mutexattr_settype(&tMutexAttr, PTHREAD_MUTEX_RECURSIVE);//判断返回值 123
    pthread_mutex_init(&m_mutex, &tMutexAttr);//判断返回值 123
    pthread_mutexattr_destroy(&tMutexAttr);
}

void sg_mutex_exit(void)
{
    pthread_mutex_destroy(&m_mutex);
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
