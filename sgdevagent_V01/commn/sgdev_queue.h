#ifndef _SGDEV_QUEUE_H_
#define _SGDEV_QUEUE_H_


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================
//队列
//=============================================================

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

//*********发布队列*********
void sg_push_pack_item(mqtt_data_info_s *item);          //入列

//*********解析/结果队列*********
void sg_push_unpack_item(mqtt_data_info_s *item);		//解析入列


//*********任务队列*********


//*********任务队列1 设备类及查询类队列*********

void sg_push_dev_item(uint32_t type, int32_t mid, char *param);          //入列

//*********任务队列2 容器操作类队列*********
void sg_push_container_item(uint32_t type, int32_t mid, char *param);    //入列

//*********任务队列3 应用操作类队列*********
void sg_push_app_item(uint32_t type, int32_t mid, char *param);          //入列



#ifdef __cplusplus
}
#endif 

#endif


