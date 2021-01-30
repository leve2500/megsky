

/*=====================================================================
 * �ļ���timer_pack.h
 *
 * ���������нӿ� ����json��֡��json��֡
 *
 * ���ߣ�����			2020��9��27��17:10:15
 *
 * �޸ļ�¼��
 =====================================================================*/


#ifndef _TIMER_PACK_H_
#define _TIMER_PACK_H_

#ifdef __cplusplus
extern "C" {
#endif

int sg_timer_pre_create();
// ������ʱ��
int sg_timer_heart_create(uint32_t timeout);
int sg_timer_dev_create(uint32_t timeout);
int sg_timer_container_create(uint32_t timeout);
int sg_timer_app_create(uint32_t timeout);

// ɾ����ʱ��
void sg_timer_heart_delete(void);
void sg_timer_dev_delete(void);
void sg_timer_container_deletee(void);
void sg_timer_app_delete(void);

int sg_get_dev_heart_flag(void);
void sg_set_dev_heart_flag(int flag);

#ifdef __cplusplus
}
#endif 

#endif
