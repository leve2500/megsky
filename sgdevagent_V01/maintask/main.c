
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "vrp.h"
#include "vrp_task.h"
#include "vos_typdef.h"

#include "sgdev_debug.h"
#include "sgdev_param.h"
#include "sgdev_struct.h"
#include "sgdev_queue.h"

#include "thread_dev_insert.h"
#include "thread_task_exe.h"
#include "thread_interact.h"
#include "thread_manage.h"

#include "timer_pack.h"
#include "rmt_socket.h"
#include "mqtt_pub.h"


uint32_t create_dev_ins_id = 0;
uint32_t create_bus_int_id = 0;
uint32_t create_tsk_exe_dev_id = 0;
uint32_t create_tsk_exe_con_id = 0;
uint32_t create_tsk_exe_app_id = 0;
uint32_t create_mqt_pub_id = 0;
uint32_t create_socket_read_id = 0;
uint32_t create_socket_write_id = 0;
uint32_t create_event_id = 0;

static void sg_create_task(void)
{
    sg_dev_param_info_s param = sg_get_param();
    VOS_UINT32 ret = 0;

    ret = VOS_T_Create("devI", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_dev_insert_thread,                                        //启动任务1 设备接入线程
        &create_dev_ins_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create DEV insert fail ret = %d!", ret);
    }

    ret = VOS_T_Create("busi", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)bus_inter_thread, &create_bus_int_id);                        //业务交互线程
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create businter fail! ret = %d!", ret);
    }

    ret = VOS_T_Create("edev", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_dev_thread,                                  //任务执行线程
        &create_tsk_exe_dev_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create task exe dev fail! ret = %d!", ret);
    }

    ret = VOS_T_Create("econ", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_container_thread,                            //容器操作类
        &create_tsk_exe_con_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create task exe container fail! ret = %d!", ret);
    }

    ret = VOS_T_Create("eapp", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_app_thread,                                   //app操作类
        &create_tsk_exe_app_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create task exe app fail! ret = %d!", ret);
    }
    // //事件处理
    // ret = VOS_T_Create("eventpro",VOS_T_PRIORITY_NORMAL,0,0,0,(TaskStartAddress_PF)sg_event_deal_thread,&create_event_id);
    // if(ret != VOS_OK){
    //     printf(" \n VOS_T_Create task exe app fail!");
    // }
    if (MODE_RMTMAN == param.startmode) {                                                                                                           //socket收、发数据线程       
        ret = VOS_T_Create("sred", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_rpc_read_data_thread,
            &create_socket_read_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Create socket read fail! ret = %d!", ret);
        }
        ret = VOS_T_Create("swte", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_rpc_write_data_thread,
            &create_socket_write_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Create socket write fail! ret = %d!", ret);
        }
    } else {                                                                                                                                        //发布mqtt线程
        ret = VOS_T_Create("mqtp", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_mqtt_pub_thread,
            &create_mqt_pub_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Create mqttpub fail! ret = %d!", ret);
        }
    }
}

static void sg_destroy_task(void)
{
    sg_dev_param_info_s param = sg_get_param();
    VOS_UINT32 ret = 0;
    ret = VOS_T_Delete(create_dev_ins_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete DEV insert fail!");
    }
    ret = VOS_T_Delete(create_bus_int_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete businter fail!");
    }
    ret = VOS_T_Delete(create_tsk_exe_dev_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete taskexe fail!");
    }
    ret = VOS_T_Delete(create_tsk_exe_con_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete taskexe fail!");
    }
    ret = VOS_T_Delete(create_tsk_exe_app_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete taskexe fail!");
    }
    ret = VOS_T_Delete(create_event_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete create_event_id fail!");
    }
    if (MODE_RMTMAN == param.startmode) {            //socket收、发数据线程
        ret = VOS_T_Delete(create_socket_read_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Delete create_socket_read_id fail!");
        }
        ret = VOS_T_Delete(create_socket_write_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Delete create_socket_write_id fail!");
        }
    } else {
        ret = VOS_T_Delete(create_mqt_pub_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Delete mqttpub fail!");
        }
    }
}

static int sg_init()
{
    int nRet = VOS_OK;
    sg_mutex_init();            //初始化锁
    //队列初始化
    sg_que_unpack_create();
    sg_que_pack_create();
    sg_que_task_create();
    sg_que_result_create();
    sg_create_task();           //创建任务

    //注册定时器
    sg_period_info_s param = sg_get_period();
    if (sg_timer_pre_create() == VOS_OK) {
        nRet = sg_timer_heart_create(param.devheartbeatperiod);
        if (nRet != VOS_OK) {
            return nRet;
        }
        nRet = sg_timer_dev_create(param.devperiod);
        if (nRet != VOS_OK) {
            return nRet;
        }
        nRet = sg_timer_container_create(param.containerperiod);
        if (nRet != VOS_OK) {
            return nRet;
        }
        nRet = sg_timer_app_create(param.appperiod);
        if (nRet != VOS_OK) {
            return nRet;
        }
    }
    return  nRet;
}


static int sg_exit()
{
    int nRet = 0;
    sg_que_unpack_destroy();	//队列删除
    sg_que_pack_destroy();
    sg_que_task_destroy();
    sg_que_result_destroy();

    sg_timer_heart_delete();    //定时器删除
    sg_timer_dev_delete();
    sg_timer_container_deletee();
    sg_timer_app_delete();

    sg_destroy_task();		//任务退出
    sg_mutex_exit();
    return  nRet;
}

int main(int argc, char *argv[])
{
    int ch;
    int rc;
    int nRet = 0;

    sg_log_init(); //日志初始化
    nRet = SSP_Init();
    if (nRet != VOS_OK) {
        goto main_error;
    }

    read_param_file();      //读参数文件
    read_period_file();     //读周期参数
    sg_init();
    sg_dev_param_info_s param = sg_get_param();

    if (MODE_RMTMAN == param.startmode) {
        sg_sockket_init();
        while (1) {
            VOS_T_Delay(30 * 1000);
        }
        nRet = sg_sockket_exit();
    } else {
        nRet = sg_mqtt_init();
        while (1) {
            if (!sg_get_mqtt_connect_flag()) {
                sg_mqtt_exit();
                sg_mqtt_init();
            } else if (!sg_get_mqttclient_isconnected()) {
                printf("MQTTClient_isConnected false.\n");
                sg_mqtt_exit();
                sg_mqtt_init();
            }
            VOS_T_Delay(30 * 1000);  //延时30秒
            if (sg_get_dev_edge_reboot() == REBOOT_EDGE_SET) {
                break;
            }
        }
    }
    sg_mqtt_exit();
    sg_exit();

main_error:
    sg_log_close();
    return nRet;
}

