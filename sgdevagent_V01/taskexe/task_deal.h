

/*=====================================================================
  *�ļ���task_deal.h
 *
  *���������нӿ�
 *
  *���ߣ�����			2020��10��13��20:14:07
  *
  *�޸ļ�¼��

 =====================================================================*/


#ifndef _TASK_DEAL_H_
#define _TASK_DEAL_H_


#ifdef __cplusplus
extern "C" {
#endif



 //�豸��������
int sg_handle_dev_install_cmd(int32_t mid, device_upgrade_s cmdobj);

//�豸�������� ����
void sg_handle_dev_install_section(sg_dev_section_info_s pcfg);

//�豸״̬�ϱ�
int sg_handle_dev_inquire_reply(int32_t mid);

//�豸״̬��ѯ����
int sg_handle_dev_inquire_cmd(int32_t mid);

//�豸��Ϣ��ѯ����  ��param
int sg_handle_dev_info_cmd(int32_t mid);

//�豸��������޸�����
int sg_handle_dev_set_para_cmd(int32_t mid, dev_man_conf_command_s paraobj);

//�豸ʱ��ͬ������
int sg_handle_dev_set_time_cmd(int32_t mid, dev_time_command_s *timeobj);

//�豸��־�ٻ�
void sg_handle_dev_log_cmd(int32_t mid, dev_log_recall_s logobj);

//�豸��������
int sg_handle_dev_ctrl_cmd(int32_t mid, char *action);


/*========================��������================================== */
//������װ����
void sg_handle_container_install_cmd(int32_t mid, container_install_cmd_s cmdobj);

//��������
int sg_handle_container_start_cmd(int32_t mid, char *container_name);

//����ֹͣ
int sg_handle_container_stop_cmd(int32_t mid, char *container_name);

//����ж��
int sg_handle_container_delete_cmd(int32_t mid, char *container_name);

//���������޸�
int sg_handle_container_param_set_cmd(int32_t mid, container_conf_cmd_s *cmdobj);

//�������ò�ѯӦ��
int sg_handle_container_param_get(int32_t mid);

//����״̬��ѯӦ������״̬�ϱ�
int sg_handle_container_status_get(char *type, int32_t mid);

//��������
void sg_handle_container_upgrade_cmd(int32_t mid, container_upgrade_cmd_s cmdobj);

//������־�ٻ�
int sg_handle_container_log_get_cmd(int32_t mid, container_log_recall_cmd_s *cmdobj);


/*========================APP����================================== */

//Ӧ�ð�װ����
void sg_handle_app_install_cmd(int32_t mid, app_install_cmd_s cmdobj);

//Ӧ������
int sg_handle_app_start_cmd(int32_t mid, app_control_cmd_s *cmdobj);

//Ӧ��ֹͣ
int sg_handle_app_stop_cmd(int32_t mid, app_control_cmd_s *cmdobj);

//Ӧ��ж��
int sg_handle_app_uninstall_cmd(int32_t mid, app_control_cmd_s *cmdobj);

//Ӧ��ʹ��
int sg_handle_app_enble_cmd(int32_t mid, app_control_cmd_s *cmdobj);

//Ӧ��ȥʹ��
int sg_handle_app_unenble_cmd(int32_t mid, app_control_cmd_s *cmdobj);

//Ӧ�������޸�
int sg_handle_app_param_set_cmd(int32_t mid, app_conf_cmd_s *cmdobj);

//Ӧ������״̬��ѯ����
void sg_handle_app_param_get(int32_t mid, char *container_name);

//Ӧ��״̬��ѯ����
void sg_handle_app_status_get(int32_t mid, char *container_name);

//Ӧ������
void sg_handle_app_upgrade_cmd(int32_t mid, app_upgrade_cmd_s cmdobj);

//Ӧ����־��ѯ
void sg_handle_app_log_get_cmd(int32_t mid, app_log_recall_cmd_s cmdobj);

#ifdef __cplusplus
}
#endif 

#endif


