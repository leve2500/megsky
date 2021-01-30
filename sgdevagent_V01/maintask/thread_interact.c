#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"

#include "vrp_task.h"
#include "vrp_queue.h"
#include "ssp_mid.h"

#include "sgdev_struct.h"
#include "sgdev_queue.h"
#include "sgdev_debug.h"
#include "mqtt_pub.h"
#include "mqtt_dev.h"
#include "mqtt_app.h"
#include "mqtt_container.h"
#include "mqtt_json.h"

#include "timer_pack.h"
#include "thread_dev_insert.h"
#include "thread_interact.h"

dev_status_reply_s device_upgrade_status = { 0 };
dev_status_reply_s container_install_status = { 0 };
dev_status_reply_s container_upgrade_status = { 0 };
dev_status_reply_s app_install_status = { 0 };
dev_status_reply_s app_upgrade_status = { 0 };

uint8_t g_exe_state = TASK_EXE_STATUS_NULL;
uint32_t g_create_bus_int_id = 0;

int sg_bus_inter_thread(void);

int sg_init_interact_thread(void)
{
    VOS_UINT32 ret = 0;
    ret = VOS_T_Create(SG_INTERACT_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_bus_inter_thread,
        &g_create_bus_int_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_INTERACT_TASK_NAME, ret);
        return VOS_ERR;
    }

    return VOS_OK;
}

int sg_exit_interact_thread(void)
{
    VOS_UINT32 ret = 0;
    ret = VOS_T_Delete(g_create_bus_int_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_INTERACT_TASK_NAME, ret);
        return VOS_ERR;
    }
    
    return VOS_OK;
}

static void sg_pack_dev_upgrade_status(int32_t mid, int32_t jobId)
{
    int16_t code = REQUEST_SUCCESS;
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    char errormsg[DATA_BUF_F256_SIZE] = { 0 };
    if (sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success")) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_pack_dev_upgrade_status:sprintf_s errormsg failed.\n");
        return;
    }

    if (sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_sub_dev_res()) < 0) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_pack_dev_upgrade_status:sprintf_s pub_topic failed.\n");
        return;
    }

    if (jobId != device_upgrade_status.jobId) {
        code = REQUEST_FAILED;
    }

    ret = sg_pack_dev_install_query(code, mid, errormsg, device_upgrade_status, item->msg_send);
    if (ret ！= VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "send device upgrade status failed.\n");
    }

    (void)sg_push_pack_item(item);
}

static void sg_pack_container_upgrade_status(int32_t mid, int32_t jobId)
{
    int16_t code = REQUEST_SUCCESS;
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;

    dev_status_reply_s status = { 0 };
    (void)memset_s(&status, sizeof(dev_status_reply_s), 0, sizeof(dev_status_reply_s));

    if (g_exe_state = TASK_EXE_CONTAINER_INSTALL) {
        status = container_install_status;
    } else if (g_exe_state = TASK_EXE_CONTAINER_UP) {
        status = container_upgrade_status;
    }

    if (jobId != status.jobId) {
        code = REQUEST_FAILED;
    }

    char errormsg[DATA_BUF_F256_SIZE] = { 0 };
    if (sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success")) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_pack_container_upgrade_status:sprintf_s errormsg failed.\n");
        return;
    }

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    if(sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub()) < 0) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_pack_container_upgrade_status:sprintf_s pub_topic failed.\n");
    }
    ret = sg_pack_dev_install_query(code, mid, errormsg, status, item->msg_send);
    if (ret != VOS_OK !) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "send container install or update status failed.\n");
    }
    (void)sg_push_pack_item(item);
}

static void sg_pack_app_upgrade_status(int32_t mid, int32_t jobId)
{
    int16_t code = REQUEST_SUCCESS;
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;
    dev_status_reply_s status;
    if (g_exe_state == TASK_EXE_APP_INSTALL) {
        status = app_install_status;
    } else if (g_exe_state == TASK_EXE_APP_UP) {
        status = app_upgrade_status;
    }

    if (jobId != status.jobId) {
        code = REQUEST_FAILED;
    }

    char errormsg[DATA_BUF_F256_SIZE];
    if (sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success")) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_pack_container_upgrade_status:sprintf_s errormsg failed.\n");
        return;
    }

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    if (sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub()) < 0) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_pack_app_upgrade_status:sprintf_s pub_topic failed.\n");
    }

    ret = sg_pack_dev_install_query(code, mid, errormsg, status, item->msg_send);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "send app install or update status failed.\n");
    }

    (void)sg_push_pack_item(item);
}

static void sg_dev_com_data_unpack(char* param, char* type, int32_t mid)
{
    uint16_t cmd_type = TAG_CMD_NULL;
    int32_t jobId;
    int16_t code = REQUEST_SUCCESS;
    json_t *json_param = NULL;
    if (type == NULL) {
        return;
    }

    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "unpack  command (%s).\n", type);
    if (strncmp(type, CMD_STATUS_QUERY, strlen(CMD_STATUS_QUERY)) == 0) {        // 设备升级状态查询
        if (param == NULL) {
            return;
        }
        json_param = load_json(param);
        if (json_param == NULL) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "unframe the device  message body failed or param is valid.\n", param);
        } else {
            jobId = sg_unpack_dev_install_query(json_param);
            (void)sg_pack_dev_upgrade_status(mid, device_upgrade_status.jobId);
        }
        if (param != NULL) {
            free(param);
        }
    } else {
        if (strncmp(type, CMD_SYS_UPGRADE, strlen(CMD_SYS_UPGRADE)) == 0) {                 // 设备升级命令
            cmd_type = TAG_CMD_SYS_UPGRADE;
            g_exe_state = TASK_EXE_DEV_UP;
        } else if (strncmp(type, CMD_SYS_STATUS, strlen(CMD_SYS_STATUS)) == 0) {            // 设备状态查询命令
            cmd_type = TAG_CMD_SYS_STATUS;
        } else if (strncmp(type, CMD_INFO_QUERY, strlen(CMD_INFO_QUERY)) == 0) {            // 设备信息查询命令
            cmd_type = TAG_CMD_INFO_QUERY;
        } else if (strncmp(type, CMD_SYS_SET_CONFIG, strlen(CMD_SYS_SET_CONFIG)) == 0) {    // 设备管理参数修改命令
            cmd_type = TAG_CMD_SYS_SET_CONFIG;
        } else if (strncmp(type, CMD_DATETIME_SYN, strlen(CMD_DATETIME_SYN)) == 0) {        // 设备时间同步命令
            cmd_type = TAG_CMD_DATETIME_SYN;
        } else if (strncmp(type, CMD_SYS_LOG, strlen(CMD_SYS_LOG)) == 0) {                  // 设备日志召回
            cmd_type = TAG_CMD_SYS_LOG;
        } else if (strncmp(type, CMD_CTRL, strlen(CMD_CTRL)) == 0) {                        // 设备控制命令
            cmd_type = TAG_CMD_CTRL;
        }

        (void)sg_push_dev_item(cmd_type, mid, param);
    }

    if (json_param != NULL) {
        (void)json_decref(json_param);
    }

}

static void sg_dev_res_data_unpack(char *param, char *type)
{
    uint8_t heart_flag = 0;
    if (strncmp(type, EVENT_LINKUP, strlen(EVENT_LINKUP)) == 0) {                       // 上线
        (void)sg_set_dev_ins_flag(DEVICE_ONLINE);
    } else if (strncmp(type, EVENT_HEARTBEAT, strlen(EVENT_HEARTBEAT)) == 0) {          // 心跳应答
        heart_flag = 1;
        (void)sg_set_dev_heart_flag(heart_flag);
    }

     if(param != NULL){
         free(param);
         param = NULL;
     }
}

static void sg_pack_container_param_item(char *type, int32_t mid, char *param)
{
    if (strncmp(type, CMD_CON_GET_CONFIG, strlen(CMD_CON_GET_CONFIG)) == 0) {               // 容器配置查询命令 设备队列中
        cmd_type = TAG_CMD_CON_GET_CONFIG;
        (void)sg_push_dev_item(cmd_type, mid, param);
    } else if (strncmp(type, CMD_CON_STATUS, strlen(CMD_CON_STATUS)) == 0) {                // 容器状态查询命令 设备队列中
        cmd_type = TAG_CMD_CON_STATUS;
        (void)sg_push_dev_item(cmd_type, mid, param);
    } else {
        if (strncmp(type, CMD_CON_INSTALL, strlen(CMD_CON_INSTALL)) == 0) {                 // 容器安装控制命令 
            cmd_type = TAG_CMD_CON_INSTALL;
            g_exe_state = TASK_EXE_CONTAINER_INSTALL;
        } else if (strncmp(type, CMD_CON_START, strlen(CMD_CON_START)) == 0) {              // 容器启动控制命令
            cmd_type = TAG_CMD_CON_START;
        } else if (strncmp(type, CMD_CON_STOP, strlen(CMD_CON_STOP)) == 0) {                // 容器停止控制命令
            cmd_type = TAG_CMD_CON_STOP;
        } else if (strncmp(type, CMD_CON_REMOVE, strlen(CMD_CON_REMOVE)) == 0) {            // 容器删除控制
            cmd_type = TAG_CMD_CON_REMOVE;
        } else if (strncmp(type, CMD_CON_SET_CONFIG, strlen(CMD_CON_SET_CONFIG)) == 0) {    // 容器配置修改命令
            cmd_type = TAG_CMD_CON_SET_CONFIG;
        } else if (strncmp(type, CMD_CON_UPGRADE, strlen(CMD_CON_UPGRADE)) == 0) {          // 容器升级命令
            g_exe_state = TASK_EXE_CONTAINER_UP;
            cmd_type = TAG_CMD_CON_UPGRADE;
        } else if (strncmp(type, CMD_CON_LOG, strlen(CMD_CON_LOG)) == 0) {                  // 容器日志召回命令
            cmd_type = TAG_CMD_CON_LOG;
        }

        (void)sg_push_container_item(cmd_type, mid, param);
    }
}

static void sg_container_com_data_unpack(char *param, char *type, int32_t mid)
{
    uint16_t cmd_type = TAG_CMD_NULL;
    int32_t jobId;
    json_t *json_param = NULL;
    if (type == NULL) {
        return;
    }

    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "unpack  command (%s).\n", type);
    if (strncmp(type, CMD_STATUS_QUERY, strlen(CMD_STATUS_QUERY)) == 0) {                       // 容器升级状态查询命令,容器安装状态查询命令
        if (param == NULL) {
            return;
        }

        json_param = load_json(param);
        if (json_param == NULL) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "unframe the container  message body failed.\n", param);
        } else {
            jobId = sg_unpack_dev_install_query(json_param);
            (void)sg_pack_container_upgrade_status(mid, jobId);
        }

        if (param != NULL) {
            (void)free(param);
            param = NULL;
        }
    } else {
        (void)sg_pack_container_param_item(type, mid, param);
    }

    if (NULL != json_param) {
        (void)json_decref(json_param);//会将所有的子节点都free
    }
}

static void sg_pack_app_param_item(char *type, int32_t mid, char *param)
{
    if (strncmp(type, CMD_APP_GET_CONFIG, strlen(CMD_APP_GET_CONFIG)) == 0) {               // 应用配置查询  设备队列中
        cmd_type = TAG_CMD_APP_GET_CONFIG;
        (void)sg_push_dev_item(cmd_type, mid, param);
    } else if (strncmp(type, CMD_APP_STATUS, strlen(CMD_APP_STATUS)) == 0) {                // 应用状态查询命令  设备队列中
        cmd_type = TAG_CMD_APP_STATUS;
        (void)sg_push_dev_item(cmd_type, mid, param);
    } else {
        if (strncmp(type, CMD_APP_INSTALL, strlen(CMD_APP_INSTALL)) == 0) {                 // 应用安装控制命令
            g_exe_state = TASK_EXE_APP_INSTALL;
            cmd_type = TAG_CMD_CON_LOG;
        } else if (0 == strncmp(type, CMD_APP_START, strlen(CMD_APP_START))) {              // 应用启动
            cmd_type = TAG_CMD_APP_START;
        } else if (0 == strncmp(type, CMD_APP_STOP, strlen(CMD_APP_STOP))) {                // 应用停止
            cmd_type = TAG_CMD_APP_STOP;
        } else if (0 == strncmp(type, CMD_APP_REMOVE, strlen(CMD_APP_REMOVE))) {            // 应用卸载
            cmd_type = TAG_CMD_APP_REMOVE;
        } else if (0 == strncmp(type, CMD_APP_ENABLE, strlen(CMD_APP_ENABLE))) {            // 应用使能
            cmd_type = TAG_CMD_APP_ENABLE;
        } else if (0 == strncmp(type, CMD_APP_UNENABLE, strlen(CMD_APP_UNENABLE))) {        // 去使能
            cmd_type = TAG_CMD_APP_UNENABLE;
        } else if (0 == strncmp(type, CMD_APP_SET_CONFIG, strlen(CMD_APP_SET_CONFIG))) {    // 应用配置修改命令
            cmd_type = TAG_CMD_APP_SET_CONFIG;
        } else if (0 == strncmp(type, CMD_APP_UPGRADE, strlen(CMD_APP_UPGRADE))) {          // 容器升级命令
            g_exe_state = TASK_EXE_APP_UP;
            cmd_type = TAG_CMD_APP_UPGRADE;
        } else if (0 == strncmp(type, CMD_APP_LOG, strlen(CMD_APP_LOG))) {                  // 应用日志召回命令
            cmd_type = TAG_CMD_APP_LOG;
        }

        (void)sg_push_app_item(cmd_type, mid, param);
    }
}

static void sg_app_com_data_unpack(char* param, char *type, int32_t mid)
{
    uint16_t cmd_type = TAG_CMD_NULL;
    int32_t jobId;
    json_t *json_param = NULL;
    if (type == NULL) {
        return;
    }

    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "unpack  command (%s).\n", type);
    if (strncmp(type, CMD_STATUS_QUERY, strlen(CMD_STATUS_QUERY)) == 0) {            // app安装状态查询命令/app升级状态查询命令
        if (param == NULL) {
            return;
        }

        json_param = load_json(param);
        if (NULL == json_param) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "load app json(%s) failed.\n", param);
        } else {
            jobId = sg_unpack_dev_install_query(json_param);
            (void)sg_pack_app_upgrade_status(mid, jobId);
        }

        if (param != NULL) {
            free(param);
            param = NULL;
        }
    } else {
        (void)sg_pack_app_param_item(type, mid, param);
    }

    if (NULL != json_param) {
        (void)json_decref(json_param);
    }
}

static int32_t sg_get_json_content_heaser(uint8_t type, char *msg_type, json_t *param)
{
    if (type == TYPE_REQUEST) {
        header_request = sg_unpack_json_msg_header_request(root);
        param = header_request->param;
        (void)memcpy_s(msg_type, DATA_BUF_F32_SIZE, header_request->type, strlen(header_request->type));
        mid_id = header_request->mid;
        if (header_request != NULL) {
            (void)VOS_Free(header_request);
            header_request = NULL;
        }
    } else if (type == TYPE_REPLY) {
        header_reply = sg_unpack_json_msg_header_reply(root);
        param = header_reply->param;
        (void)memcpy_s(msg_type, 32, header_reply->type, strlen(header_reply->type) + 1);
        mid_id = header_reply->mid;
        if (header_reply != NULL) {
            (void)VOS_Free(header_reply);
            header_reply = NULL;
        }
    }
    return mid_id;
}

static void sg_topic_data_unpack(mqtt_data_info_s *item)
{
    mqtt_request_header_s *header_request = NULL;   // 请求报文头
    mqtt_reply_header_s *header_reply = NULL;       // 应答报文头
    json_t *param = NULL;
    uint8_t type = 0;
    char msg_type[DATA_BUF_F32_SIZE] = { 0 };
    char *srt_pram = NULL;
    int32_t mid_id;
    json_t *root = NULL;
    if (item == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_topic_data_unpack item failed.\n");
        return;
    }

    root = load_json(item->msg_send);
    if (root != NULL) {
        type = msg_parse(root);     // 判断消息类型：根据是否有code判断是请求帧还是应答帧
    }

    mid_id = sg_get_json_content_heaser(type, msg_type, param);

    if (param != NULL) {
        srt_pram = json_dumps(param, JSON_PRESERVE_ORDER);                                                      // 分配内存
    }

    if (strncmp(item->pub_topic, get_topic_sub_dev_com(), strlen(get_topic_sub_dev_com())) == 0) {              // 设备控制命令
        (void)sg_dev_com_data_unpack(srt_pram, msg_type, mid_id);
    } else if (strncmp(item->pub_topic, get_topic_sub_dev_res(), strlen(get_topic_sub_dev_res())) == 0) {       // 设备请求命令
        (void)sg_dev_res_data_unpack(srt_pram, msg_type);
    } else if (strncmp(item->pub_topic, get_topic_sub_ctai_com(), strlen(get_topic_sub_ctai_com())) == 0) {     // 容器控制请求命令
        (void)sg_container_com_data_unpack(srt_pram, msg_type, mid_id);
    } else if (strncmp(item->pub_topic, get_topic_sub_app_com(), strlen(get_topic_sub_app_com())) == 0) {       // app控制请求命令
        (void)sg_app_com_data_unpack(srt_pram, msg_type, mid_id);
    }

    if (root != NULL) {
        (void)json_decref(root);
    }

}

void set_device_upgrade_status(dev_status_reply_s sta)
{
    device_upgrade_status = sta;
}

void set_container_install_status(dev_status_reply_s sta)
{
    container_install_status = sta;
}

void set_container_upgrade_status(dev_status_reply_s sta)
{
    container_upgrade_status = sta;
}

void set_app_install_status(dev_status_reply_s sta)
{
    app_install_status = sta;
}

void set_app_upgrade_status(dev_status_reply_s sta)
{
    app_upgrade_status = sta;
}

// 业务交互线程
int sg_bus_inter_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "business interaction task thread start.\n");
    int bRet = 0;
    int connect_flag;
    VOS_UINT32 freeRet = 0;
    uint32_t unpack_queue_id = sg_get_que_unpack_id();      // 获取队列id
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    sg_dev_param_info_s param = sg_get_param();
    mqtt_data_info_s *item = NULL;
    for (; ;) {
        if (param.startmode == MODE_RMTMAN) {
            connect_flag = sg_get_mqtt_connect_flag();  // 更换rmtman 与平台连接标志判断，socket 连接状态判断
        } else {
            connect_flag = sg_get_mqtt_connect_flag();
        }

        if (connect_flag == DEVICE_OFFLINE) {
            (void)VOS_T_Delay(MS_BUS_THREAD_CONNECT_WAIT);
            continue;
        }

        if (VOS_OK == VOS_Que_Read(unpack_queue_id, msg, VOS_WAIT, 0)) {
            switch (msg[0]) {
            case QUEUE_UNPACK:
                item = (mqtt_data_info_s *)(msg[1]);
                SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "unpack_queue : topic(%s). \n", item->pub_topic);
                (void)sg_topic_data_unpack(item);
                if (item != NULL) {
                    (void)VOS_Free(item);
                }
                break;
            default:
                break;
            }
        }

        (void)VOS_T_Delay(MS_TEN_INTERVAL);
    }
    return bRet;
}

