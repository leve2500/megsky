#ifndef _SGDEV_QUEUE_H_
#define _SGDEV_QUEUE_H_


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================
//����
//=============================================================

//��������
void sg_que_unpack_create(void);
void sg_que_pack_create(void);
void sg_que_task_create(void);
void sg_que_result_create(void);

//ɾ������ 
void sg_que_unpack_destroy(void);
void sg_que_pack_destroy(void);
void sg_que_task_destroy(void);
void sg_que_result_destroy(void);

uint32_t sg_get_que_pack_id(void);
uint32_t sg_get_que_unpack_id(void);
uint32_t sg_get_que_dev_id(void);
uint32_t sg_get_que_container_id(void);
uint32_t sg_get_que_app_id(void);

//*********��������*********
void sg_push_pack_item(mqtt_data_info_s *item);          //����

//*********����/�������*********
void sg_push_unpack_item(mqtt_data_info_s *item);		//��������


//*********�������*********


//*********�������1 �豸�༰��ѯ�����*********

void sg_push_dev_item(uint32_t type, int32_t mid, char *param);          //����

//*********�������2 �������������*********
void sg_push_container_item(uint32_t type, int32_t mid, char *param);    //����

//*********�������3 Ӧ�ò��������*********
void sg_push_app_item(uint32_t type, int32_t mid, char *param);          //����



#ifdef __cplusplus
}
#endif 

#endif


