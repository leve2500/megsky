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


#ifndef __JSON_FRAME_H__
#define __JSON_FRAME_H__


#ifdef __cplusplus
extern "C" {
#endif

int sg_unpack_container_install_cmd(json_t *obj, container_install_cmd_s *cmd_obj);                 // 容器安装控制
int sg_pack_container_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg);    // 容器安装控制命令应答

// 容器安装状态查询命令  见设备升级状态查询

// 容器安装状态查询应答  见设备升级状态查询应答

// 容器安装结果上报 见设备升级结果上报

int sg_unpack_container_control_cmd(json_t *obj, char *msg);                                          // 容器控制 (包括启动，停止，删除)
int sg_pack_container_start_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);      // 容器启动控制命令应答
int sg_pack_container_stop_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);       // 容器停止控制应答
int sg_pack_container_remove_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);     // 容器删除控制应答
int sg_unpack_container_param_set_cmd(json_t *obj, container_conf_cmd_s *cmd_obj);                    // 容器配置修改命令
int sg_pack_container_param_set_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);  // 容器配置修改应答

// 容器配置状态查询命令 无param不需要解帧

void sg_pack_container_param_get_reply(uint16_t code, int32_t mid, const char *error_msg,
        container_config_reply_s *statusobj, char *msg);                                              // 容器配置状态查询应答

// 容器状态查询命令  无param不需要解帧

int sg_pack_container_status_get_reply(char * type, uint16_t code, int32_t mid, const char *error_msg,
    container_status_reply_s *statusobj, char *msg);                                                   // 容器状态查询应答
void sg_pack_container_event_pack(uint16_t code, int32_t mid, const char *error_msg,
    container_event_report_s *statusobj, char *msg);                                                   // 容器事件上报
int sg_unpack_container_upgrade_cmd(json_t *obj, container_upgrade_cmd_s *cmd_obj);                    // 容器升级命令
int sg_pack_container_upgrade_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);     // 容器升级命令应答

// 容器升级状态查询命令  见设备升级状态查询命令

// 容器升级状态查询应答  见设备升级状态查询应答

// 容器升级结果上报  见设备升级结果上报

int sg_unpack_container_log_get_cmd(json_t *obj, container_log_recall_cmd_s *cmd_obj);                 // 容器日志召回命令

void sg_pack_container_log_get_reply(uint16_t code, int32_t mid, const char *error_msg, 
        file_info_s *file, char *msg);                                                                 // 容器日志召回应答

#ifdef __cplusplus
}
#endif 

#endif


