#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/sysinfo.h>
#include <sys/time.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "ssp_mid.h"
#include "sgdev_struct.h"


#define  STR_ERROR_NEG_TWO (-2)

static char* sg_strncpy(char *io_pdst, const char *i_psrc, int i_nlen)
{
    int size;
    if (NULL != io_pdst) {
        if (NULL != i_psrc && i_nlen >= 0) {
            size = strncpy_s(io_pdst, i_nlen, i_psrc, i_nlen);
            if (size < 0) {
                io_pdst[0] = '\0';
            }            
            io_pdst[i_nlen] = '\0';
        } else {
            io_pdst[0] = '\0';
        }
    }

    return io_pdst;
}

// 统计无符号长整数二进制表示中1的个数 Hamming_weight算法二---只考虑1的位数
int sg_hamming_weight(unsigned long long number)
{
    int count = 0;
    while (number != 0) {
        number &= number - 1;
        count++;
    }

    return count;
}
// 从Index处开始搜索第num次出现字符串i_pstr的位置
int sg_find(char *m_pbuf, int m_nlen, const char *i_pstr, int i_nnum, int i_nindex)
{
    if (i_pstr == NULL)
        return -1;

    char *pos = NULL;
    char *old_pos = NULL;

    if (i_nindex > m_nlen - 1 || i_nindex < 0) {
        return -1;
    }

    old_pos = m_pbuf + i_nindex;

    for (int i = 0; i < i_nnum; ++i) {
        pos = strstr(old_pos, i_pstr);
        if (pos == NULL) {
            return STR_ERROR_NEG_TWO;
        }

        old_pos = pos + strlen(i_pstr);
    }

    return (int)(pos - m_pbuf);
}

// 从字符串左开始向左取i_nlen长度的字符串
// 输入参数  : char *p_data  : 要搜索数据的首地址
// m_nlen ：输入长度
// d_data ：输出指针
// i_nlen ：截取个数
int sg_str_left(char *p_data, int m_nlen, char *d_data, int i_nlen)
{
    int t_nLen = 0;
    if (d_data == NULL)
        return 0;

    if (i_nlen <= 0 || m_nlen <= 0) {
        return 0;
    } else if (i_nlen > m_nlen) {
        t_nLen = m_nlen;
    } else {
        t_nLen = i_nlen;
    }

    sg_strncpy(d_data, p_data, t_nLen);

    return t_nLen;
}
//输入参数  : char *p_data  : 要搜索数据的首地址
//m_nlen ：输入长度
//d_data ：输出指针
//i_nlen ：截取个数
//i_nindex 从第几位开始

int sg_str_mid(char *p_data, int m_nlen, int i_nindex, char *d_data, int i_nlen)
{
    int t_nLen = 0;
    if (d_data == NULL)
        return 0;
    // i_nlen 负数表示取后面所有字符
    if (i_nindex < 0 || i_nindex >= m_nlen || m_nlen <= 0)
        return 0;
    else if (i_nlen > m_nlen - i_nindex || i_nlen < 0)
        t_nLen = m_nlen - i_nindex;
    else
        t_nLen = i_nlen;

    sg_strncpy(d_data, p_data + i_nindex, t_nLen);
    return t_nLen;
}
//输入参数  : char *p_data  : 要搜索数据的首地址
//m_nlen ：输入长度
//d_data ：输出指针
//i_nlen ：截取个数
//i_nindex 从第几位开始
//从字符串右开始向左取i_nlen长度的字符串
int sg_str_right(char *p_data, int m_nlen, char *d_data, int i_nlen)
{
    if (d_data == NULL)
        return 0;
    int t_nLen = 0;

    if (i_nlen <= 0 || m_nlen <= 0)
        return 0;
    else if (i_nlen > m_nlen)
        t_nLen = m_nlen;
    else
        t_nLen = i_nlen;

    sg_strncpy(d_data, p_data + m_nlen - t_nLen, t_nLen);

    return t_nLen;
}
int sg_str_colon(char *p_data, int m_nlen, char *ld_datal, char *rd_data)
{
    int pos = 0;
    if (ld_datal == NULL || rd_data == NULL) {
        return VOS_ERR;
    }
    pos = sg_find(p_data, m_nlen, ":", 1, 0);
    if (pos > 0) {
        if (sg_str_left(p_data, m_nlen, ld_datal, pos) <= 0) {
            return VOS_ERR;
        }
        if (sg_str_right(p_data, m_nlen, rd_data, m_nlen - pos - 1) <= 0) {
            return VOS_ERR;
        }
    } else {
        sg_strncpy(ld_datal, p_data, m_nlen);
    }

    return VOS_OK;
}
/*****************************************************************************
 函 数 名  : MQTT_SkipKey
 功能描述  : 跳过要搜索的关键字字符串
 输入参数  : char *p_data  : 要搜索数据的首地址
             char *p_key   : 关键字字符串
 输出参数  : 无
 返 回 值  : char * : 越过关键字以后的地址指针。如果没有找到关键字，返回NULL
 调用函数  :
 被调函数  :
*****************************************************************************/
static char *sg_skip_key(char *p_data, char *p_key)
{
    char *pkey_data = NULL;

    if ((NULL == p_data) || (NULL == p_key))
    {
        return NULL;
    }

    pkey_data = (char *)strstr(p_data, p_key);

    if (NULL != pkey_data)
    {
        pkey_data += strlen(p_key);
    }

    return pkey_data;
}

/*****************************************************************************
 函 数 名  : Mqtt_Response_Copy
 功能描述  : 接收数据拷贝函数
 输入参数  : char *p_data    : 数据首地址
            char *p_key     : 关键字指针
            char *p_end     : 结束关键字指针
            char *p_ou_buf  : 输出缓冲区
 输出参数  : 无
 返 回 值  : UINT32 : 处理的数据长度
 调用函数  :
 被调函数  :
*****************************************************************************/

static uint32_t sg_str_copy_sd(char *p_data, char *p_key, char *p_end, char *p_ou_buf, uint32_t ul_length)
{
    uint32_t u32CpLen = 0;
    uint32_t u32RdLen = 0;
    char   *pkey_data = NULL;
    char   *pEndData = NULL;
    if ((NULL == p_data)
        || (NULL == p_key)
        || (NULL == p_ou_buf)
        // || (NULL == p_end)
        || (0 == ul_length))
    {
        return 0;
    }
    pkey_data = sg_skip_key(p_data, p_key);
    if (NULL != pkey_data)
    {
        pEndData = (char *)strstr(pkey_data, p_end);

        if (NULL != pEndData)
        {
            u32CpLen = (uint32_t)(pEndData - pkey_data);
            memcpy(p_ou_buf, pkey_data, u32CpLen);
            p_ou_buf[u32CpLen] = 0;
            u32RdLen = (uint32_t)(pEndData - p_data);
        }
    }
    return u32RdLen;
}

//读指令通用接口
int sg_cmd_common_get(const char* p, char* desp)
{
    int ret = VOS_ERR;
    char temp_str[DEV_INFO_MSG_BUFFER_LEN];

    char buf[DEV_INFO_MSG_BUFFER_LEN];
    FILE * p_file = NULL;

    if (desp == NULL) {
        return ret;
    }

    p_file = popen(p, "r");
    if (!p_file) {
        printf("Erro to popen\n");
        return ret;
    }
    while (fgets(buf, DEV_INFO_MSG_BUFFER_LEN, p_file) != NULL) {
        sprintf_s(temp_str, DEV_INFO_MSG_BUFFER_LEN, "%s", buf);
    }
    memcpy_s(desp, DEV_INFO_MSG_BUFFER_LEN, buf, strlen(buf) - 1);
    pclose(p_file);
    return VOS_OK;
}

//读多行文件接口
int sg_read_file_get(const char *p, char *desp)
{
    int filesize = 0;
    FILE *fp = NULL;
    char *buf = NULL;
    char real_path[PATH_MAX] = { 0 };

    filesize = getfilesize(p);
    if (filesize <= 0) {
        return VOS_ERR;
    }
    buf = (char *)VOS_Malloc(MID_SGDEV, filesize);
    if (buf == NULL) {
        fprintf(stderr, "Error - unable to allocate required memory\n");
        return VOS_ERR;
    }
    (void)memset_s(buf, filesize, 0, filesize);

    if (realpath(p, real_path) == NULL) {
        return -1;
    }
    fp = fopen(real_path, "r");
    if (fp == NULL) {
        (void)printf("Failed to open the file!\n");
        (void)VOS_Free(buf);
        return VOS_ERR;
    }
    if (!fread(buf, filesize, 1, fp)) {
        (void)printf("read file error!\n");
        (void)fclose(fp);
        (void)VOS_Free(buf);
        return VOS_ERR;
    }
    if (desp == NULL) {
        return VOS_ERR;
    }
    memcpy_s(desp, strlen(buf), buf, strlen(buf));
    printf("buf = %s\n", buf);
    (void)VOS_Free(buf);
    return VOS_OK;
}

//读单行文件内容通用接口
int sg_file_common_get(const char *p, char *desp)
{
    FILE *fp;
    int ret = VOS_ERR;
    char buf[DEV_INFO_MSG_BUFFER_LEN] = { 0 };
    if (desp == NULL) {
        return ret;
    }
    if ((fp = fopen(p, "r")) == NULL) {
        printf("\nfile open common failed.\n");
        return VOS_ERR;
    }
    while (!feof(fp)) {
        fgets(buf, DEV_INFO_MSG_BUFFER_LEN, fp);
    }
    fclose(fp);
    sprintf_s(desp, DEV_INFO_MSG_BUFFER_LEN, "%s", buf);
    return VOS_OK;
}
