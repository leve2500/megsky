#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"
#include "ssp_mid.h"

#include "vrp_queue.h"
#include "sgdev_struct.h"
#include "sgdev_param.h"
#include "sgdev_queue.h"
#include "mqtt_pub.h"
#include "mqtt_app.h"
#include "mqtt_dev.h"
#include "mqtt_container.h"
#include "mqtt_json.h"

#include "thread_dev_insert.h"
#include "thread_task_exe.h"
#include "task_deal.h"

uint32_t g_create_tsk_exe_dev_id = 0;
uint32_t g_create_tsk_exe_con_id = 0;
uint32_t g_create_tsk_exe_app_id = 0;

int sg_task_execution_dev_thread(void);
int sg_task_execution_container_thread(void);
int sg_task_execution_app_thread(void);

// 设备升级
static void sg_deal_dev_install_cmd(int32_t mid, char* param)
{
    if (param == NULL) {
        return;
    }
    json_t *json_param = NULL;
    json_param = load_json(param);

    if (json_param == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "load equipment upgrade json(%s) failed.\n", param);
        return;
    }

    device_upgrade_s cmd_obj = { 0 };
    (void)memset_s(&cmd_obj, sizeof(device_upgrade_s), 0, sizeof(device_upgrade_s));
    if (sg_unpack_dev_install_cmd(json_param, &cmd_obj) == VOS_OK) {
        (void)sg_handle_dev_install_cmd(mid, &cmd_obj);
    }

    return;
}

// 设备管理参数修改命令
static void sg_deal_dev_set_para_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "load equipment parameters modification json(%s) failed.\n", param);
        return;
    }

    dev_man_conf_command_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_dev_set_para_cmd(json_param, &cmd_obj)) {
        (void)sg_handle_dev_set_para_cmd(mid, cmd_obj);
    }
    return;
}

// 设备时间同步命令
static void sg_deal_dev_set_time_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_dev_set_time_cmd fail");
        return;
    }
    dev_time_command_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_dev_set_time_cmd(json_param, &cmd_obj)) {
        sg_handle_dev_set_time_cmd(mid, &cmd_obj);
    }
    return;
}

// 设备日志召回
static void sg_deal_dev_log_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_dev_log_cmd fail");
        return;
    }
    dev_log_recall_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_dev_log_cmd(json_param, &cmd_obj)) {
        sg_handle_dev_log_cmd(mid, cmd_obj);
    }
    return;
}

// 设备控制命令
static void sg_deal_dev_ctrl_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_dev_ctrl_cmd fail");
        return;
    }
    char cmd_obj[128];
    if (VOS_OK == sg_unpack_dev_ctrl_cmd(json_param, cmd_obj)) {
        sg_handle_dev_ctrl_cmd(mid, cmd_obj);
    }
    return;
}

// 容器安装控制
static void sg_deal_container_install_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    if (param == NULL) {
        printf("\n param is null,sg_deal_container_install_cmd fail");
        return;
    }
    json_param = load_json(param);

    container_install_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_container_install_cmd(json_param, &cmd_obj)) {
        sg_handle_container_install_cmd(mid, cmd_obj);
    }
    return;
}

// 容器启动
static void sg_deal_container_start_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_start_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_container_control_cmd(json_param, container_name)) {
        sg_handle_container_start_cmd(mid, container_name);
    }
    return;
}

// 容器停止
static void sg_deal_container_stop_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_stop_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_container_control_cmd(json_param, container_name)) {
        sg_handle_container_stop_cmd(mid, container_name);
    }
    return;
}

// 容器删除
static void sg_deal_container_remove_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_remove_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_container_control_cmd(json_param, container_name)) {
        sg_handle_container_delete_cmd(mid, container_name);
    }
    return;
}

// 容器配置修改
static void sg_deal_container_param_set_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_param_set_cmd fail");
        return;
    }
    container_conf_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_container_param_set_cmd(json_param, &cmd_obj)) {   //json解帧  json_param传入  cmd_obj传出
        sg_handle_container_param_set_cmd(mid, &cmd_obj);
    }
    return;
}

// 容器升级
static void sg_deal_container_upgrade_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_upgrade_cmd fail");
        return;
    }
    container_upgrade_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_container_upgrade_cmd(json_param, &cmd_obj)) {
        sg_handle_container_upgrade_cmd(mid, cmd_obj);
    }
    return;
}

// 容器日志召回命令
static void sg_deal_container_log_get_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_log_get_cmd fail");
        return;
    }
    container_log_recall_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_container_log_get_cmd(json_param, &cmd_obj)) {
        sg_handle_container_log_get_cmd(mid, &cmd_obj);
    }
    return;
}

// 应用安装
static void sg_deal_app_install_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_install_cmd fail");
        return;
    }
    app_install_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_install_cmd(json_param, &cmd_obj)) {
        sg_handle_app_install_cmd(mid, cmd_obj);
    }
    return;
}

// 应用启动
static void sg_deal_app_start_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_start_cmd fail");
        return;
    }
    
    app_control_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(json_param, &cmd_obj)) {
        sg_handle_app_start_cmd(mid, &cmd_obj);
    }
    return;
}

// 应用停止
static void sg_deal_app_stop_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_stop_cmd fail");
        return;
    }
    app_control_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(json_param, &cmd_obj)) {
        sg_handle_app_stop_cmd(mid, &cmd_obj);
    }
    return;
}

// 应用卸载
static void sg_deal_app_uninstall_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_uninstall_cmd fail");
        return;
    }
    app_control_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(json_param, &cmd_obj)) {
        sg_handle_app_uninstall_cmd(mid, &cmd_obj);
    }
    return;
}

// 应用使能
static void sg_deal_app_enble_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_enble_cmd fail");
        return;
    }
    app_control_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(json_param, &cmd_obj)) {
        sg_handle_app_enble_cmd(mid, &cmd_obj);
    }
    return;
}

// 应用去使能
static void sg_deal_app_unenble_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_unenble_cmd fail");
        return;
    }
    app_control_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(json_param, &cmd_obj)) {
        sg_handle_app_unenble_cmd(mid, &cmd_obj);
    }
    return;
}

// 应用配置修改命令
static void sg_deal_app_param_set_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("param is null,sg_deal_app_param_set_cmd fail \n");
        return;
    }
    app_conf_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_param_set_cmd(json_param, &cmd_obj)) {
        sg_handle_app_param_set_cmd(mid, &cmd_obj);
    }
    return;
}

// 应用配置状态查询命令
static void sg_deal_app_param_config_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_param_config_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_app_param_get(json_param, container_name)) {
        sg_handle_app_param_get(mid, container_name);
    }
    return;
}

// 应用状态查询命令
static void sg_deal_app_param_status_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_param_status_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_app_status_get(json_param, container_name)) {
        sg_handle_app_status_get(mid, container_name);
    }
    return;
}

// app升级命令
static void sg_deal_app_upgrade_cmd(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("param is null,sg_deal_app_upgrade_cmd fail \n");
        return;
    }
    app_upgrade_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_upgrade_cmd(json_param, &cmd_obj)) {
        sg_handle_app_upgrade_cmd(mid, cmd_obj);
    }
    return;
}

// 应用日志召回命令
static void sg_deal_app_log_get_reply(int32_t mid, char* param)
{
    json_t *json_param = NULL;
    json_param = load_json(param);
    if (NULL == param) {
        printf("param is null,sg_deal_app_log_get_reply fail \n");
        return;
    }
    app_log_recall_cmd_s cmd_obj = { 0 };
    if (VOS_OK == sg_unpack_app_log_get_cmd(json_param, &cmd_obj)) {
        sg_handle_app_log_get_cmd(mid, cmd_obj);
    }
    return;
}

static void sg_task_pro_dev(uint32_t type, int32_t mid, char* param)
{
    switch (type) {
    case TAG_CMD_SYS_UPGRADE:			//设备升级
        (void)sg_deal_dev_install_cmd(mid, param);
        break;
    case TAG_CMD_SYS_STATUS:			//设备状态查询命令
        (void)sg_handle_dev_inquire_cmd(mid);
        break;
    case TAG_CMD_INFO_QUERY:			//设备信息查询命令
        (void)sg_handle_dev_info_cmd(mid);
        break;
    case TAG_CMD_SYS_SET_CONFIG:		//设备管理参数修改命令
        (void)sg_deal_dev_set_para_cmd(mid, param);
        break;
    case TAG_CMD_DATETIME_SYN:			//设备时间同步命令
        (void)sg_deal_dev_set_time_cmd(mid, param);
        break;
    case TAG_CMD_SYS_LOG:               //设备日志召回
        (void)sg_deal_dev_log_cmd(mid, param);
        break;
    case TAG_CMD_CTRL:					//设备控制命令
        (void)sg_deal_dev_ctrl_cmd(mid, param);
        break;
    case TAG_CMD_CON_GET_CONFIG:        //容器配置查询命令
        (void)sg_handle_container_param_get(mid);
        break;
    case TAG_CMD_CON_STATUS:			//容器状态查询命令
        (void)sg_handle_container_status_get(CMD_CON_STATUS, mid);
        break;
    case TAG_CMD_APP_GET_CONFIG:		//应用配置查询命令
        (void)sg_deal_app_param_config_cmd(mid, param);
        break;
    case TAG_CMD_APP_STATUS:			//应用状态查询命令
        (void)sg_deal_app_param_status_cmd(mid, param);
        break;
    default:
        break;
    }

}

static void sg_task_pro_container(uint32_t type, int32_t mid, char* param)
{
    switch (type) {
    case TAG_CMD_CON_INSTALL:          //容器安装控制命令
        (void)sg_deal_container_install_cmd(mid, param);
        break;
    case TAG_CMD_CON_START:				//容器启动控制命令
        (void)sg_deal_container_start_cmd(mid, param);
        break;
    case TAG_CMD_CON_STOP:				//容器停止控制命令
        (void)sg_deal_container_stop_cmd(mid, param);
        break;
    case TAG_CMD_CON_REMOVE:			//容器删除控制
        (void)sg_deal_container_remove_cmd(mid, param);
        break;
    case TAG_CMD_CON_SET_CONFIG:        //容器配置修改命令
        (void)sg_deal_container_param_set_cmd(mid, param);
        break;
    case TAG_CMD_CON_UPGRADE:           //容器升级命令
        (void)sg_deal_container_upgrade_cmd(mid, param);
        break;
    case TAG_CMD_CON_LOG:               //容器日志召回命令
        (void)sg_deal_container_log_get_cmd(mid, param);
        break;
    default:
        break;
    }

}

static void sg_task_pro_app(uint32_t type, int32_t mid, char* param)
{
    switch (type) {
    case TAG_CMD_CON_LOG:				//应用安装控制命令
        (void)sg_deal_app_install_cmd(mid, param);
        break;
    case TAG_CMD_APP_START:				//应用启动
        (void)sg_deal_app_start_cmd(mid, param);
        break;
    case TAG_CMD_APP_STOP:				//应用停止
        (void)sg_deal_app_stop_cmd(mid, param);
        break;
    case TAG_CMD_APP_REMOVE:			//应用卸载
        (void)sg_deal_app_uninstall_cmd(mid, param);
        break;
    case TAG_CMD_APP_ENABLE:			//应用使能
        (void)sg_deal_app_enble_cmd(mid, param);
        break;
    case TAG_CMD_APP_UNENABLE:          //去使能
        (void)sg_deal_app_unenble_cmd(mid, param);
        break;
    case TAG_CMD_APP_SET_CONFIG:        //应用配置修改命令
        (void)sg_deal_app_param_set_cmd(mid, param);
        break;
    case TAG_CMD_APP_UPGRADE:           //app升级命令
        (void)sg_deal_app_upgrade_cmd(mid, param);
        break;
    case TAG_CMD_APP_LOG:				//应用日志召回命令
        (void)sg_deal_app_log_get_reply(mid, param);
        break;
    default:
        break;
    }
}

int sg_init_exe_thread(void)
{
    int i_ret = VOS_OK;
    VOS_UINT32 ret = 0;
    ret = VOS_T_Create(SG_DEVICE_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_dev_thread,                                  //任务执行线程
        &g_create_tsk_exe_dev_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_DEVICE_TASK_NAME, ret);
        i_ret = VOS_ERR;
    }

    ret = VOS_T_Create(SG_CONTAINER_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_container_thread,                            //容器操作类
        &g_create_tsk_exe_con_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_CONTAINER_TASK_NAME, ret);
        i_ret = VOS_ERR;
    }

    ret = VOS_T_Create(SG_APP_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_app_thread,                                   //app操作类
        &g_create_tsk_exe_app_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_APP_TASK_NAME, ret);
        return VOS_ERR;
    }

    return i_ret;
}

int sg_exit_exe_thread(void)
{
    int i_ret = VOS_OK;
    VOS_UINT32 ret = 0;
    ret = VOS_T_Delete(g_create_tsk_exe_dev_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_DEVICE_TASK_NAME, ret);
        i_ret = VOS_ERR;
    }

    ret = VOS_T_Delete(g_create_tsk_exe_con_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_CONTAINER_TASK_NAME, ret);
        i_ret = VOS_ERR;
    }

    ret = VOS_T_Delete(g_create_tsk_exe_app_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_APP_TASK_NAME, ret);
        return VOS_ERR;
    }

    return i_ret;
}

// 任务执行线程 设备及查询类线程函数
int sg_task_execution_dev_thread(void)
{
    int connect_flag;
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_task_execution_dev_thread start.\n");
    uint32_t dev_queue_id = sg_get_que_dev_id();      //获取队列id
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    sg_dev_param_info_s dev_param = sg_get_param();
    char* param = NULL;
    for (; ;) {
        if (dev_param.startmode == MODE_RMTMAN) {
            connect_flag = sg_get_mqtt_connect_flag();  // 更换rmtman 与平台连接标志判断，socket 连接状态判断
        } else {
            connect_flag = sg_get_mqtt_connect_flag();
        }

        if (connect_flag == DEVICE_OFFLINE) {
            (void)VOS_T_Delay(MS_EXECUTE_THREAD_CONNECT_WAIT);
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_task_execution_dev_thread :link disconnected .\n");
            continue;
        }

        if (sg_get_dev_ins_flag() == DEVICE_OFFLINE) {
            (void)VOS_T_Delay(MS_EXECUTE_THREAD_CONNECT_WAIT);
            continue;
        }

        sg_dev_section_info_s  paramcfg = sg_get_section();        // 断面任务
        if (paramcfg.rebootReason == SG_DEV_UPDATE_SECTION_FLAG) {
            (void)sg_handle_dev_install_section(paramcfg);
            (void)VOS_T_Delay(MS_EXECUTE_THREAD_CONNECT_WAIT);
            continue;
        }

        if (VOS_OK == VOS_Que_Read(dev_queue_id, msg, VOS_WAIT, 0)) {
            switch (msg[MSG_ORDER_NUM_ZERO]) {
            case QUEUE_DEVTASK:
                param = (char*)msg[MSG_ORDER_NUM_FIRST];
                (void)sg_task_pro_dev(msg[MSG_ORDER_NUM_SECOND], msg[MSG_ORDER_NUM_THIRD], param);
                if (param != NULL) {
                    (void)free(param);
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(MS_HUNDRED_INTERVAL);
    }
    return VOS_OK;
}

// 容器操作类线程函数
int sg_task_execution_container_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_task_execution_container_thread start.\n");
    sg_dev_param_info_s dev_param = sg_get_param();
    uint32_t container_queue_id = sg_get_que_container_id();      // 获取队列id
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    char* param = NULL;

    for (; ;) {
        if (dev_param.startmode == MODE_RMTMAN) {
            connect_flag = sg_get_mqtt_connect_flag();  // 更换rmtman 与平台连接标志判断，socket 连接状态判断
        } else {
            connect_flag = sg_get_mqtt_connect_flag();
        }

        if (connect_flag == DEVICE_OFFLINE) {
            (void)VOS_T_Delay(MS_EXECUTE_THREAD_CONNECT_WAIT);
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_task_execution_container_thread: link disconnected .\n");
            continue;
        }

        if (sg_get_dev_ins_flag() == DEVICE_OFFLINE) {
            (void)VOS_T_Delay(MS_EXECUTE_THREAD_CONNECT_WAIT);
            continue;
        }
        if (VOS_OK == VOS_Que_Read(container_queue_id, msg, VOS_WAIT, 0)) {
            switch (msg[MSG_ORDER_NUM_ZERO]) {
            case QUEUE_CONTAINERTASK:
                param = (char*)msg[MSG_ORDER_NUM_FIRST];
                (void)sg_task_pro_container(msg[MSG_ORDER_NUM_SECOND], msg[MSG_ORDER_NUM_THIRD], param);
                if (param != NULL) {
                    (void)free(param);
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(MS_HUNDRED_INTERVAL);
    }
    return VOS_OK;
}

// app操作类线程函数
int sg_task_execution_app_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_task_execution_app_thread start.\n");
    uint32_t app_queue_id = sg_get_que_app_id();      // 获取队列id
    sg_dev_param_info_s dev_param = sg_get_param();
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    char* param = NULL;
    for (; ;) {
        if (dev_param.startmode == MODE_RMTMAN) {
            connect_flag = sg_get_mqtt_connect_flag();  // 更换rmtman 与平台连接标志判断，socket 连接状态判断
        } else {
            connect_flag = sg_get_mqtt_connect_flag();
        }

        if (connect_flag == DEVICE_OFFLINE) {
            (void)VOS_T_Delay(MS_EXECUTE_THREAD_CONNECT_WAIT);
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_task_execution_app_thread: link disconnected .\n");
            continue;
        }

        if (sg_get_dev_ins_flag() == DEVICE_OFFLINE) {
            (void)VOS_T_Delay(MS_EXECUTE_THREAD_CONNECT_WAIT);
            continue;
        }

        if (VOS_OK == VOS_Que_Read(app_queue_id, msg, VOS_WAIT, 0)) {
            switch (msg[MSG_ORDER_NUM_ZERO]) {
            case QUEUE_APPTASK:
                param = (char*)msg[MSG_ORDER_NUM_FIRST];
                (void)sg_task_pro_app(msg[MSG_ORDER_NUM_SECOND], msg[MSG_ORDER_NUM_THIRD], param);
                if (param != NULL) {
                    (void)free(param);
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(MS_HUNDRED_INTERVAL);
    }
    return ret;
}
