
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description : sgdevagent dev access header
*/

#ifndef __DEV_INSERT_H__
#define __DEV_INSERT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MS_CONVERSION_INTERNAL (1000)
#define MS_THREAD_CONNECT_WAIT(1000 * 10)
#define SG_DEVICE_TASK_NAME "devi"
#define SG_POLY_TASK_NAME "poly"
//¡¥±Ì∂®“Â

typedef struct time_cnt_info {
    int order;
    clock_t m_dw_last_count;
    bool    m_b_valid_flag;
    uint32_t m_dw_diff_param;
}time_cnt_info_s;

int sg_init_insert_thread(void);
int sg_exit_insert_thread(void);

int sg_get_dev_ins_flag(void);
void sg_set_dev_ins_flag(int flag);

bool sg_calover_time(time_cnt_info_s *info);


#ifdef __cplusplus
}
#endif 

#endif
