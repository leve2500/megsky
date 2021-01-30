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
#include "sgdev_queue.h"
#include "sgdev_param.h"
#include "sgdev_debug.h"

#include "mqtt_pub.h"
#include "thread_dev_insert.h"
#include "thread_manage.h"
#include "rmt_socket.h"
#include "rmt_frame.h"

char g_by_recv[DEV_INFO_MSG_BUFFER_LEN * DEV_INFO_MSG_BUFFER_NUM];
char g_by_send[DEV_INFO_MSG_BUFFER_LEN * DEV_INFO_MSG_BUFFER_NUM];
uint32_t g_create_mqt_pub_id = 0;
uint32_t g_create_socket_read_id = 0;
uint32_t g_create_socket_write_id = 0;

int g_rmt_connect_flag = 0;

int sg_mqtt_pub_thread(void);
int sg_rpc_read_data_thread(void);
int sg_rpc_write_data_thread(void);

int sg_init_data_manager_thread(void)
{
    int i_ret = VOS_OK;
    VOS_UINT32 ret = 0;
    sg_dev_param_info_s param = sg_get_param();

    if (param.startmode == MODE_RMTMAN) {
        ret = VOS_T_Create(SG_RPC_READ_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_rpc_read_data_thread,
            &g_create_socket_read_id);
        if (ret != VOS_OK) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_RPC_READ_TASK_NAME, ret);
            i_ret = VOS_ERR;
        }
        ret = VOS_T_Create(SG_RPC_WRITE_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_rpc_write_data_thread,
            &g_create_socket_write_id);
        if (ret != VOS_OK) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_RPC_WRITE_TASK_NAME, ret);
            i_ret = VOS_ERR;
        }
    } else {
        ret = VOS_T_Create(SG_MQTT_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_mqtt_pub_thread,
            &g_create_mqt_pub_id);
        if (ret != VOS_OK) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_MQTT_TASK_NAME, ret);
            return VOS_ERR;
        }
    }

    return i_ret;
}

int sg_exit_data_manager_thread(void)
{
    int i_ret = VOS_OK;
    VOS_UINT32 ret = 0;
    sg_dev_param_info_s param = sg_get_param();
    if (param.startmode == MODE_RMTMAN) {
        ret = VOS_T_Delete(g_create_socket_read_id);
        if (ret != VOS_OK) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_RPC_READ_TASK_NAME, ret);
            i_ret = VOS_ERR;
        }
        ret = VOS_T_Delete(g_create_socket_write_id);
        if (ret != VOS_OK) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_RPC_WRITE_TASK_NAME, ret);
            i_ret = VOS_ERR;
        }
    } else {
        ret = (int)VOS_T_Delete(g_create_mqt_pub_id);
        if (ret != VOS_OK) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_MQTT_TASK_NAME, ret);
            return VOS_ERR;
        }
    }

    return i_ret;
}

static uint16_t sg_get_continue_send(char *buf)
{
    uint16_t len;
    uint32_t pack_queue_id = sg_get_que_pack_id();
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    mqtt_data_info_s* info = NULL;
    if (buf != NULL) {
        return 0;
    }

    if (VOS_Que_Read(pack_queue_id, msg, VOS_WAIT, 0) == VOS_OK) {
        info = (mqtt_data_info_s*)msg[1];
        len = sg_packframe(buf, info);        // 组帧
        if (info != NULL) {
            (void)VOS_Free(info);
        }
    }

    return len;
}

int sg_rpc_read_data_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_rpc_read_data_thread start.\n");
    uint16_t  us_pos = 0;
    int maxlen = DEV_INFO_MSG_BUFFER_LEN * DEV_INFO_MSG_BUFFER_NUM;
    int n_ret_state = 0;
    int n_tmp_count = 0;
    unsigned char by_state1 = SOCKET_CONNECTNORMAL;
    unsigned char by_state2 = SOCKET_CONNECTNORMAL;
    s_socket_param sock_param = { 0, 0, {0} };
    sock_param.type = TCP_TYPE; // TCP客户端
    sock_param.port_number = 2406;
    if (sprintf_s(sock_param.ip_address, SOCKET16_LEN, "%s", "192.169.11.100") < 0) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_rpc_read_data_thread start.\n");
        return VOS_ERR;
    }

    (void)sg_set_socket_param(sock_param, 0);
    for (; ;) {
        (void)VOS_T_Delay(MS_HUNDRED_INTERVAL);
        by_state1 = sg_get_connect_state();
        if (SOCKET_CONNECTNORMAL != by_state1) {
            (void)VOS_T_Delay(MS_MGR_THREAD_CONNECT_WAIT); // 重连时间
            n_ret_state = sg_connect_socket();
            if (SOCKET_CONNECTNORMAL == n_ret_state) {
                (void)sg_set_rpc_connect_flag(DEVICE_ONLINE);
            }
        }
        by_state1 = sg_get_connect_state();
        if (SOCKET_CONNECTNORMAL == by_state1) {
            n_tmp_count = sg_read_data(g_by_recv, maxlen);
            if (n_tmp_count > 0) {
                (void)sg_recvive((uint8_t *)g_by_recv, n_tmp_count, us_pos);
                us_pos = n_tmp_count;
                (void)sg_set_recv_cnt(us_pos);
                us_pos = 0;
                sg_unpack_frame();
            }
            if (n_tmp_count > 0 && sg_get_recvive_num() <= 0) {
                continue;
            }
            if (sg_get_connect_state() != SOCKET_CONNECTNORMAL) {
                sg_set_rpc_connect_flag(DEVICE_OFFLINE);
            }
        }
    }

    sg_cloese_socket();
    return 0;
}

int sg_rpc_write_data_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_rpc_write_data_thread start.\n");
    int n_tmp_count = 0;
    int maxlen = DEV_INFO_MSG_BUFFER_LEN * DEV_INFO_MSG_BUFFER_NUM;
    unsigned char by_state1 = SOCKET_CONNECTNORMAL;
    uint16_t ul_req_send_num = 0;
    char *p_send_buf = NULL;
    for (; ;) {
        VOS_T_Delay(MS_HUNDRED_INTERVAL);
        by_state1 = sg_get_connect_state();
        if (by_state1 != SOCKET_CONNECTNORMAL) {
            continue;
        }
        ul_req_send_num = sg_get_continue_send(g_by_send);
        if (ul_req_send_num > 0) {
            if (ul_req_send_num > maxlen) {
                continue;
            }

            n_tmp_count = sg_write_data(g_by_send, (int)(ul_req_send_num));
            if (n_tmp_count > 0) {
                p_send_buf = NULL;
            }
        }

        if (sg_get_connect_state() != SOCKET_CONNECTNORMAL) {
            sg_set_rpc_connect_flag(DEVICE_OFFLINE);
        }

    }
}

int sg_mqtt_pub_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_mqtt_pub_thread start.\n");
    int send_flag;
    uint32_t pack_queue_id = sg_get_que_pack_id();
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    mqtt_data_info_s *info = NULL;
    for (; ;) {
        if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
            VOS_T_Delay(1000 * 10);
            continue;
        }
        if (VOS_OK == VOS_Que_Read(pack_queue_id, msg, VOS_WAIT, 0)) {
            printf("megsky --test：sg_mqtt_pub_thread VOS_Que_Read has content. \n");
            switch (msg[0]) {
            case QUEUE_PACK:
                info = (mqtt_data_info_s*)msg[1];
                send_flag = sg_mqtt_msg_publish(info->msg_send, info->pub_topic);                // 发送失败?
                if (info != NULL) {
                    (void)VOS_Free(info);
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(MS_HUNDRED_INTERVAL);
    }
    return send_flag;
}
void sg_set_rpc_connect_flag(int flag)
{
    g_rmt_connect_flag = flag;
    if (g_rmt_connect_flag == DEVICE_OFFLINE) {
        sg_set_dev_ins_flag(DEVICE_OFFLINE);
    }
}

int sg_get_rpc_connect_flag(void)
{
    return g_rmt_connect_flag;
}
