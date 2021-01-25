#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <sys/sysinfo.h>
#include <sys/time.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vos_mem.h"
#include "ssp_mid.h"
#include "sgdev_struct.h"


static char* sg_strncpy(char *io_pDst, const char *i_pSrc, int i_nLen)
{
    if (NULL != io_pDst) {
        if (NULL != i_pSrc && i_nLen >= 0) {
            strncpy(io_pDst, i_pSrc, i_nLen);
            io_pDst[i_nLen] = '\0';
        } else
        {
            io_pDst[0] = '\0';
        }
    }

    return io_pDst;
}

// 统计无符号长整数二进制表示中1的个数
// Hamming_weight算法二---只考虑1的位数
unsigned long long sg_hamming_weight(unsigned long long number)
{
    int count_ = 0; //声明计数变量
    while (number != 0) { //遍历
        number &= number - 1;
        count_++;
    }

    return count_;
}
//从Index处开始搜索第num次出现字符串i_pStr的位置
int sg_find(char *m_pBuf, int m_nLen, const char *i_pStr, int i_nNum, int i_nIndex)
{
    if (i_pStr == NULL)
        return -1;

    char *pos = NULL, *oldPos = NULL;

    if (i_nIndex > m_nLen - 1 || i_nIndex < 0)
        return -1;

    oldPos = m_pBuf + i_nIndex;

    for (int i = 0; i < i_nNum; ++i)
    {
        pos = strstr(oldPos, i_pStr);

        if (pos == NULL)
            return -2;

        oldPos = pos + strlen(i_pStr);
    }

    return (int)(pos - m_pBuf);
}

//从字符串左开始向左取i_nLen长度的字符串
 //输入参数  : char *pData  : 要搜索数据的首地址
 //m_nLen ：输入长度
 //dData ：输出指针
 //i_nLen ：截取个数
int sg_str_left(char *pData, int m_nLen, char *dData, int i_nLen)
{
    int t_nLen = 0;
    if (dData == NULL)
        return 0;

    if (i_nLen <= 0 || m_nLen <= 0)
        return 0;
    else if (i_nLen > m_nLen)
        t_nLen = m_nLen;
    else
        t_nLen = i_nLen;

    sg_strncpy(dData, pData, t_nLen);

    return t_nLen;
}
//输入参数  : char *pData  : 要搜索数据的首地址
//m_nLen ：输入长度
//dData ：输出指针
//i_nLen ：截取个数
//i_nIndex 从第几位开始

int sg_str_mid(char *pData, int m_nLen, int i_nIndex, char *dData, int i_nLen)
{
    int t_nLen = 0;
    if (dData == NULL)
        return 0;
    // i_nLen 负数表示取后面所有字符
    if (i_nIndex < 0 || i_nIndex >= m_nLen || m_nLen <= 0)
        return 0;
    else if (i_nLen > m_nLen - i_nIndex || i_nLen < 0)
        t_nLen = m_nLen - i_nIndex;
    else
        t_nLen = i_nLen;

    sg_strncpy(dData, pData + i_nIndex, t_nLen);
    return t_nLen;
}
//输入参数  : char *pData  : 要搜索数据的首地址
//m_nLen ：输入长度
//dData ：输出指针
//i_nLen ：截取个数
//i_nIndex 从第几位开始
//从字符串右开始向左取i_nLen长度的字符串
int sg_str_right(char *pData, int m_nLen, char *dData, int i_nLen)
{
    if (dData == NULL)
        return 0;
    int t_nLen = 0;

    if (i_nLen <= 0 || m_nLen <= 0)
        return 0;
    else if (i_nLen > m_nLen)
        t_nLen = m_nLen;
    else
        t_nLen = i_nLen;

    sg_strncpy(dData, pData + m_nLen - t_nLen, t_nLen);

    return t_nLen;
}
int sg_str_colon(char *pData, int m_nLen, char *ldDatal, char *rdData)
{
    int pos = 0;
    if (ldDatal == NULL || rdData == NULL) {
        return VOS_ERR;
    }
    pos = sg_find(pData, m_nLen, ":", 1, 0);
    if (pos > 0) {
        if (sg_str_left(pData, m_nLen, ldDatal, pos) <= 0) {
            return VOS_ERR;
        }
        if (sg_str_right(pData, m_nLen, rdData, m_nLen - pos - 1) <= 0) {
            return VOS_ERR;
        }
    } else {
        sg_strncpy(ldDatal, pData, m_nLen);
    }

    return VOS_OK;
}
/*****************************************************************************
 函 数 名  : MQTT_SkipKey
 功能描述  : 跳过要搜索的关键字字符串
 输入参数  : char *pData  : 要搜索数据的首地址
             char *pKey   : 关键字字符串
 输出参数  : 无
 返 回 值  : char * : 越过关键字以后的地址指针。如果没有找到关键字，返回NULL
 调用函数  :
 被调函数  :
*****************************************************************************/
static char *sg_skip_key(char *pData, char *pKey)
{
    char *pKeyData = NULL;

    if ((NULL == pData) || (NULL == pKey))
    {
        return NULL;
    }

    pKeyData = (char *)strstr(pData, pKey);

    if (NULL != pKeyData)
    {
        pKeyData += strlen(pKey);
    }

    return pKeyData;
}

/*****************************************************************************
 函 数 名  : Mqtt_Response_Copy
 功能描述  : 接收数据拷贝函数
 输入参数  : char *pData    : 数据首地址
            char *pKey     : 关键字指针
            char *pEnd     : 结束关键字指针
            char *pOutBuf  : 输出缓冲区
 输出参数  : 无
 返 回 值  : UINT32 : 处理的数据长度
 调用函数  :
 被调函数  :
*****************************************************************************/

static uint32_t sg_str_copy_sd(char *pData, char *pKey, char *pEnd, char *pOutBuf, uint32_t ulLength)
{
    uint32_t u32CpLen = 0;
    uint32_t u32RdLen = 0;
    char   *pKeyData = NULL;
    char   *pEndData = NULL;
    if ((NULL == pData)
        || (NULL == pKey)
        || (NULL == pOutBuf)
        // || (NULL == pEnd)
        || (0 == ulLength))
    {
        return 0;
    }
    pKeyData = sg_skip_key(pData, pKey);
    if (NULL != pKeyData)
    {
        pEndData = (char *)strstr(pKeyData, pEnd);

        if (NULL != pEndData)
        {
            u32CpLen = (uint32_t)(pEndData - pKeyData);
            memcpy(pOutBuf, pKeyData, u32CpLen);
            pOutBuf[u32CpLen] = 0;
            u32RdLen = (uint32_t)(pEndData - pData);
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
    json_t *piload = NULL;

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
    char dstbuf[DEV_INFO_MSG_BUFFER_LEN] = { 0 };
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
