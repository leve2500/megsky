#ifndef __SGDEV_QUEUE_H__
#define __SGDEV_QUEUE_H__

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QUEUE_DEPTH_LEN (512)

#define QUEUE_UNPACK_NAME "unpa"
#define QUEUE_PACK_NAME "pack"
#define QUEUE_DEV_NAME "pdev"
#define QUEUE_CONTAINER_NAME "pcon"
#define QUEUE_APP_NAME "papp"
#define QUEUE_RESULT_NAME "prel"

//创建队列
void sg_que_unpack_create(void);
void sg_que_pack_create(void);
void sg_que_task_create(void);
void sg_que_result_create(void);

//删除队列 
void sg_que_unpack_destroy(void);
void sg_que_pack_destroy(void);
void sg_que_task_destroy(void);
void sg_que_result_destroy(void);

uint32_t sg_get_que_pack_id(void);
uint32_t sg_get_que_unpack_id(void);
uint32_t sg_get_que_dev_id(void);
uint32_t sg_get_que_container_id(void);
uint32_t sg_get_que_app_id(void);

void sg_push_pack_item(mqtt_data_info_s *item);          // 入列
void sg_push_unpack_item(mqtt_data_info_s *item);        // 入列
void sg_push_dev_item(uint32_t type, int32_t mid, char *param);          // 入列
void sg_push_container_item(uint32_t type, int32_t mid, char *param);    // 入列
void sg_push_app_item(uint32_t type, int32_t mid, char *param);          // 入列



#ifdef __cplusplus
}
#endif 

#endif


