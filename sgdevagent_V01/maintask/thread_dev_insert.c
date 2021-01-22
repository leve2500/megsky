#include <sys/times.h>
#include "vos_typdef.h"

#include "sysman_devinfo_def.h"
#include "sysman_rpc_api.h"
#include "thread_dev_insert.h"
#include "mqtt_dev.h"
#include "mqtt_pub.h"
#include "sgdev_queue.h"
#include "sgdev_mutex.h"
#include "sgdev_struct.h"
#include "task_link.h"

int bdevInsFlag = 0;
//设备接入线程
int sg_dev_insert_thread(void)
{
    printf("sg_dev_insert_thread start. \n");
    int ret = VOS_OK;
    VOS_UINT32 freeRet = 0;
    mqtt_data_info_s *item = NULL;
    dev_acc_req_s devinfo = { 0 };
    for (; ;) {
        if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
            VOS_T_Delay(1000 * 3);
            printf("megsky--test :sg_dev_insert_thread: mqtt not connect \n");
            continue;
        }
        if (bdevInsFlag != DEVICE_ONLINE)    //上线标志位为 DEVICE_ONLINE时，启动上线，否则延时等待 等待时间为10s
        {
            //如果是rmtman 则增加判断其是否与平台已经连接,未连接则continue
            //检测到离线就发送上线，
            item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
            if (item == NULL) {
                printf("megsky--test :VOS_Malloc mqtt_data_info_s failed \n");
                continue;
            }
            (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

            if (sprintf_s(item->pubtopic, DATA256_LEN, "%s", get_topic_device_request_pub()) < 0) {
                printf("megsky--test :sprintf_s pubtopic failed \n");
                continue;
            }
            ret = sg_get_dev_insert_info(&devinfo);            //获取设备接入信息 从华为中间件获取
            sg_pack_dev_linkup_data(&devinfo, item->msg_send);  //将设备接入信息打包为json格式

            sg_push_pack_item(item);        //入列  
        }
        VOS_T_Delay(1000 * 10);  //延时10秒
    }
    return ret;
}

int sg_get_dev_ins_flag(void)
{
    return bdevInsFlag;
}

void sg_set_dev_ins_flag(int flag)
{
    bdevInsFlag = flag;
    printf("sg_set_dev_ins_flag = %d . \n", bdevInsFlag);
    //上线、离线通知？ 发socket通知。
}

void sg_send_link_down(char *reason)
{

}

bool sg_start_count(time_cnt_info_s *info)
{
    struct tms ctm;
    info->m_bValidFlag = true;
    info->m_dwLastCount = times(&ctm);
}

bool sg_stop_count(time_cnt_info_s *info)
{
    info->m_bValidFlag = false;
}

bool sg_calovertime(time_cnt_info_s *info)
{
    bool ret = false;
    if (info->m_bValidFlag)
    {
        uint32_t clktck = sysconf(_SC_CLK_TCK);
        struct tms ctm;
        clock_t	dwCurCnt = times(&ctm);
        int64_t	dwSpan = (dwCurCnt - info->m_dwLastCount) * 1000 / clktck;
        if (dwSpan >= info->m_dwDiffParam) {
            ret = true;
            //tms ctm;
            //m_dwLastCount = times(&ctm);
        }
    }
    return ret;
}

int sg_poly_thread(void)
{
    printf("sg_poly_thread start. \n");

    time_cnt_info_s info = { 0 };

    for (; ;) {
        //获取表中的参数
        for (size_t i = 0; i < 8; i++)
        {
            // info.m_dwDiffParam = 20;
            if (sg_calovertime(&info))
            {
                //执行
            }
        }
    }
    //设备升级
    //容器安装
    //容器升级
    //app安装
    //app升级
}
