/*=====================================================================
 *�ļ���json_frame.h
*
 *���������нӿ� ����json��֡��json��֡
*
 *���ߣ�����			2020��9��27��17:32:05
 *
 *�޸ļ�¼��
 *
 *������������������float תΪdouble  ��json_frame.c��Ϊ����С���ļ�
=====================================================================*/


#ifndef __SG_MQTT_DEV_H__
#define __SG_MQTT_DEV_H__


#ifdef __cplusplus
extern "C" {
#endif

/*========================�豸����================================== */

//�ն���ƽ̨�����豸��������
void sg_pack_dev_linkup_data(dev_acc_req_s *linkupobj, char *msg);

//�豸����Ӧ��(��)
// void sg_pack_dev_linkup_reply(const char *error_msg,char *msg);

//�豸�����Ͽ��ϱ�
void sg_pack_dev_linkdown_data(char *linkdownobj, const char *error_msg, char *msg);

//�豸��������
int sg_pack_dev_heartbeat_request_data(char *msg);

//�豸����Ӧ��
int sg_pack_dev_heartbeat_response_data(char *msg);

//�豸��������
int sg_unpack_dev_install_cmd(json_t *obj, device_upgrade_s *cmdobj);

//�豸����Ӧ��
void sg_pack_dev_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//�豸����״̬��ѯ����   ������װ״̬��ѯ���� Ӧ�ð�װ״̬��ѯ����һ��
int32_t sg_unpack_dev_install_query(json_t *obj);

//�豸����״̬��ѯӦ��(������Ӧ��Ҳһ��)
//������װ״̬��ѯӦ�� //Ӧ�ð�װ״̬��ѯӦ��
int sg_pack_dev_install_query(uint16_t code, int32_t mid, const char *error_msg, dev_status_reply_s statusobj, char *msg);

//�豸��������ϱ�  ����������װ����ϱ� Ӧ�ð�װ����ϱ�
void sg_pack_dev_install_result(dev_upgrede_res_reply_s statusobj, const char *error_msg, char *msg);

//�豸״̬�ϱ�
void sg_pack_dev_run_status(dev_sta_reply_s *statusobj, const char *error_msg, char *msg);

//�豸״̬��ѯ���� ��param����Ҫ��֡

//�豸״̬��ѯ����Ӧ��
int sg_pack_dev_run_status_reply(uint16_t code, int32_t mid, const char *error_msg, dev_sta_reply_s *statusobj, char *msg);

//�豸��Ϣ��ѯ����  ��param ����Ҫ��֡

//�豸��Ϣ��ѯ����Ӧ��
int sg_pack_dev_info_reply(uint16_t code, int32_t mid, const char *error_msg, dev_info_inq_reply_s *statusobj, char *msg);

//�豸��������޸�����
int sg_unpack_dev_set_para_cmd(json_t *obj, dev_man_conf_command_s *paraobj);

//�豸��������޸�����Ӧ��
void sg_pack_dev_set_para_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//�豸ʱ��ͬ������
int sg_unpack_dev_set_time_cmd(json_t *obj, dev_time_command_s *timeobj);

//�豸ʱ��ͬ������Ӧ��
void sg_pack_dev_set_time_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//�豸�¼��ϱ�
void sg_pack_dev_event(dev_thing_reply_s *evenobj, const char *error_msg, char *msg);

//�豸��־�ٻ�
int sg_unpack_dev_log_cmd(json_t *obj, dev_log_recall_s *logobj);

//�豸��־�ٻ�Ӧ��
void sg_pack_dev_log_reply(uint16_t code, int32_t mid, const char *error_msg, file_info_s *logobj, char *msg);

//�豸��������
int sg_unpack_dev_ctrl_cmd(json_t *obj, char *ctrlobj);

//�豸��������Ӧ��
void sg_pack_dev_ctrl_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

#ifdef __cplusplus
}
#endif 

#endif


