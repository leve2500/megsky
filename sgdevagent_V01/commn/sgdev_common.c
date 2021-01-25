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

// ͳ���޷��ų����������Ʊ�ʾ��1�ĸ���
// Hamming_weight�㷨��---ֻ����1��λ��
unsigned long long sg_hamming_weight(unsigned long long number)
{
    int count_ = 0; //������������
    while (number != 0) { //����
        number &= number - 1;
        count_++;
    }

    return count_;
}
//��Index����ʼ������num�γ����ַ���i_pStr��λ��
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

//���ַ�����ʼ����ȡi_nLen���ȵ��ַ���
 //�������  : char *pData  : Ҫ�������ݵ��׵�ַ
 //m_nLen �����볤��
 //dData �����ָ��
 //i_nLen ����ȡ����
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
//�������  : char *pData  : Ҫ�������ݵ��׵�ַ
//m_nLen �����볤��
//dData �����ָ��
//i_nLen ����ȡ����
//i_nIndex �ӵڼ�λ��ʼ

int sg_str_mid(char *pData, int m_nLen, int i_nIndex, char *dData, int i_nLen)
{
    int t_nLen = 0;
    if (dData == NULL)
        return 0;
    // i_nLen ������ʾȡ���������ַ�
    if (i_nIndex < 0 || i_nIndex >= m_nLen || m_nLen <= 0)
        return 0;
    else if (i_nLen > m_nLen - i_nIndex || i_nLen < 0)
        t_nLen = m_nLen - i_nIndex;
    else
        t_nLen = i_nLen;

    sg_strncpy(dData, pData + i_nIndex, t_nLen);
    return t_nLen;
}
//�������  : char *pData  : Ҫ�������ݵ��׵�ַ
//m_nLen �����볤��
//dData �����ָ��
//i_nLen ����ȡ����
//i_nIndex �ӵڼ�λ��ʼ
//���ַ����ҿ�ʼ����ȡi_nLen���ȵ��ַ���
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
 �� �� ��  : MQTT_SkipKey
 ��������  : ����Ҫ�����Ĺؼ����ַ���
 �������  : char *pData  : Ҫ�������ݵ��׵�ַ
             char *pKey   : �ؼ����ַ���
 �������  : ��
 �� �� ֵ  : char * : Խ���ؼ����Ժ�ĵ�ַָ�롣���û���ҵ��ؼ��֣�����NULL
 ���ú���  :
 ��������  :
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
 �� �� ��  : Mqtt_Response_Copy
 ��������  : �������ݿ�������
 �������  : char *pData    : �����׵�ַ
            char *pKey     : �ؼ���ָ��
            char *pEnd     : �����ؼ���ָ��
            char *pOutBuf  : ���������
 �������  : ��
 �� �� ֵ  : UINT32 : ��������ݳ���
 ���ú���  :
 ��������  :
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

//��ָ��ͨ�ýӿ�
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

//�������ļ��ӿ�
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

//�������ļ�����ͨ�ýӿ�
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
