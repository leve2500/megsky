/*=====================================================================
 *文件：json_frame.h
*
 *描述：上行接口 包括json组帧，json解帧
*
 *作者：田振超			2020年9月27日17:32:05
 *
 *修改记录：
 *
 *遗留工作：数据类型float 转为double  将json_frame.c分为三个小的文件
=====================================================================*/


#ifndef __SG_APP_DEV_H__
#define __SG_APP_DEV_H__


#ifdef __cplusplus
extern "C" {
#endif


//应用安装控制命令
int sg_unpack_app_install_cmd(json_t *obj, app_install_cmd_s *cmdobj);

//应用安装控制命令应答
void sg_pack_app_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//应用安装状态查询命令  见设备升级状态查询命令

//应用安装状态查询应答  见容器安装状态查询应答

//应用安装结果上报 见设备升级结果上报

//应用控制命令  启动  停止 卸载 使能  去使能  合成一个
int sg_unpack_app_control_cmd(json_t *obj, app_control_cmd_s *cmdobj);

//应用启动应答
void sg_pack_app_start_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//应用停止应答
void sg_pack_app_stop_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//应用卸载应答
void sg_pack_app_uninstall_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//应用使能应答
void sg_pack_app_enable_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//应用去使能应答
void sg_pack_app_unenble_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//应用配置修改命令
int sg_unpack_app_param_set_cmd(json_t *obj, app_conf_cmd_s *cmdobj);

//应用配置修改应答
void sg_pack_app_param_set_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//应用配置状态查询命令
int sg_unpack_app_param_get(json_t *obj, char *msg);

//应用配置查询应答
void sg_pack_app_param_get_reply(uint16_t code, int32_t mid, const char *error_msg, app_conf_reply_s *statusobj, char *msg);

//应用状态状态查询命令
int sg_unpack_app_status_get(json_t *obj, char *msg);

//应用状态查询应答
void sg_pack_app_status_get_reply(uint16_t code, int32_t mid, const char *error_msg, app_inq_reply_s *statusobj, char *msg);

//应用状态上报
void sg_pack_app_status_get_data(uint16_t code, int32_t mid, const char *error_msg, app_status_reply_s *statusobj, char *msg);

//应用事件上报
void sg_pack_app_event_pack(uint16_t code, int32_t mid, const char *error_msg, app_event_reply_s *statusobj, char *msg);

//应用升级命令
int sg_unpack_app_upgrade_cmd(json_t *obj, app_upgrade_cmd_s *cmdobj);

//应用升级命令应答
void sg_pack_app_upgrade_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//应用升级状态查询命令  见设备升级状态查询 

//应用升级状态查询应答 见设备升级状态查询应答

//应用升级结果上报  见设备升级结果上报

//应用日志召回命令
int sg_unpack_app_log_get_cmd(json_t *obj, app_log_recall_cmd_s *cmdobj);

//应用日志召回应答
void sg_pack_app_log_get_reply(uint16_t code, int32_t mid, const char *error_msg, file_info_s *file, char *msg);

#ifdef __cplusplus
}
#endif 

#endif


