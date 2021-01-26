/*
 * Copyright（c）Huawei Technologies Co.,Ltd.2019-2020.All rights reserved
 * Description : sgdevagent common header file
*/

#ifndef _SGDEV_COMMON_H_
#define _SGDEV_COMMON_H_

#ifdef __cplusplus
extern "C" {  // only need to export C interface if used by C++ source code
#endif

int sg_hamming_weight(unsigned long long number);    //统计无符号长整数二进制表示中1的个数
int sg_find(char *m_pbuf, int m_nlen, const char *i_pstr, int i_nnum, int i_nindex);
int sg_str_left(char *p_data, int m_nlen, char *d_data, int i_nlen);
int sg_str_mid(char *p_data, int m_nlen, int i_nindex, char *d_data, int i_nlen);
int sg_str_right(char *p_data, int m_nlen, char *d_data, int i_nlen);
int sg_str_colon(char *p_data, int m_nlen, char *ld_datal, char *rd_data);// ：分割左右字符
int sg_cmd_common_get(const char* p, char* desp);
int sg_read_file_get(const char *p, char *desp);
int sg_file_common_get(const char *p, char *desp);



#ifdef __cplusplus
}
#endif 

#endif


