
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
#include "sgdev_queue.h"

#include "thread_dev_insert.h"
#include "thread_task_exe.h"
#include "thread_interact.h"
#include "thread_manage.h"
#include "thread_event.h"

#include "timer_pack.h"
#include "rmt_socket.h"
#include "mqtt_pub.h"

void sgdevagent_ignore_sig(int signum);
void sgdevagent_sighandler(int signo);
void sg_handle_signal(void);

void sgdevagent_ignore_sig(int signum)
{
    struct sigaction sa;
    (void)memset_s(&sa, sizeof(sa), 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    (void)sigaction(signum, &sa, 0);
}

void sgdevagent_sighandler(int signo)
{
    SGDEV_NOTICE(SYSLOG_LOG, SGDEV_MODULE, "sgdevagent receive sigo:%d", signo);
    switch (signo)
    {
        case SIGQUIT:
        case SIGILL:
        case SIGBUS:
        case SIGFPE:
        case SIGSEGV:
            SSP_Backtrace("SGDEVAGENT");
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

static int sg_create_task(void)
{
    int ret = VOS_OK;
    if (sg_init_insert_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_init_interact_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_init_exe_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_init_event_thread() != VOS_OK) {
        ret = VOS_ERR;
    }
    if (sg_init_data_manager_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    return ret;
}

static int sg_destroy_task(void)
{
    int ret = VOS_OK;
    if (sg_exit_interact_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_exit_insert_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_exit_event_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_exit_exe_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_exit_data_manager_thread() != VOS_OK) {
        ret = VOS_ERR;
    }

    return ret;
}

static int sg_init()
{
    int ret = VOS_OK;
    (void)sg_mutex_init();            // 初始化锁

    (void)sg_que_unpack_create();    // 队列初始化
    (void)sg_que_pack_create();
    (void)sg_que_task_create();
    (void)sg_que_result_create();

    if (sg_create_task() != VOS_OK) {       // 创建任务
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_create_task creat failed.\n");
        ret = VOS_ERR;
    }

    sg_period_info_s param = sg_get_period();
    if (sg_timer_pre_create() != VOS_OK) {    // 注册定时器线程
        return VOS_ERR;
    }
    if (sg_timer_heart_create(param.dev_heartbeat_period)) {
        ret = VOS_ERR;
    }

    if (sg_timer_dev_create(param.dev_period) != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_timer_container_create(param.container_period) != VOS_OK) {
        ret = VOS_ERR;
    }

    if (sg_timer_app_create(param.app_period) != VOS_OK) {
        ret = VOS_ERR;
    }
    return  ret;
}

static void sg_exit()
{
    (void)sg_que_unpack_destroy();      //队列删除
    (void)sg_que_pack_destroy();
    (void)sg_que_task_destroy();
    (void)sg_que_result_destroy();

    (void)sg_timer_heart_delete();      //定时器删除
    (void)sg_timer_dev_delete();
    (void)sg_timer_container_deletee();
    (void)sg_timer_app_delete();

    if (sg_destroy_task() != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_destroy_task destroy failed.\n");
    }
    (void)sg_mutex_exit();
}

int main(int argc, char *argv[])
{
    int ret = 0;

    (void)sgdevagent_ignore_sig(SIGPIPE);
    (void)sgdevagent_ignore_sig(SIGRTMIN + 4);

    (void)sg_log_init();      // 日志初始化

    ret = (int)SSP_Init();
    if (ret != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "SSP_Init failed ret:%d", ret);
        goto main_error;
    }

    if (read_param_file() != VOS_OK) {      // 读参数文件
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "read_param_file failed");
        goto main_error;
    }

    if (read_period_file() != VOS_OK) {     // 读周期参数
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "read_period_file failed");
        goto main_error;
    }
    if (sg_init() != VOS_OK) {              // 初始化
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_init failed");
        goto main_error;
    }
    //信号处理
    (void)sg_handle_signal();
    sg_dev_param_info_s param = sg_get_param();

    if (param.startmode == MODE_RMTMAN) {
        (void)sg_sockket_init();
        (void)sg_sockket_exit();
    } else {
        if (sg_mqtt_main_task() != VOS_OK)) {
            SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "sg_mqtt_main_task failed");
        }
        (void)sg_mqtt_exit();
    }
    (void)sg_exit();

main_error:
    (void)sg_log_close();
    return ret;
}

