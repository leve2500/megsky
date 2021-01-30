

/*=====================================================================
  *文件：task_deal.h
 *
  *描述：下行接口
 *
  *作者：田振超			2020年10月13日20:14:07
  *
  *修改记录：

 =====================================================================*/


#ifndef _TASK_DEAL_H_
#define _TASK_DEAL_H_


#ifdef __cplusplus
extern "C" {
#endif



 //设备升级命令
int sg_handle_dev_install_cmd(int32_t mid, device_upgrade_s *cmd_obj);

//设备升级命令 断面
void sg_handle_dev_install_section(sg_dev_section_info_s pcfg);

//设备状态上报
int sg_handle_dev_inquire_reply(int32_t mid);

//设备状态查询命令
int sg_handle_dev_inquire_cmd(int32_t mid);

//设备信息查询命令  无param
int sg_handle_dev_info_cmd(int32_t mid);

//设备管理参数修改命令
int sg_handle_dev_set_para_cmd(int32_t mid, dev_man_conf_command_s paraobj);

//设备时间同步命令
int sg_handle_dev_set_time_cmd(int32_t mid, dev_time_command_s *timeobj);

//设备日志召回
void sg_handle_dev_log_cmd(int32_t mid, dev_log_recall_s logobj);

//设备控制命令
int sg_handle_dev_ctrl_cmd(int32_t mid, char *action);


/*========================容器管理================================== */
//容器安装控制
void sg_handle_container_install_cmd(int32_t mid, container_install_cmd_s cmd_obj);

//容器启动
int sg_handle_container_start_cmd(int32_t mid, char *container_name);

//容器停止
int sg_handle_container_stop_cmd(int32_t mid, char *container_name);

//容器卸载
int sg_handle_container_delete_cmd(int32_t mid, char *container_name);

//容器配置修改
int sg_handle_container_param_set_cmd(int32_t mid, container_conf_cmd_s *cmd_obj);

//容器配置查询应答
int sg_handle_container_param_get(int32_t mid);

//容器状态查询应答，容器状态上报
int sg_handle_container_status_get(char *type, int32_t mid);

//容器升级
void sg_handle_container_upgrade_cmd(int32_t mid, container_upgrade_cmd_s cmd_obj);

//容器日志召回
int sg_handle_container_log_get_cmd(int32_t mid, container_log_recall_cmd_s *cmd_obj);


/*========================APP管理================================== */

//应用安装控制
void sg_handle_app_install_cmd(int32_t mid, app_install_cmd_s cmd_obj);

//应用启动
int sg_handle_app_start_cmd(int32_t mid, app_control_cmd_s *cmd_obj);

//应用停止
int sg_handle_app_stop_cmd(int32_t mid, app_control_cmd_s *cmd_obj);

//应用卸载
int sg_handle_app_uninstall_cmd(int32_t mid, app_control_cmd_s *cmd_obj);

//应用使能
int sg_handle_app_enble_cmd(int32_t mid, app_control_cmd_s *cmd_obj);

//应用去使能
int sg_handle_app_unenble_cmd(int32_t mid, app_control_cmd_s *cmd_obj);

//应用配置修改
int sg_handle_app_param_set_cmd(int32_t mid, app_conf_cmd_s *cmd_obj);

//应用配置状态查询命令
void sg_handle_app_param_get(int32_t mid, char *container_name);

//应用状态查询命令
void sg_handle_app_status_get(int32_t mid, char *container_name);

//应用升级
void sg_handle_app_upgrade_cmd(int32_t mid, app_upgrade_cmd_s cmd_obj);

//应用日志查询
void sg_handle_app_log_get_cmd(int32_t mid, app_log_recall_cmd_s cmd_obj);

#ifdef __cplusplus
}
#endif 

#endif


