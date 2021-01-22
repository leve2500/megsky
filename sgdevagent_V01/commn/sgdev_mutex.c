//====================================================================
// �ļ���: tzc_mutex
//
// �ļ�����:
// ------------------------------------------------------------------
// ��ƽ̨ͨ�ýӿڻ�����unixƽ̨ʵ��
// ------------------------------------------------------------------
//
// ʱ��: 2020.9
// ���: ����
// ------------------------------------------------------------------
// �޸�˵��(�밴��ʽ˵��)...
//====================================================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sgdev_mutex.h"

pthread_mutex_t m_mutex;

void sg_mutex_init(void)
{
    pthread_mutexattr_t tMutexAttr;
    pthread_mutexattr_init(&tMutexAttr);
    pthread_mutexattr_settype(&tMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_mutex, &tMutexAttr);
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
