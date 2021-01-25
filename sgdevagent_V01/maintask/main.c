
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include "vrp.h"
#include "vrp_task.h"
#include "vos_typdef.h"
#include "vos_errno.h"
#include "ssp_mid.h"
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

void sgdevagent_ignore_sig(int signum);
void sgdevagent_sighandler(int signo);
void sg_handle_signal(void);

uint32_t g_create_dev_ins_id = 0;
uint32_t g_create_bus_int_id = 0;
uint32_t g_create_tsk_exe_dev_id = 0;
uint32_t g_create_tsk_exe_con_id = 0;
uint32_t g_create_tsk_exe_app_id = 0;
uint32_t g_create_mqt_pub_id = 0;
uint32_t g_create_socket_read_id = 0;
uint32_t g_create_socket_write_id = 0;


void sgdevagent_ignore_sig(int signum)
{
    struct sigaction sa;
    (void)memset_s(&sa, sizeof(sa), 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    (void)sigaction(signum, &sa, 0);
}

void sgdevagent_sighandler(int signo)
{
    SGDEV_NOTICE(SYSLOG_LOG, SGDEV_MODULE, "sgdevagent receive sigo:%d",signo);
    switch (signo)
    {
    case SIGQUIT:
    case SIGILL:
    case SIGBUS:
    case SIGFPE:
    case SIGSEGV:
        SSP_Backtrace("ACAGENT");
        break;
    default:
        break;
    }

    (void)kill(getpid(), signo);
    return;
}

void sg_handle_signal(void)
{
    struct sigaction action;

    (void)sigemptyset(&action.sa_mask);
    action.sa_flags = (int)(SA_NODEFER | SA_ONESHOT | SA_SIGINFO);
    action.sa_handler = sgdevagent_sighandler;

    (void)sigaction(SIGINT, &action, NULL);          // 2 中断进程
    (void)sigaction(SIGQUIT, &action, NULL);         // 3 终止进程
    (void)sigaction(SIGILL, &action, NULL);          // 4 非法指令
    (void)sigaction(SIGBUS, &action, NULL);          // 7 总线错误
    (void)sigaction(SIGFPE, &action, NULL);          // 8 浮点异常
    (void)sigaction(SIGSEGV, &action, NULL);         // 11 段非法错误
    (void)sigaction(SIGTERM, &action, NULL);         // 15 软件终止
    (void)sigaction(SIGXCPU, &action, NULL);         // 24 CPU资源限制
    (void)sigaction(SIGXFSZ, &action, NULL);         // 25 文件大小限制
    (void)sigaction(SIGSYS, &action, NULL);          // 31 系统调用异常

    return;

}
static void sg_create_task(void)
{
    sg_dev_param_info_s param = sg_get_param();
    VOS_UINT32 ret = 0;

    ret = (int)VOS_T_Create("devI", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_dev_insert_thread,                                        //启动任务1 设备接入线程
        &g_create_dev_ins_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create DEV insert fail ret = %d!", ret);
    }

    ret = VOS_T_Create("busi", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)bus_inter_thread, &g_create_bus_int_id);                        //业务交互线程
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create businter fail! ret = %d!", ret);
    }

    ret = VOS_T_Create("edev", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_dev_thread,                                  //任务执行线程
        &g_create_tsk_exe_dev_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create task exe dev fail! ret = %d!", ret);
    }

    ret = VOS_T_Create("econ", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_container_thread,                            //容器操作类
        &g_create_tsk_exe_con_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create task exe container fail! ret = %d!", ret);
    }

    ret = VOS_T_Create("eapp", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_task_execution_app_thread,                                   //app操作类
        &g_create_tsk_exe_app_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Create task exe app fail! ret = %d!", ret);
    }
    // 事件处理
     ret = (int)VOS_T_Create("skmg",VOS_T_PRIORITY_NORMAL,0,0,0,(TaskStartAddress_PF)sg_event_deal_thread,&g_create_event_id);
     if(ret != VOS_OK){
         printf(" \n VOS_T_Create task exe app fail!");
     }
    if (MODE_RMTMAN == param.startmode) {                                                                                                           //socket收、发数据线程       
        ret = VOS_T_Create("sred", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_rpc_read_data_thread,
            &g_create_socket_read_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Create socket read fail! ret = %d!", ret);
        }
        ret = VOS_T_Create("swte", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_rpc_write_data_thread,
            &g_create_socket_write_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Create socket write fail! ret = %d!", ret);
        }
    } else {                                                                                                                                        //发布mqtt线程
        ret = VOS_T_Create("mqtp", VOS_T_PRIORITY_NORMAL, 0, 0, 0, (TaskStartAddress_PF)sg_mqtt_pub_thread,
            &g_create_mqt_pub_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Create mqttpub fail! ret = %d!", ret);
        }
    }
}

static void sg_destroy_task(void)
{
    sg_dev_param_info_s param = sg_get_param();
    VOS_UINT32 ret = 0;
    ret = VOS_T_Delete(g_create_dev_ins_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete DEV insert fail!");
    }
    ret = VOS_T_Delete(g_create_bus_int_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete businter fail!");
    }
    ret = VOS_T_Delete(g_create_tsk_exe_dev_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete taskexe fail!");
    }
    ret = VOS_T_Delete(g_create_tsk_exe_con_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete taskexe fail!");
    }
    ret = VOS_T_Delete(g_create_tsk_exe_app_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete taskexe fail!");
    }
    ret = VOS_T_Delete(g_create_event_id);
    if (ret != VOS_OK) {
        printf(" \n VOS_T_Delete g_create_event_id fail!");
    }
    if (MODE_RMTMAN == param.startmode) {            //socket收、发数据线程
        ret = VOS_T_Delete(g_create_socket_read_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Delete g_create_socket_read_id fail!");
        }
        ret = VOS_T_Delete(g_create_socket_write_id);
        if (ret != VOS_OK) {
            printf(" \n VOS_T_Delete g_create_socket_write_id fail!");
        }
    } else {
        ret = VOS_T_Delete(g_create_mqt_pub_id);
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
    int ret = 0;

    pthread_cond_t main_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

    (void)sgdevagent_ignore_sig(SIGPIPE);
    (void)sgdevagent_ignore_sig(SIGRTMIN + 4);

    sg_log_init();      // 日志初始化

    ret = (int)SSP_Init();
    if(ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "SSP_Init failed ret:%d",ret);
        goto main_error;
    }

    //读参数文件
    if (read_param_file() != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "read_param_file failed");
        goto main_error;
    }

    //读周期参数
    if (read_period_file() != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "read_period_file failed");
        goto main_error;
    } 
    if (sg_init() != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_init failed");
        goto main_error;
    }

    sg_dev_param_info_s param = sg_get_param();

    //信号处理
    sg_handle_signal();

    if (MODE_RMTMAN == param.startmode) {
        sg_sockket_init();
        while (1) {
            VOS_T_Delay(30 * 1000);
        }
        ret = sg_sockket_exit();
    } else {
        if (sg_mqtt_init() != VOS_OK) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_mqtt_init failed");
            goto main_error;
        }
        for (;;) {
            if (sg_get_mqtt_connect_flag() != DEVICE_ONLINE) {
                if (sg_agent_mqtt_connect() != VOS_OK) {
                    continue;
                }            
                if (sg_create_sub_topic() != VOS_OK) {
                    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "mqtt connect subscribe failed.\n");
                    goto main_error;
                }
            }
            VOS_T_Delay(30 * 1000);  //延时30秒
            if (sg_get_dev_edge_reboot() == REBOOT_EDGE_SET) {
                break;
            }
        }
    }
    //for (;;) {
    //    (void)pthread_mutex_lock(&main_mutex);
    //    (void)pthread_cond_wait(&main_cond, &main_mutex);
    //    (void)pthread_mutex_unlock(&main_mutex);
    //}
    sg_mqtt_exit();
    sg_exit();

main_error:
    sg_log_close();
    return ret;
}

