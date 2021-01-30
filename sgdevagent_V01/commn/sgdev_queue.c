#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "vrp.h"
#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_queue.h"
#include "vrp_mem.h"

#include "sgdev_queue.h"
#include "sgdev_mutex.h"

pthread_mutex_t g_pack_mqtt_mutex = PTHREAD_MUTEX_INITIALIZER;

uint32_t g_dev_heartbeat_period = 0;
uint32_t g_create_pack_queue_id = 0;
uint32_t g_create_task_dev_queue_id = 0;
uint32_t g_create_task_container_queue_id = 0;
uint32_t g_create_task_app_queue_id = 0;
uint32_t g_create_task_result_queue_id = 0;

void sg_que_unpack_create(void)
{
    uint32_t ret;
    uint32_t que_depth = QUEUE_DEPTH_LEN;

    ret = VOS_Que_Create(QUEUE_UNPACK_NAME, que_depth, VOS_Q_FIFO | VOS_Q_SYN, &g_dev_heartbeat_period);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_create(%s) fail ret = %d!.\n", QUEUE_UNPACK_NAME, ret);
    }
}

void sg_que_pack_create(void)
{
    uint32_t ret;
    uint32_t que_depth = QUEUE_DEPTH_LEN;

    ret = VOS_Que_Create("pack", que_depth, VOS_Q_FIFO | VOS_Q_SYN, &g_create_pack_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_create(%s) fail ret = %d!.\n", QUEUE_PACK_NAME, ret);
    }
}

void sg_que_task_create(void)
{
    uint32_t ret;
    uint32_t que_depth = QUEUE_DEPTH_LEN;

    ret = VOS_Que_Create(QUEUE_DEV_NAME, que_depth, VOS_Q_FIFO | VOS_Q_SYN, &g_create_task_dev_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_create(%s) fail ret = %d!.\n", QUEUE_DEV_NAME, ret);
    }

    ret = VOS_Que_Create(QUEUE_CONTAINER_NAME, que_depth, VOS_Q_FIFO | VOS_Q_SYN, &g_create_task_container_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_create(%s) fail ret = %d!.\n", QUEUE_CONTAINER_NAME, ret);
    }

    ret = VOS_Que_Create(QUEUE_APP_NAME, que_depth, VOS_Q_FIFO | VOS_Q_SYN, &g_create_task_app_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_create(%s) fail ret = %d!.\n", QUEUE_APP_NAME, ret);
    }
}

void sg_que_result_create(void)
{
    uint32_t ret;
    uint32_t que_depth = QUEUE_DEPTH_LEN;

    ret = VOS_Que_Create(QUEUE_RESULT_NAME, que_depth, VOS_Q_FIFO | VOS_Q_SYN, &g_create_task_result_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_create(%s) fail ret = %d!.\n", QUEUE_RESULT_NAME, ret);
    }
}

void sg_que_unpack_destroy(void)
{
    uint32_t ret;
    ret = VOS_Que_Delete(g_dev_heartbeat_period);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_unpack_destroy(%s) fail ret = %d!.\n", QUEUE_UNPACK_NAME, ret);
    }
}

void sg_que_pack_destroy(void)
{
    uint32_t ret;
    ret = VOS_Que_Delete(g_create_pack_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_unpack_destroy(%s) fail ret = %d!.\n", QUEUE_PACK_NAME, ret);
    }
}

void sg_que_task_destroy(void)
{
    uint32_t ret;
    ret = VOS_Que_Delete(g_create_task_dev_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_unpack_destroy(%s) fail ret = %d!.\n", QUEUE_DEV_NAME, ret);
    }
    ret = VOS_Que_Delete(g_create_task_container_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_unpack_destroy(%s) fail ret = %d!.\n", QUEUE_CONTAINER_NAME, ret);
    }
    ret = VOS_Que_Delete(g_create_task_app_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_unpack_destroy(%s) fail ret = %d!.\n", QUEUE_APP_NAME, ret);
    }
}

void sg_que_result_destroy(void)
{
    uint32_t ret;
    ret = VOS_Que_Delete(g_create_task_result_queue_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_que_unpack_destroy(%s) fail ret = %d!.\n", QUEUE_RESULT_NAME, ret);
    }
}

uint32_t sg_get_que_pack_id(void)
{
    return g_create_pack_queue_id;
}

uint32_t sg_get_que_unpack_id(void)
{
    return g_dev_heartbeat_period;
}

uint32_t sg_get_que_dev_id(void)
{
    return g_create_task_dev_queue_id;
}

uint32_t sg_get_que_container_id(void)
{
    return g_create_task_container_queue_id;
}

uint32_t sg_get_que_app_id(void)
{
    return g_create_task_app_queue_id;
}


void sg_push_pack_item(mqtt_data_info_s *item)
{
    pthread_mutex_lock(&g_pack_mqtt_mutex);

    uint32_t ret;
    uint32_t msg_type = QUEUE_PACK;				 // 定义消息传递类型
    VOS_UINT32 freeRet = 0;
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msg_type;
    msg[1] = (VOS_UINTPTR)item;

    ret = VOS_Que_Write(g_create_pack_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);

    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "topic(%s) que write failed ret:%d.\n", item->pub_topic, ret);
        if (item != NULL) {
            (void)VOS_Free(item);
        }
    }

    pthread_mutex_unlock(&g_pack_mqtt_mutex);
}

void sg_push_unpack_item(mqtt_data_info_s *item)
{
    //订阅回调单一处理，不用锁
    VOS_UINT32 freeRet = 0;
    uint32_t ret;
    uint32_t msg_type = QUEUE_UNPACK;				// 定义消息传递类型
    // mqtt_data_info_s *msgitem = item;			// 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msg_type;
    msg[1] = (VOS_UINTPTR)item;

    ret = VOS_Que_Write(g_dev_heartbeat_period, msg, VOS_NO_WAIT | VOS_NORMAL, 0);
    if (ret != VOS_OK) {
        printf("VOS_Que_Write fail ret = %d \n", ret);
        freeRet = VOS_Free(item);
        if (freeRet != VOS_OK) {
            printf("bus_inter_thread VOS_Free fail \n");
        }
    }
}

void sg_push_dev_item(uint32_t type, int32_t mid, char *param)
{
    //调用此函数时 不会重复
    uint32_t ret;
    uint32_t msg_type = QUEUE_DEVTASK;				// 定义消息传递类型
    // mqtt_data_info_s *msgitem = item;			// 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msg_type;
    msg[1] = (VOS_UINTPTR)param;
    msg[2] = (VOS_UINTPTR)type;
    msg[3] = (VOS_UINTPTR)mid;

    ret = VOS_Que_Write(g_create_task_dev_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);
    if (ret != VOS_OK) {
        printf("\n VOS_Que_Write fail ret = %d", ret);
        free(param);
    }
}

void sg_push_container_item(uint32_t type, int32_t mid, char * param)
{
    uint32_t ret;
    uint32_t msg_type = QUEUE_CONTAINERTASK;				// 定义消息传递类型
    // mqtt_data_info_s *msgitem = item;			    // 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msg_type;
    msg[1] = (VOS_UINTPTR)param;
    msg[2] = (VOS_UINTPTR)type;
    msg[3] = (VOS_UINTPTR)mid;

    ret = VOS_Que_Write(g_create_task_container_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);
    if (ret != VOS_OK) {
        printf("\n VOS_Que_Write fail ret = %d", ret);
        free(param);
    }
}

void sg_push_app_item(uint32_t type, int32_t mid, char *param)
{
    uint32_t ret;
    uint32_t msg_type = QUEUE_APPTASK;				// 定义消息传递类型
    // mqtt_data_info_s *msgitem = item;			// 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msg_type;
    msg[1] = (VOS_UINTPTR)param;
    msg[2] = (VOS_UINTPTR)type;
    msg[3] = (VOS_UINTPTR)mid;
    ret = VOS_Que_Write(g_create_task_app_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);
    if (ret != VOS_OK) {
        printf("\n VOS_Que_Write fail ret = %d", ret);
        free(param);
    }
}









