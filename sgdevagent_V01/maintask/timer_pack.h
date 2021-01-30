

/*=====================================================================
 * 文件：timer_pack.h
 *
 * 描述：上行接口 包括json组帧，json解帧
 *
 * 作者：田振超			2020年9月27日17:10:15
 *
 * 修改记录：
 =====================================================================*/


#ifndef _TIMER_PACK_H_
#define _TIMER_PACK_H_

#ifdef __cplusplus
extern "C" {
#endif

int sg_timer_pre_create();
// 创建定时器
int sg_timer_heart_create(uint32_t timeout);
int sg_timer_dev_create(uint32_t timeout);
int sg_timer_container_create(uint32_t timeout);
int sg_timer_app_create(uint32_t timeout);

// 删除定时器
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
