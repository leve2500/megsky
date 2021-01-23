/*
 * Copyright（c）Huawei Technologies Co.,Ltd.2019-2020.All rights reserved
 * Description : sgdevagent common header file
*/

#ifndef _SGDEV_COMMON_H_
#define _SGDEV_COMMON_H_

#ifdef __cplusplus
extern "C" {  // only need to export C interface if used by C++ source code
#endif

unsigned long long sg_hamming_weight(unsigned long long number);    //统计无符号长整数二进制表示中1的个数
int sg_find(char *m_pBuf, int m_nLen, const char *i_pStr, int i_nNum, int i_nIndex);
int sg_str_left(char *pData, int m_nLen, char *dData, int i_nLen);
int sg_str_mid(char *pData, int m_nLen, int i_nIndex, char *dData, int i_nLen);
int sg_str_right(char *pData, int m_nLen, char *dData, int i_nLen);
int sg_str_colon(char *pData, int m_nLen, char *ldDatal, char *rdData);// ：分割左右字符
int sg_cmd_common_get(const char* p, char* desp);
int sg_read_file_get(const char *p, char *desp);
int sg_file_common_get(const char *p, char *desp);



#ifdef __cplusplus
}
#endif 

#endif


