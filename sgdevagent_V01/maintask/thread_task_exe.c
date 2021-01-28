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
#include "mqtt_json.h"

#include "thread_dev_insert.h"
#include "thread_task_exe.h"
#include "task_deal.h"


//设备升级
static void sg_deal_dev_install_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);

    if (param == NULL) {
        printf("\n param is null,sg_deal_dev_install_cmd fail");
        return;
    }
    device_upgrade_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_dev_install_cmd(jparam, &cmdobj)) {
        sg_handle_dev_install_cmd(mid, cmdobj);
    }
    return;
}

//设备管理参数修改命令
static void sg_deal_dev_set_para_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_dev_set_para_cmd fail");
        return;
    }
    dev_man_conf_command_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_dev_set_para_cmd(jparam, &cmdobj)) {
        sg_handle_dev_set_para_cmd(mid, cmdobj);
    }
    return;
}

//设备时间同步命令
static void sg_deal_dev_set_time_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_dev_set_time_cmd fail");
        return;
    }
    dev_time_command_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_dev_set_time_cmd(jparam, &cmdobj)) {
        sg_handle_dev_set_time_cmd(mid, &cmdobj);
    }
    return;
}

//设备日志召回
static void sg_deal_dev_log_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_dev_log_cmd fail");
        return;
    }
    dev_log_recall_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_dev_log_cmd(jparam, &cmdobj)) {
        sg_handle_dev_log_cmd(mid, cmdobj);
    }
    return;
}

//设备控制命令
static void sg_deal_dev_ctrl_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_dev_ctrl_cmd fail");
        return;
    }
    char cmdobj[128];
    if (VOS_OK == sg_unpack_dev_ctrl_cmd(jparam, cmdobj)) {
        sg_handle_dev_ctrl_cmd(mid, cmdobj);
    }
    return;
}

//容器安装控制
static void sg_deal_container_install_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_install_cmd fail");
        return;
    }
    container_install_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_container_install_cmd(jparam, &cmdobj)) {
        sg_handle_container_install_cmd(mid, cmdobj);
    }
    return;
}

//容器启动
static void sg_deal_container_start_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_start_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_container_control_cmd(jparam, container_name)) {
        sg_handle_container_start_cmd(mid, container_name);
    }
    return;
}

//容器停止
static void sg_deal_container_stop_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_stop_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_container_control_cmd(jparam, container_name)) {
        sg_handle_container_stop_cmd(mid, container_name);
    }
    return;
}


//容器删除
static void sg_deal_container_remove_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_remove_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_container_control_cmd(jparam, container_name)) {
        sg_handle_container_delete_cmd(mid, container_name);
    }
    return;
}


//容器配置修改
static void sg_deal_container_param_set_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_param_set_cmd fail");
        return;
    }
    container_conf_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_container_param_set_cmd(jparam, &cmdobj)) {   //json解帧  jparam传入  cmdobj传出
        sg_handle_container_param_set_cmd(mid, &cmdobj);
    }
    return;
}

//容器升级
static void sg_deal_container_upgrade_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_upgrade_cmd fail");
        return;
    }
    container_upgrade_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_container_upgrade_cmd(jparam, &cmdobj)) {
        sg_handle_container_upgrade_cmd(mid, cmdobj);
    }
    return;
}

//容器日志召回命令
static void sg_deal_container_log_get_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_container_log_get_cmd fail");
        return;
    }
    container_log_recall_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_container_log_get_cmd(jparam, &cmdobj)) {
        sg_handle_container_log_get_cmd(mid, &cmdobj);
    }
    return;
}

//应用安装
static void sg_deal_app_install_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_install_cmd fail");
        return;
    }
    app_install_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_install_cmd(jparam, &cmdobj)) {
        sg_handle_app_install_cmd(mid, cmdobj);
    }
    return;
}
//应用启动
static void sg_deal_app_start_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_start_cmd fail");
        return;
    }
    
    app_control_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(jparam, &cmdobj)) {
        sg_handle_app_start_cmd(mid, &cmdobj);
    }
    return;
}
//应用停止
static void sg_deal_app_stop_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_stop_cmd fail");
        return;
    }
    app_control_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(jparam, &cmdobj)) {
        sg_handle_app_stop_cmd(mid, &cmdobj);
    }
    return;
}
//应用卸载
static void sg_deal_app_uninstall_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_uninstall_cmd fail");
        return;
    }
    app_control_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(jparam, &cmdobj)) {
        sg_handle_app_uninstall_cmd(mid, &cmdobj);
    }
    return;
}
//应用使能
static void sg_deal_app_enble_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_enble_cmd fail");
        return;
    }
    app_control_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(jparam, &cmdobj)) {
        sg_handle_app_enble_cmd(mid, &cmdobj);
    }
    return;
}
//应用去使能
static void sg_deal_app_unenble_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_unenble_cmd fail");
        return;
    }
    app_control_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_control_cmd(jparam, &cmdobj)) {
        sg_handle_app_unenble_cmd(mid, &cmdobj);
    }
    return;
}
//应用配置修改命令
static void sg_deal_app_param_set_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("param is null,sg_deal_app_param_set_cmd fail \n");
        return;
    }
    app_conf_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_param_set_cmd(jparam, &cmdobj)) {
        sg_handle_app_param_set_cmd(mid, &cmdobj);
    }
    return;
}

//应用配置状态查询命令
static void sg_deal_app_param_config_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_param_config_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_app_status_get(jparam, container_name)) {
        sg_handle_app_param_get(mid, container_name);
    }
    return;
}



//应用状态查询命令
static void sg_deal_app_param_status_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("\n param is null,sg_deal_app_param_status_cmd fail");
        return;
    }
    char container_name[128];
    if (VOS_OK == sg_unpack_app_status_get(jparam, container_name)) {
        sg_handle_app_status_get(mid, container_name);
    }
    return;
}



//app升级命令
static void sg_deal_app_upgrade_cmd(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("param is null,sg_deal_app_upgrade_cmd fail \n");
        return;
    }
    app_upgrade_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_upgrade_cmd(jparam, &cmdobj)) {
        sg_handle_app_upgrade_cmd(mid, cmdobj);
    }
    return;
}
//应用日志召回命令
static void sg_deal_app_log_get_reply(int32_t mid, char* param)
{
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == param) {
        printf("param is null,sg_deal_app_log_get_reply fail \n");
        return;
    }
    app_log_recall_cmd_s cmdobj = { 0 };
    if (VOS_OK == sg_unpack_app_log_get_cmd(jparam, &cmdobj)) {
        sg_handle_app_log_get_cmd(mid, cmdobj);
    }
    return;
}

static int sg_task_pro_dev(uint32_t type, int32_t mid, char* param)
{
    switch (type) {
    case TAG_CMD_SYS_UPGRADE:			//设备升级
        sg_deal_dev_install_cmd(mid, param);
        break;
    case TAG_CMD_SYS_STATUS:			//设备状态查询命令
        if (sg_handle_dev_inquire_cmd(mid) != VOS_OK) {
            printf("sg_handle_dev_inquire_cmd error! \n");
            return VOS_ERR;
        }
        break;
    case TAG_CMD_INFO_QUERY:			//设备信息查询命令
        sg_handle_dev_info_cmd(mid);
        printf("TAG_CMD_INFO_QUERY OK!\n");
        break;
    case TAG_CMD_SYS_SET_CONFIG:		//设备管理参数修改命令
        sg_deal_dev_set_para_cmd(mid, param);
        break;
    case TAG_CMD_DATETIME_SYN:			//设备时间同步命令
        sg_deal_dev_set_time_cmd(mid, param);
        break;
    case TAG_CMD_SYS_LOG:               //设备日志召回
        sg_deal_dev_log_cmd(mid, param);
        break;
    case TAG_CMD_CTRL:					//设备控制命令
        sg_deal_dev_ctrl_cmd(mid, param);
        break;
    case TAG_CMD_CON_GET_CONFIG:        //容器配置查询命令
        sg_handle_container_param_get(mid);
        printf("TAG_CMD_CON_GET_CONFIG OK! \n");
        break;
    case TAG_CMD_CON_STATUS:			//容器状态查询命令
        if (sg_handle_container_status_get(CMD_CON_STATUS, mid) != VOS_OK) {
            printf("sg_handle_container_status_get error! \n");
            return VOS_ERR;
        }
        break;
    case TAG_CMD_APP_GET_CONFIG:		//应用配置查询命令
        sg_deal_app_param_config_cmd(mid, param);
        printf("TAG_CMD_APP_STATUS OK! \n");
        break;
    case TAG_CMD_APP_STATUS:			//应用状态查询命令
        sg_deal_app_param_status_cmd(mid, param);
        printf("TAG_CMD_APP_STATUS OK! \n");
        break;
    default:
        break;
    }
    return VOS_OK;
}

static int sg_task_pro_container(uint32_t type, int32_t mid, char* param)
{
    switch (type) {
    case TAG_CMD_CON_INSTALL:          //容器安装控制命令
        sg_deal_container_install_cmd(mid, param);
        break;
    case TAG_CMD_CON_START:				//容器启动控制命令
        sg_deal_container_start_cmd(mid, param);
        break;
    case TAG_CMD_CON_STOP:				//容器停止控制命令
        sg_deal_container_stop_cmd(mid, param);
        break;
    case TAG_CMD_CON_REMOVE:			//容器删除控制
        sg_deal_container_remove_cmd(mid, param);
        break;
    case TAG_CMD_CON_SET_CONFIG:        //容器配置修改命令
        sg_deal_container_param_set_cmd(mid, param);
        break;
    case TAG_CMD_CON_UPGRADE:           //容器升级命令
        sg_deal_container_upgrade_cmd(mid, param);
        break;
    case TAG_CMD_CON_LOG:               //容器日志召回命令
        sg_deal_container_log_get_cmd(mid, param);
        break;
    default:
        break;
    }
    return VOS_OK;
}

static int sg_task_pro_app(uint32_t type, int32_t mid, char* param)
{
    switch (type) {
    case TAG_CMD_CON_LOG:				//应用安装控制命令
        sg_deal_app_install_cmd(mid, param);
        break;
    case TAG_CMD_APP_START:				//应用启动
        sg_deal_app_start_cmd(mid, param);
        break;
    case TAG_CMD_APP_STOP:				//应用停止
        sg_deal_app_stop_cmd(mid, param);
        break;
    case TAG_CMD_APP_REMOVE:			//应用卸载
        sg_deal_app_uninstall_cmd(mid, param);
        break;
    case TAG_CMD_APP_ENABLE:			//应用使能
        sg_deal_app_enble_cmd(mid, param);
        break;
    case TAG_CMD_APP_UNENABLE:          //去使能
        sg_deal_app_unenble_cmd(mid, param);
        break;
    case TAG_CMD_APP_SET_CONFIG:        //应用配置修改命令
        sg_deal_app_param_set_cmd(mid, param);
        break;
    case TAG_CMD_APP_UPGRADE:           //app升级命令
        sg_deal_app_upgrade_cmd(mid, param);
        break;
    case TAG_CMD_APP_LOG:				//应用日志召回命令
        sg_deal_app_log_get_reply(mid, param);
        break;
    default:
        break;
    }
    return VOS_OK;
}
//任务执行线程 设备及查询类
int sg_task_execution_dev_thread(void)
{
    printf("sg_task_execution_dev_thread start. \n");
    int ret = VOS_OK;
    uint32_t dev_queue_id = sg_get_que_dev_id();      //获取队列id
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    char* param = NULL;
    for (; ;) {
        if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
            VOS_T_Delay(1000 * 10);
            continue;
        }
        if (sg_get_dev_ins_flag() == DEVICE_OFFLINE) {
            VOS_T_Delay(1000);
            continue;
        }
        //断面任务？   
        sg_dev_section_info_s  paramcfg = sg_get_section();
        if (1 == paramcfg.rebootReason) {
            sg_handle_dev_install_section(paramcfg);
            continue;
        }
        if (VOS_OK == VOS_Que_Read(dev_queue_id, msg, VOS_WAIT, 0)) {
            switch (msg[0]) {
            case QUEUE_DEVTASK:
                param = (char*)msg[1];
                //处理函数
                printf("megsky --test：unpack_queue  pubtopic==%s. \n", param);
                ret = sg_task_pro_dev(msg[2], msg[3], param);
                printf("**********msg[2] = %d\n", msg[2]);
                if (VOS_OK != ret) {
                    printf("sg_task_pro_dev fail \n");
                }
                if (param != NULL) {
                    free(param);
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(100);
    }
    return ret;
}

//容器操作类
int sg_task_execution_container_thread(void)
{
    printf("sg_task_execution_container_thread start. \n");
    int ret = VOS_OK;
    uint32_t container_queue_id = sg_get_que_container_id();      //获取队列id
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    char* param = NULL;
    for (; ;) {
        if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
            VOS_T_Delay(1000 * 10);
            continue;
        }
        if (1 != sg_get_dev_ins_flag()) {
            VOS_T_Delay(1000);
            continue;
        }
        if (VOS_OK == VOS_Que_Read(container_queue_id, msg, VOS_WAIT, 0)) {
            switch (msg[0]) {
            case QUEUE_CONTAINERTASK:
                param = (char*)msg[1];
                //处理函数
                ret = sg_task_pro_container(msg[2], msg[3], param);
                if (VOS_OK != ret) {
                    printf("sg_task_pro_container fail \n");
                }
                if (param != NULL) {
                    free(param);
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(100);
    }
    return ret;
}

//app操作类
int sg_task_execution_app_thread(void)
{
    printf("sg_task_execution_app_thread start. \n");
    int ret = VOS_OK;
    uint32_t app_queue_id = sg_get_que_app_id();      //获取队列id
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    char* param = NULL;
    for (; ;) {
        if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
            VOS_T_Delay(1000 * 10);
            continue;
        }
        if (1 != sg_get_dev_ins_flag()) {
            VOS_T_Delay(1000);
            continue;
        }
        if (VOS_OK == VOS_Que_Read(app_queue_id, msg, VOS_WAIT, 0)) {
            switch (msg[0]) {
            case QUEUE_APPTASK:
                param = (char*)msg[1];
                //处理函数
                ret = sg_task_pro_app(msg[2], msg[3], param);
                if (VOS_OK != ret) {
                    printf("sg_task_pro_app fail \n");
                }
                if (param != NULL) {
                    free(param);
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(100);
    }
    return ret;
}
