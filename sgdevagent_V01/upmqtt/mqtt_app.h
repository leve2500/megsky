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


#ifndef __SG_APP_DEV_H__
#define __SG_APP_DEV_H__


#ifdef __cplusplus
extern "C" {
#endif


//Ӧ�ð�װ��������
int sg_unpack_app_install_cmd(json_t *obj, app_install_cmd_s *cmdobj);

//Ӧ�ð�װ��������Ӧ��
void sg_pack_app_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//Ӧ�ð�װ״̬��ѯ����  ���豸����״̬��ѯ����

//Ӧ�ð�װ״̬��ѯӦ��  ��������װ״̬��ѯӦ��

//Ӧ�ð�װ����ϱ� ���豸��������ϱ�

//Ӧ�ÿ�������  ����  ֹͣ ж�� ʹ��  ȥʹ��  �ϳ�һ��
int sg_unpack_app_control_cmd(json_t *obj, app_control_cmd_s *cmdobj);

//Ӧ������Ӧ��
void sg_pack_app_start_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//Ӧ��ֹͣӦ��
void sg_pack_app_stop_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//Ӧ��ж��Ӧ��
void sg_pack_app_uninstall_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//Ӧ��ʹ��Ӧ��
void sg_pack_app_enable_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//Ӧ��ȥʹ��Ӧ��
void sg_pack_app_unenble_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//Ӧ�������޸�����
int sg_unpack_app_param_set_cmd(json_t *obj, app_conf_cmd_s *cmdobj);

//Ӧ�������޸�Ӧ��
void sg_pack_app_param_set_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//Ӧ������״̬��ѯ����
int sg_unpack_app_param_get(json_t *obj, char *msg);

//Ӧ�����ò�ѯӦ��
void sg_pack_app_param_get_reply(uint16_t code, int32_t mid, const char *error_msg, app_conf_reply_s *statusobj, char *msg);

//Ӧ��״̬״̬��ѯ����
int sg_unpack_app_status_get(json_t *obj, char *msg);

//Ӧ��״̬��ѯӦ��
void sg_pack_app_status_get_reply(uint16_t code, int32_t mid, const char *error_msg, app_inq_reply_s *statusobj, char *msg);

//Ӧ��״̬�ϱ�
void sg_pack_app_status_get_data(uint16_t code, int32_t mid, const char *error_msg, app_status_reply_s *statusobj, char *msg);

//Ӧ���¼��ϱ�
void sg_pack_app_event_pack(uint16_t code, int32_t mid, const char *error_msg, app_event_reply_s *statusobj, char *msg);

//Ӧ����������
int sg_unpack_app_upgrade_cmd(json_t *obj, app_upgrade_cmd_s *cmdobj);

//Ӧ����������Ӧ��
void sg_pack_app_upgrade_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//Ӧ������״̬��ѯ����  ���豸����״̬��ѯ 

//Ӧ������״̬��ѯӦ�� ���豸����״̬��ѯӦ��

//Ӧ����������ϱ�  ���豸��������ϱ�

//Ӧ����־�ٻ�����
int sg_unpack_app_log_get_cmd(json_t *obj, app_log_recall_cmd_s *cmdobj);

//Ӧ����־�ٻ�Ӧ��
void sg_pack_app_log_get_reply(uint16_t code, int32_t mid, const char *error_msg, file_info_s *file, char *msg);

#ifdef __cplusplus
}
#endif 

#endif


