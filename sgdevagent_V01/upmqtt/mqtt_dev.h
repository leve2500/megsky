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


#ifndef __SG_MQTT_DEV_H__
#define __SG_MQTT_DEV_H__


#ifdef __cplusplus
extern "C" {
#endif

/*========================设备管理================================== */

//终端向平台发送设备接入请求
void sg_pack_dev_linkup_data(dev_acc_req_s *linkupobj, char *msg);

//设备接入应答(无)
// void sg_pack_dev_linkup_reply(const char *error_msg,char *msg);

//设备主动断开上报
void sg_pack_dev_linkdown_data(char *linkdownobj, const char *error_msg, char *msg);

//设备心跳请求
int sg_pack_dev_heartbeat_request_data(char *msg);

//设备心跳应答
int sg_pack_dev_heartbeat_response_data(char *msg);

//设备升级命令
int sg_unpack_dev_install_cmd(json_t *obj, device_upgrade_s *cmdobj);

//设备升级应答
void sg_pack_dev_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//设备升级状态查询命令   容器安装状态查询命令 应用安装状态查询命令一样
int32_t sg_unpack_dev_install_query(json_t *obj);

//设备升级状态查询应答(容器，应用也一样)
//容器安装状态查询应答 //应用安装状态查询应答
int sg_pack_dev_install_query(uint16_t code, int32_t mid, const char *error_msg, dev_status_reply_s statusobj, char *msg);

//设备升级结果上报  包括容器安装结果上报 应用安装结果上报
void sg_pack_dev_install_result(dev_upgrede_res_reply_s statusobj, const char *error_msg, char *msg);

//设备状态上报
void sg_pack_dev_run_status(dev_sta_reply_s *statusobj, const char *error_msg, char *msg);

//设备状态查询命令 无param不需要解帧

//设备状态查询命令应答
int sg_pack_dev_run_status_reply(uint16_t code, int32_t mid, const char *error_msg, dev_sta_reply_s *statusobj, char *msg);

//设备信息查询命令  无param 不需要解帧

//设备信息查询命令应答
int sg_pack_dev_info_reply(uint16_t code, int32_t mid, const char *error_msg, dev_info_inq_reply_s *statusobj, char *msg);

//设备管理参数修改命令
int sg_unpack_dev_set_para_cmd(json_t *obj, dev_man_conf_command_s *paraobj);

//设备管理参数修改命令应答
void sg_pack_dev_set_para_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//设备时间同步命令
int sg_unpack_dev_set_time_cmd(json_t *obj, dev_time_command_s *timeobj);

//设备时间同步命令应答
void sg_pack_dev_set_time_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

//设备事件上报
void sg_pack_dev_event(dev_thing_reply_s *evenobj, const char *error_msg, char *msg);

//设备日志召回
int sg_unpack_dev_log_cmd(json_t *obj, dev_log_recall_s *logobj);

//设备日志召回应答
void sg_pack_dev_log_reply(uint16_t code, int32_t mid, const char *error_msg, file_info_s *logobj, char *msg);

//设备控制命令
int sg_unpack_dev_ctrl_cmd(json_t *obj, char *ctrlobj);

//设备控制命令应答
void sg_pack_dev_ctrl_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg);

#ifdef __cplusplus
}
#endif 

#endif


