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

#include "task_deal.h"
#include "thread_dev_insert.h"
#include "thread_task_exe.h"
#include "thread_event.h"

uint32_t g_create_event_id = 0;

void sg_alarm_topic_pro(char *msg_buf, size_t msg_len);
int sg_event_deal_thread(void);

int sg_init_event_thread(void)
{
    int ret;
    ret = (int)VOS_T_Create(SG_EVENT_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_event_deal_thread,                                        //启动任务1 设备接入线程
        &g_create_event_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_EVENT_TASK_NAME, ret);
        return VOS_ERR;
    }

    return VOS_OK;
}

int sg_exit_event_thread(void)
{
    int ret;
    ret = (int)VOS_T_Delete(g_create_event_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_EVENT_TASK_NAME, ret);
        return VOS_ERR;
    }

    return VOS_OK;
}
void sg_alarm_topic_pro(char *msg_buf, size_t msg_len)
{
    AlarmInfoStatusType *alarm_status = NULL;

    if (msg_len < sizeof(AlarmInfoStatusType)) {
        return;
    }

    alarm_status = (AlarmInfoStatusType *)msg_buf;
}

// 事件线程
int sg_event_deal_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_event_deal_thread start.\n");
    int ret;
    int fd;
    char *msg_buf = NULL;
    size_t msg_len;
    msg_buf = (char *)VOS_Malloc(MID_SGDEV, MSGPROC_MSG_LENGTH_MAX);
    if (msg_buf == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "event buf malloc failed.\n");
        return VOS_ERR;
    }

    fd = kmsg_sub_open(ALARM_TOPIC, 0);
    if (fd < 0) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "kmsg_sub_open failed.\n");
        return VOS_ERR;
    }
    for (;;) {
        msg_len = MSGPROC_MSG_LENGTH_MAX;
        (void)memset_s(msg_buf, MSGPROC_MSG_LENGTH_MAX, 0, MSGPROC_MSG_LENGTH_MAX);
        ret = kmsg_subpub_sonsume_block(%fd, NULL, 0, msg_buf, &msg_len);           // 获取告警信息
        if(ret < 0){
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "kmsg_subpub_sonsume_block failed.\n");
        }

        sg_alarm_topic_pro(msg_buf, msg_len);
    }
    (void)kmsg_sub_close(fd);
}
