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


#ifndef __JSON_FRAME_H__
#define __JSON_FRAME_H__


#ifdef __cplusplus
extern "C" {
#endif

int sg_unpack_container_install_cmd(json_t *obj, container_install_cmd_s *cmd_obj);                 // ������װ����
int sg_pack_container_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg);    // ������װ��������Ӧ��

// ������װ״̬��ѯ����  ���豸����״̬��ѯ

// ������װ״̬��ѯӦ��  ���豸����״̬��ѯӦ��

// ������װ����ϱ� ���豸��������ϱ�

int sg_unpack_container_control_cmd(json_t *obj, char *msg);                                          // �������� (����������ֹͣ��ɾ��)
int sg_pack_container_start_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);      // ����������������Ӧ��
int sg_pack_container_stop_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);       // ����ֹͣ����Ӧ��
int sg_pack_container_remove_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);     // ����ɾ������Ӧ��
int sg_unpack_container_param_set_cmd(json_t *obj, container_conf_cmd_s *cmd_obj);                    // ���������޸�����
int sg_pack_container_param_set_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);  // ���������޸�Ӧ��

// ��������״̬��ѯ���� ��param����Ҫ��֡

void sg_pack_container_param_get_reply(uint16_t code, int32_t mid, const char *error_msg,
        container_config_reply_s *statusobj, char *msg);                                              // ��������״̬��ѯӦ��

// ����״̬��ѯ����  ��param����Ҫ��֡

int sg_pack_container_status_get_reply(char * type, uint16_t code, int32_t mid, const char *error_msg,
    container_status_reply_s *statusobj, char *msg);                                                   // ����״̬��ѯӦ��
void sg_pack_container_event_pack(uint16_t code, int32_t mid, const char *error_msg,
    container_event_report_s *statusobj, char *msg);                                                   // �����¼��ϱ�
int sg_unpack_container_upgrade_cmd(json_t *obj, container_upgrade_cmd_s *cmd_obj);                    // ������������
int sg_pack_container_upgrade_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);     // ������������Ӧ��

// ��������״̬��ѯ����  ���豸����״̬��ѯ����

// ��������״̬��ѯӦ��  ���豸����״̬��ѯӦ��

// ������������ϱ�  ���豸��������ϱ�

int sg_unpack_container_log_get_cmd(json_t *obj, container_log_recall_cmd_s *cmd_obj);                 // ������־�ٻ�����

void sg_pack_container_log_get_reply(uint16_t code, int32_t mid, const char *error_msg, 
        file_info_s *file, char *msg);                                                                 // ������־�ٻ�Ӧ��

#ifdef __cplusplus
}
#endif 

#endif


