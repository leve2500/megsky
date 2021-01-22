

/*=====================================================================
 * �ļ���tzc_mutex.h
 *
 * ��������
 *
 * ���ߣ�����			2020��10��12��20:10:37
 *
 * �޸ļ�¼��
 =====================================================================*/


#ifndef _SGDEV_MUTEX_H_
#define _SGDEV_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

void sg_mutex_init(void);
void sg_mutex_exit(void);
bool sg_try_lock(void);
void sg_lock(void);
void sg_unlock(void);



#ifdef __cplusplus
}
#endif 

#endif