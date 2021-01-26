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

uint32_t create_unpack_queue_id = 0;
uint32_t create_pack_queue_id = 0;
uint32_t create_task_dev_queue_id = 0;
uint32_t create_task_container_queue_id = 0;
uint32_t create_task_app_queue_id = 0;
uint32_t create_task_result_queue_id = 0;

void sg_que_unpack_create(void)
{
    uint32_t ret;
    uint32_t que_depth = 512;

    ret = VOS_Que_Create("unpa", que_depth, VOS_Q_FIFO | VOS_Q_SYN, &create_unpack_queue_id);
    if (ret != VOS_OK) {
        printf("\n sg_que_create fail ret = %d!", ret);
    }
}

void sg_que_pack_create(void)
{
    uint32_t ret;
    uint32_t que_depth = 512;

    ret = VOS_Que_Create("pack", que_depth, VOS_Q_FIFO | VOS_Q_SYN, &create_pack_queue_id);
    if (ret != VOS_OK) {
        printf("\n sg_que_pack_create fail ret = %d!", ret);
    }
}

void sg_que_task_create(void)
{
    uint32_t ret;
    uint32_t que_depth = 512;

    ret = VOS_Que_Create("pdev", que_depth, VOS_Q_FIFO | VOS_Q_SYN, &create_task_dev_queue_id);
    if (ret != VOS_OK) {
        printf("\n create_task_dev_queue fail ret = %d!", ret);
    }

    ret = VOS_Que_Create("pcon", que_depth, VOS_Q_FIFO | VOS_Q_SYN, &create_task_container_queue_id);
    if (ret != VOS_OK) {
        printf("\n create_task_container_queue fail ret = %d!", ret);
    }

    ret = VOS_Que_Create("papp", que_depth, VOS_Q_FIFO | VOS_Q_SYN, &create_task_app_queue_id);
    if (ret != VOS_OK) {
        printf("\n create_task_app_queue fail ret = %d!", ret);
    }
}

void sg_que_result_create(void)
{
    uint32_t ret;
    uint32_t que_depth = 512;

    ret = VOS_Que_Create("prel", que_depth, VOS_Q_FIFO | VOS_Q_SYN, &create_task_result_queue_id);
    if (ret != VOS_OK) {
        printf("\n create_task_result_queue fail ret = %d!", ret);
    }
}

void sg_que_unpack_destroy(void)
{
    uint32_t ret;
    ret = VOS_Que_Delete(create_unpack_queue_id);
    if (ret != VOS_OK) {
        printf("\n sg_que_unpack_destroy fail ret = %d!", ret);
    }
}

void sg_que_pack_destroy(void)
{
    uint32_t ret;
    ret = VOS_Que_Delete(create_pack_queue_id);
    if (ret != VOS_OK) {
        printf("\n sg_que_pack_destroy fail ret = %d!", ret);
    }
}

void sg_que_task_destroy(void)
{
    uint32_t ret;
    ret = VOS_Que_Delete(create_task_dev_queue_id);
    if (ret != VOS_OK) {
        printf("\n destroy_task_dev_queue_id fail ret = %d!", ret);
    }
    ret = VOS_Que_Delete(create_task_container_queue_id);
    if (ret != VOS_OK) {
        printf("\n destroy_task_container_queue_id fail ret = %d!", ret);
    }
    ret = VOS_Que_Delete(create_task_app_queue_id);
    if (ret != VOS_OK) {
        printf("\n destroy_task_app_queue_id fail ret = %d!", ret);
    }
}

void sg_que_result_destroy(void)
{
    uint32_t ret;
    ret = VOS_Que_Delete(create_task_result_queue_id);
    if (ret != VOS_OK) {
        printf("\n sg_que_result_destroy fail ret = %d!", ret);
    }
}

uint32_t sg_get_que_pack_id(void)
{
    return create_pack_queue_id;
}

uint32_t sg_get_que_unpack_id(void)
{
    return create_unpack_queue_id;
}

uint32_t sg_get_que_dev_id(void)
{
    return create_task_dev_queue_id;
}

uint32_t sg_get_que_container_id(void)
{
    return create_task_container_queue_id;
}

uint32_t sg_get_que_app_id(void)
{
    return create_task_app_queue_id;
}


void sg_push_pack_item(mqtt_data_info_s *item)
{
    //存在多个地方发送队列，必须加锁
    //sg_lock();
    pthread_mutex_lock(&g_pack_mqtt_mutex);
    uint32_t ret;
    uint32_t msgType = QUEUE_PACK;				 // 定义消息传递类型
    VOS_UINT32 freeRet = 0;
    // mqtt_data_info_s *msgitem = item;		 // 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msgType;
    msg[1] = (VOS_UINTPTR)item;

    printf("megsky --test:pack_queue topic====.%s \n ", item->pubtopic);

    ret = VOS_Que_Write(create_pack_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);

    if (ret != VOS_OK) {
        printf("VOS_Que_Write fail ret = %d\n", ret);  //写失败是否要释放item内存？ tianzhenchao
        freeRet = VOS_Free(item);
        if (freeRet != VOS_OK) {
            printf("bus_inter_thread VOS_Free fail \n");
        }
    }
    //sg_unlock();
    pthread_mutex_unlock(&g_pack_mqtt_mutex);
}

void sg_push_unpack_item(mqtt_data_info_s *item)
{
    //订阅回调单一处理，不用锁
    VOS_UINT32 freeRet = 0;
    uint32_t ret;
    uint32_t msgType = QUEUE_UNPACK;				// 定义消息传递类型
    // mqtt_data_info_s *msgitem = item;			// 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msgType;
    msg[1] = (VOS_UINTPTR)item;

    ret = VOS_Que_Write(create_unpack_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);
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
    uint32_t msgType = QUEUE_DEVTASK;				// 定义消息传递类型
    // mqtt_data_info_s *msgitem = item;			// 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msgType;
    msg[1] = (VOS_UINTPTR)param;
    msg[2] = (VOS_UINTPTR)type;
    msg[3] = (VOS_UINTPTR)mid;

    ret = VOS_Que_Write(create_task_dev_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);
    if (ret != VOS_OK) {
        printf("\n VOS_Que_Write fail ret = %d", ret);
        free(param);
    }
}

void sg_push_container_item(uint32_t type, int32_t mid, char * param)
{
    uint32_t ret;
    uint32_t msgType = QUEUE_CONTAINERTASK;				// 定义消息传递类型
    // mqtt_data_info_s *msgitem = item;			    // 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msgType;
    msg[1] = (VOS_UINTPTR)param;
    msg[2] = (VOS_UINTPTR)type;
    msg[3] = (VOS_UINTPTR)mid;

    ret = VOS_Que_Write(create_task_container_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);
    if (ret != VOS_OK) {
        printf("\n VOS_Que_Write fail ret = %d", ret);
        free(param);
    }
}

void sg_push_app_item(uint32_t type, int32_t mid, char *param)
{
    uint32_t ret;
    uint32_t msgType = QUEUE_APPTASK;				// 定义消息传递类型
    // mqtt_data_info_s *msgitem = item;			// 定义消息传递结构
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    msg[0] = (VOS_UINTPTR)msgType;
    msg[1] = (VOS_UINTPTR)param;
    msg[2] = (VOS_UINTPTR)type;
    msg[3] = (VOS_UINTPTR)mid;
    ret = VOS_Que_Write(create_task_app_queue_id, msg, VOS_NO_WAIT | VOS_NORMAL, 0);
    if (ret != VOS_OK) {
        printf("\n VOS_Que_Write fail ret = %d", ret);
        free(param);
    }
}









