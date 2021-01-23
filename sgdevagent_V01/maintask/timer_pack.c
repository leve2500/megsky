#include "vrp_queue.h"
#include "vrp_task.h"
#include "vrp_timer.h"
#include "timer_pack.h"
#include "mqtt_json.h"
#include "mqtt_dev.h"
#include "mqtt_pub.h"
#include "sgdev_queue.h"
#include "thread_dev_insert.h"
#include "task_deal.h"
//示例
unsigned int g_timer_task_id = 0;
unsigned int g_timer_queue_id = 0;

uint32_t create_time_heart_id = VOS_NULL_LONG;
uint32_t create_time_dev_id = VOS_NULL_LONG;
uint32_t create_time_container_id = VOS_NULL_LONG;
uint32_t create_time_app_id = VOS_NULL_LONG;

int heartflag = 0;
int heart_send_count = 0;


//心跳定时器
static int sg_timer_heart_callback(void)
{
    printf("megsky --test：sg_timer_heart_callback.\n");
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;

    if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE)
        return VOS_ERR;

    if (sg_get_dev_ins_flag() == DEVICE_OFFLINE) {
        printf("megsky --test：device insert offine.\n");
        return VOS_ERR;
    }

    if (heartflag == 1) {
        heart_send_count = 0;
        heartflag = 0;
    }
    if (heart_send_count > HEART_MAX_COUNT) {
        (void)sg_set_dev_ins_flag(DEVICE_OFFLINE);
        heart_send_count = 0;
        //发送设备断开命令？ 不发送
        //日志保存
        printf("The number of heartbeats sent reaches a limit  DEVICE_OFFLINE.\n");
        // ssp_syslog(LOG_INFO, SYSLOG_LOG, EG_MODULE, "The number of heartbeats sent reaches a limit.");
    }

    if (DEVICE_ONLINE != sg_get_dev_ins_flag()) {
        return VOS_ERR;
    }

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_request_pub());
    ret = sg_pack_dev_heartbeat_request_data(item->msg_send);
    if (VOS_OK != ret) {
        printf("sg_pack_dev_heartbeat_request_data fail.\n");
        return VOS_ERR;
    }

    sg_push_pack_item(item);        //入列
    heart_send_count++;
    printf("megsky --test：heart_send_count = %d.\n", heart_send_count);
    return ret;
}

//设备状态刷新定时器
static int sg_timer_dev_callback(void)
{
    printf("megsky --sg_timer_dev_callback.\n");
    //定时器启动删除在外面控制,启动则干活
    if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
        return VOS_ERR;
    }
    if (sg_get_dev_ins_flag() == DEVICE_OFFLINE) {
        return VOS_ERR;
    }
    if (sg_handle_dev_inquire_reply(0) != VOS_OK) {
        printf("sg_handle_container_status_get error! \n");
        return VOS_ERR;
    }
    return VOS_OK;
}

//容器状态刷新定时器
static int sg_timer_container_callback(void)
{
    printf("megsky --sg_timer_container_callback. \n");
    if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
        return VOS_ERR;
    }
    if (sg_get_dev_ins_flag() == DEVICE_OFFLINE) {
        return VOS_ERR;
    }
    if (sg_handle_container_status_get(REP_CON_STATUS, 0) != VOS_OK) {
        printf("sg_handle_container_status_get error! \n");
        return VOS_ERR;
    }
    return VOS_OK;
}

//APP状态刷新定时器
static int sg_timer_app_callback(void)
{
    printf("megsky --sg_timer_app_callback. \n");
    if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
        return VOS_ERR;
    }
    if (sg_get_dev_ins_flag() == DEVICE_OFFLINE) {
        return VOS_ERR;
    }
    return VOS_OK;
}

static void sg_timer_queue_entry(void)
{
    unsigned int ret;
    uintptr_t msg[VOS_QUEUE_MSG_NUM];
    for (; ;) {
        (void)memset_s((void *)msg, sizeof(msg), 0, sizeof(msg));
        ret = VOS_Que_Read(g_timer_queue_id, msg, VOS_WAIT, 0);
        if (!ret) {
            if (((VOS_TIMERMSG_S *)msg)->tm_pfFunc) {
                (void)(((VOS_TIMERMSG_S *)msg)->tm_pfFunc)(((VOS_TIMERMSG_S *)msg)->tm_pArg);
            }
        } else {
            break;
        }
    }
}

static void sg_timer_task_entry(void)
{
    unsigned int ret;
    unsigned int event;
    for (; ;) {
        ret = VOS_Ev_Read(VOS_TIMER_EVENT, &event, VOS_WAIT, 0);
        if (!ret) {
            if (event & VOS_TIMER_EVENT) {
                sg_timer_queue_entry();
            }
        }
    }
}

static int sg_timer_task_create(void)
{
    unsigned int ret;
    ret = VOS_T_Create("sgtm", 0, 0, 0, NULL, (TaskStartAddress_PF)sg_timer_task_entry, &g_timer_task_id);
    if (ret) {
        return -1;
    }
    return 0;
}
static int sg_timer_queue_create(void)
{
    unsigned int ret;
    ret = VOS_Que_Create("sgtm", 0, VOS_Q_SYN | VOS_Q_FIFO, &g_timer_queue_id);
    if (ret) {
        return -1;
    }
    return 0;
}


int sg_timer_pre_create()
{
    //创建任务
    if (sg_timer_task_create() != 0) {
        return -1;
    }
    if (sg_timer_queue_create() != 0) {
        return -1;
    }
    return VOS_OK;
}

int sg_timer_heart_create(uint32_t timeout)
{
    uint32_t ret;
    uint32_t time_out = timeout * 1000; //  ms

    if (VOS_NULL_LONG != create_time_heart_id) {
        if (VOS_OK != VOS_Timer_Delete(create_time_heart_id)) {
            printf("VOS_Timer_Delete create_time_heart_id fail!\n");
        } else {
            create_time_heart_id = VOS_NULL_LONG;
        }
    }
    ret = VOS_Timer_Create(g_timer_task_id, g_timer_queue_id, time_out, (void(*) (void *))sg_timer_heart_callback, NULL,
        &create_time_heart_id, VOS_TIMER_LOOP);
    if (ret != VOS_OK) {
        printf("sg_timer_heart_create fail!\n");
        return -1;
    }
    return VOS_OK;
}

int sg_timer_dev_create(uint32_t timeout)
{
    uint32_t ret;
    uint32_t time_out = timeout * 1000; //  ms

    if (VOS_NULL_LONG != create_time_dev_id)
    {
        if (VOS_OK != VOS_Timer_Delete(create_time_dev_id)) {
            printf("VOS_Timer_Delete create_time_dev_id fail!\n");
        } else {
            create_time_dev_id = VOS_NULL_LONG;
        }
    }

    ret = VOS_Timer_Create(g_timer_task_id, g_timer_queue_id, time_out, (void(*) (void *))sg_timer_dev_callback, NULL,
        &create_time_dev_id, VOS_TIMER_LOOP);
    if (ret != VOS_OK) {
        printf("sg_timer_dev_create fail!\n");
        return -1;
    }
    return VOS_OK;
}
int sg_timer_container_create(uint32_t timeout)
{
    uint32_t ret;
    uint32_t time_out = timeout * 1000; //  ms

    if (VOS_NULL_LONG != create_time_container_id) {
        if (VOS_OK != VOS_Timer_Delete(create_time_container_id)) {
            printf("VOS_Timer_Delete create_time_container_id fail!\n");
        } else {
            create_time_container_id = VOS_NULL_LONG;
        }
    }

    ret = VOS_Timer_Create(g_timer_task_id, g_timer_queue_id, time_out, (void(*) (void *))sg_timer_container_callback, NULL,
        &create_time_container_id, VOS_TIMER_LOOP);
    if (ret != VOS_OK) {
        printf("sg_timer_container_create fail!\n");
        return -1;
    }
    return VOS_OK;
}
int sg_timer_app_create(uint32_t timeout)
{
    uint32_t ret;
    uint32_t time_out = timeout * 1000; //  ms

    if (VOS_NULL_LONG != create_time_app_id)
    {
        if (VOS_OK != VOS_Timer_Delete(create_time_app_id)) {
            printf("VOS_Timer_Delete create_time_app_id fail!\n");
        } else {
            create_time_app_id = VOS_NULL_LONG;
        }
    }

    ret = VOS_Timer_Create(g_timer_task_id, g_timer_queue_id, time_out, (void(*) (void *))sg_timer_app_callback, NULL,
        &create_time_app_id, VOS_TIMER_LOOP);
    if (ret != VOS_OK) {
        printf("sg_timer_app_create fail!\n");
        return -1;
    }
    return VOS_OK;
}

void sg_timer_heart_delete(void)
{
    uint32_t ret;
    ret = VOS_Timer_Delete(create_time_heart_id);
    if (ret != VOS_OK) {
        printf("sg_timer_heart_delete  fail!\n");
    }
}

void sg_timer_dev_delete(void)
{
    uint32_t ret;
    ret = VOS_Timer_Delete(create_time_dev_id);
    if (ret != VOS_OK) {
        printf("sg_timer_dev_delete  fail!\n");
    }
}

void sg_timer_container_deletee(void)
{
    uint32_t ret;
    ret = VOS_Timer_Delete(create_time_container_id);
    if (ret != VOS_OK) {
        printf("sg_timer_container_delete  fail!\n");
    }
}

void sg_timer_app_delete(void)
{
    uint32_t ret;
    ret = VOS_Timer_Delete(create_time_app_id);
    if (ret != VOS_OK) {
        printf("sg_timer_app_delete  fail!\n");
    }
}

int sg_get_dev_heart_flag(void)
{
    return heartflag;
}

void sg_set_dev_heart_flag(int flag)
{
    heartflag = flag;
    printf("sg_set_dev_heart_flag = %d.\n", heartflag);
}
