/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description : sgdevagent task event running header file
*/

#ifndef __TASK_EVENT_H__
#define __TASK_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SG_EVENT_TASK_NAME "skmg"

int sg_init_event_thread(void);
int sg_exit_event_thread(void);

#ifdef __cplusplus
}
#endif

#endif
