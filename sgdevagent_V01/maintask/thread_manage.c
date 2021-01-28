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
#include "mqtt_pub.h"
#include "thread_dev_insert.h"
#include "thread_manage.h"
#include "rmt_socket.h"
#include "rmt_frame.h"

char m_byRecv[DEV_INFO_MSG_BUFFER_LEN * DEV_INFO_MSG_BUFFER_NUM];
char m_bySend[DEV_INFO_MSG_BUFFER_LEN * DEV_INFO_MSG_BUFFER_NUM];


static uint16_t sg_get_continue_send(char *buf)
{
    // int sendFlag;
    uint16_t len;
    uint32_t pack_queue_id = sg_get_que_pack_id();      //获取队列id
    // uint32_t msgType = 0;
    VOS_UINT32 freeRet = 0;
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    mqtt_data_info_s* info = NULL;
    if (NULL != buf) {
        return 0;
    }
    if (VOS_OK == VOS_Que_Read(pack_queue_id, msg, VOS_WAIT, 0)) {
        info = (mqtt_data_info_s*)msg[1];

        len = sg_packframe(buf, info);        //组帧

        //发送失败放到哪里？ 链表中？
        if (info != NULL) {
            // free(info);
            freeRet = VOS_Free(info);
            if (freeRet != VOS_OK) {
                printf("bus_inter_thread VOS_Free fail \n");
            }
        }
    }
    return len;
}
int sg_rpc_read_data_thread(void)
{
    printf("sg_rpc_read_data_thread start. \n");
    int maxlen = DEV_INFO_MSG_BUFFER_LEN * DEV_INFO_MSG_BUFFER_NUM;
    int nRetState = 0;
    int nTmpCount = 0;
    unsigned char byState1 = SOCKET_CONNECTNORMAL;
    unsigned char byState2 = SOCKET_CONNECTNORMAL;
    s_socket_param sock_param = { 0, 0, {0} };
    sock_param.type = TCP_TYPE; //TCP客户端
    sock_param.portNumber = 2406;
    sprintf_s(sock_param.IPAddress, SOCKET16_LEN, "%s", "192.169.11.100");
    sg_set_socket_param(sock_param, 0);
    uint16_t  usPos = 0;
    while (1)
    {
        byState1 = sg_get_connect_state();
        if (SOCKET_CONNECTNORMAL != byState1) {
            VOS_T_Delay(1000 * 5); //重连时间
            nRetState = sg_connect_socket();
            if (SOCKET_CONNECTNORMAL == nRetState) {
                sg_set_mqtt_connect_flag(DEVICE_ONLINE);
            }
        }
        byState1 = sg_get_connect_state();
        if (SOCKET_CONNECTNORMAL == byState1) {
            nTmpCount = sg_read_data(m_byRecv, maxlen);
            if (nTmpCount > 0) {
                if (GetRecvNum() <= 0)
                    continue;
                //赋值
                Recv((uint8_t *)m_byRecv, nTmpCount, usPos);
                usPos = nTmpCount;
                SetRecvCnt(usPos);
                usPos = 0;
                // for(index = 0; index < n; index++)
                // {
                //     printf("buffer[%d] is %0x.\n", index,readbuff[index]);
                // }
                UnPackFrame();
                //给二级缓存，之后解帧
                // printf("\n nTmpCount = %d ",nTmpCount);
                // printf("\n m_byRecv = %s ",m_byRecv);
            }
            byState2 = sg_get_connect_state();
            if (SOCKET_CONNECTNORMAL != byState2) {
                sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
            }
        }
        VOS_T_Delay(10);
    }
    sg_cloese_socket();
    return 0;
}

int sg_rpc_write_data_thread(void)
{
    printf("sg_rpc_write_data_thread start. \n");
    int nTmpCount = 0;
    int maxlen = DEV_INFO_MSG_BUFFER_LEN * DEV_INFO_MSG_BUFFER_NUM;
    unsigned char byState1 = SOCKET_CONNECTNORMAL;
    unsigned char byState2 = SOCKET_CONNECTNORMAL;
    uint16_t ulReqSendNum = 0;
    char *pSendBuf = NULL;
    for (; ;) {
        //发送数据
        byState1 = sg_get_connect_state();
        if (SOCKET_CONNECTNORMAL == byState1) {
            ulReqSendNum = sg_get_continue_send(m_bySend);
            //组帧接口
            if (ulReqSendNum > 0) {
                //cout << "Req Send Number:" << ulReqSendNum << endl;
                if (ulReqSendNum > maxlen) {
                    VOS_T_Delay(10);
                    continue;
                }

                nTmpCount = sg_write_data(m_bySend, (int)(ulReqSendNum));
                if (nTmpCount > 0) {
                    pSendBuf = NULL;
                }
            }
            //检查连接关闭，就要通知上层应用
            byState2 = sg_get_connect_state();
            if (SOCKET_CONNECTNORMAL != byState2) {
                sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
            }
        }
        VOS_T_Delay(100);
    }
}

int sg_mqtt_pub_thread(void)
{
    printf("sg_mqtt_pub_thread start. \n");
    int sendFlag;
    uint32_t pack_queue_id = sg_get_que_pack_id();      //获取队列id
    // uint32_t msgType = 0;
    VOS_UINT32 freeRet = 0;
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };
    mqtt_data_info_s *info = NULL;
    for (; ;) {
        if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
            VOS_T_Delay(1000 * 10);
            continue;
        }
        // if(1 != sg_get_dev_ins_flag()){
        //     VOS_T_Delay(1000);
        //     continue;
        // }
        if (VOS_OK == VOS_Que_Read(pack_queue_id, msg, VOS_WAIT, 0)) {
            printf("megsky --test：sg_mqtt_pub_thread VOS_Que_Read has content. \n");
            switch (msg[0]) {
            case QUEUE_PACK:
                info = (mqtt_data_info_s*)msg[1];
                sendFlag = sg_mqtt_msg_publish(info->msg_send, info->pubtopic);
                //发送失败放到哪里？ 链表中？ 
                if (info != NULL) {
                    (void)VOS_Free(info);
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(100);
    }
    return sendFlag;
}