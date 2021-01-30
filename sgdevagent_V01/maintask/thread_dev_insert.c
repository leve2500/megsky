#include <sys/times.h>
#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "ssp_mid.h"
#include "net_common.h"
#include "ifm_status.h"

#include "sysman_devinfo_def.h"
#include "sysman_rpc_api.h"

#include "sgdev_struct.h"
#include "sgdev_queue.h"
#include "sgdev_mutex.h"
#include "sgdev_debug.h"
#include "mqtt_dev.h"
#include "mqtt_pub.h"
#include "task_link.h"
#include "thread_dev_insert.h"

int g_dev_access_flag = 0;
uint32_t g_create_dev_ins_id = 0;
uint32_t g_create_poly_ins_id = 0;

int sg_poly_thread(void);            // 延时任务处理
int sg_dev_insert_thread(void);      // 设备接入线程

int sg_init_insert_thread(void)
{
    int i_ret = VOS_OK;
    VOS_UINT32 ret = 0;
    ret = VOS_T_Create(SG_DEVICE_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_dev_insert_thread,
        &g_create_dev_ins_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_DEVICE_TASK_NAME, ret);
        i_ret = VOS_ERR;
    }

    ret = VOS_T_Create(SG_POLY_TASK_NAME, VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_poly_thread,
        &g_create_poly_ins_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) creat failed ret:%d.\n", SG_POLY_TASK_NAME, ret);
        return VOS_ERR;
    }

    return i_ret;
}

int sg_exit_insert_thread(void)
{
    int i_ret = VOS_OK;
    VOS_UINT32 ret = 0;
    ret = VOS_T_Delete(g_create_dev_ins_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_DEVICE_TASK_NAME, ret);
        i_ret = VOS_ERR;
    }

    ret = VOS_T_Delete(g_create_poly_ins_id);
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "task(%s) destroy failed ret:%d.\n", SG_POLY_TASK_NAME, ret);
        return VOS_ERR;
    }

    return VOS_OK;
}

// g_dev_access_flag 的判断 如果是rmtman 则增加判断其是否与平台已经连接,未连接则continue
// 检测到离线就发送上线，
// 
int sg_dev_insert_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_dev_insert_thread start.\n");
    int ret = VOS_OK;
    int connect_flag;
    mqtt_data_info_s *item = NULL;
    dev_acc_req_s devinfo = { 0 };
    (void)memset_s(&devinfo, sizeof(dev_acc_req_s), 0, sizeof(dev_acc_req_s));
    sg_dev_param_info_s param = sg_get_param();

    for (; ;) {
        if (param.startmode == MODE_RMTMAN) {
            connect_flag = sg_get_mqtt_connect_flag();  // 更换rmtman 与平台连接标志判断，socket 连接状态判断
        } else {
            connect_flag = sg_get_mqtt_connect_flag();
        }

        if (connect_flag == DEVICE_OFFLINE) {
            (void)VOS_T_Delay(MS_THREAD_CONNECT_WAIT);
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "mqtt or socket link not connect.\n");
            continue;
        }
        if (g_dev_access_flag != DEVICE_ONLINE) {
            item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
            if (item == NULL) {
                SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_dev_insert_thread:mqtt_data_info_s malloc failed.\n");
                break;
            }

            (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
            if (sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_request_pub()) < 0) {
                SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_dev_insert_thread:sprintf_s, pub topic failed.\n");
                break;
            }

            if (sg_get_dev_insert_info(&devinfo) != VOS_OK) {                      // 获取设备接入信息
                SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "get device info failed.\n");
            }

            (void)sg_pack_dev_linkup_data(&devinfo, item->msg_send);              // 将设备接入信息打包为json格式
            (void)sg_push_pack_item(item);              // 入列
        }

        VOS_T_Delay(MS_THREAD_CONNECT_WAIT);        // 延时10秒
    }
    return ret;
}
// 设备升级
// 容器安装
// 容器升级
// app安装
// app升级
int sg_poly_thread(void)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sg_poly_thread start.\n");
    time_cnt_info_s info = { 0 };
    size_t i = 0;
    (void)memset_s(&info, sizeof(time_cnt_info_s), 0, sizeof(time_cnt_info_s));
    for (; ;) {

        for (i = 0; i < 1; i++) {       // 获取表中的参数
            if (sg_calovertime(&info)) {             // info.m_dw_diff_param = 20;
                //执行
            }
        }
        VOS_T_Delay(MS_CONVERSION_INTERNAL);        // 延时1秒
    }
    return VOS_OK;
}

int sg_get_dev_ins_flag(void)
{
    return g_dev_access_flag;
}

void sg_set_dev_ins_flag(int flag)
{
    g_dev_access_flag = flag;    // 上线、离线通知   发socket通知
    SGDEV_INFO (SYSLOG_LOG, SGDEV_MODULE, "device online flag = %d.\n", g_dev_access_flag);
}

bool sg_start_count(time_cnt_info_s *info)
{
    struct tms ctm;
    info->m_b_valid_flag = true;
    info->m_dw_last_count = times(&ctm);
}

bool sg_stop_count(time_cnt_info_s *info)
{
    info->m_b_valid_flag = false;
}

bool sg_calover_time(time_cnt_info_s *info)
{
    bool ret = false;
    if (info->m_b_valid_flag) {
        uint32_t clktck = sysconf(_SC_CLK_TCK);
        struct tms ctm;
        clock_t	dw_cur_cnt = times(&ctm);
        int64_t	dw_span = (dw_cur_cnt - info->m_dw_last_count) * MS_CONVERSION_INTERNAL / clktck;
        if (dw_span >= info->m_dw_diff_param) {
            ret = true;
        }
    }

    return ret;
}


