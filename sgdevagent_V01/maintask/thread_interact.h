/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description : sgdevagent task event running header file
*/

#ifndef __THREAD_BUS_INTER_H__
#define __THREAD_BUS_INTER_H__

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SG_INTERACT_TASK_NAME "busi"
#define MS_BUS_THREAD_CONNECT_WAIT (1000 * 30)
#define MS_TEN_INTERVAL (10)

int sg_init_interact_thread(void);
int sg_exit_interact_thread(void);
void set_device_upgrade_status(dev_status_reply_s sta);
void set_container_install_status(dev_status_reply_s sta);
void set_container_upgrade_status(dev_status_reply_s sta);
void set_app_install_status(dev_status_reply_s sta);
void set_app_upgrade_status(dev_status_reply_s sta);

#ifdef __cplusplus
}
#endif

#endif


