/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description : sgdevagent task manager header file
*/

#ifndef _THREAD_DATA_MGR_H_
#define _THREAD_DATA_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SG_MQTT_TASK_NAME "sgmq"
#define SG_RPC_WRITE_TASK_NAME "sgwr"
#define SG_RPC_READ_TASK_NAME "sgrd"
#define MS_MGR_THREAD_CONNECT_WAIT (1000 * 5)

int sg_init_data_manager_thread(void);
int sg_exit_data_manager_thread(void);

void sg_set_rpc_connect_flag(int flag);
int sg_get_rpc_connect_flag(void);


#ifdef __cplusplus
}
#endif 

#endif
