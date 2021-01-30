/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description : sgdevagent task execute header file
*/

#ifndef __TASK_EXECUTION_H__
#define __TASK_EXECUTION_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SG_DEVICE_TASK_NAME "edev"
#define SG_CONTAINER_TASK_NAME "econ"
#define SG_APP_TASK_NAME "eapp"
#define MS_EXECUTE_THREAD_CONNECT_WAIT (1000 * 10)
#define MSG_ORDER_NUM_ZERO (0)
#define MSG_ORDER_NUM_FIRST (1)
#define MSG_ORDER_NUM_SECOND (2)
#define MSG_ORDER_NUM_THIRD (3)


int sg_init_exe_thread(void);
int sg_exit_exe_thread(void);

#ifdef __cplusplus
}
#endif 

#endif
