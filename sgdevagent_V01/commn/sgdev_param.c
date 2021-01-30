#include <stdio.h>
#include <linux/limits.h>
#include <limits.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "ssp_mid.h"

#include "sgdev_param.h"
#include "mqtt_json.h"

sg_dev_param_info_s m_devParam;
sg_dev_section_info_s m_devSection;
sg_period_info_s m_devperiod;

int getfilesize(char *strFileName)
{
    FILE *fp = NULL;
    char real_path[PATH_MAX] = { 0 };
    int size = 0;

    if (strFileName == NULL) {
        return -1;
    }

    if (realpath(strFileName, real_path) == NULL) {
        return -1;
    }

    fp = fopen(real_path, "r");
    if (fp == NULL) {
        return -1;
    }

    (void)fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    (void)fseek(fp, 0L, SEEK_SET);
    (void)fclose(fp);
    return size;
}
// 读取断面文件 代码还未调试完成
int read_section_file(void)
{
    FILE *fp = NULL;
    char buff[DATA_BUF_F256_SIZE] = { 0 };
    json_t* piload = NULL;
    fp = fopen("/mnt/internal_storage/sg_devagentsection", "r");
    if (NULL == fp) {
        printf("Failed to open the file !\n");
        // exit(0);
    } else {
        printf("step 1\n");
        fgets(buff, DATA_BUF_F256_SIZE, (FILE*)fp);	// 将文件内容全部读取
        printf("step 2\n");
        piload = load_json(buff);		// 将字符串转为json
        printf("step 3\n");
        // if (!json_is_object(piload)){
        //     printf("%s,%d\n",__FILE__,__LINE__);
        // }	
        // printf("step 4\n");
        if (json_into_uint8_t(&m_devSection.rebootReason, piload, "rebootReason") != VOS_OK) {
            return VOS_ERR;
        }
        printf("step 4\n");
        if (json_into_int32_t(&m_devSection.jobid, piload, "jobid") != VOS_OK) {
            return VOS_ERR;
        }
        printf("step 5\n");
        if (json_into_int32_t(&m_devSection.midid, piload, "midid") != VOS_OK) {
            return VOS_ERR;
        }
        printf("step 6\n");
        if (json_into_string(m_devSection.version, piload, "version") != VOS_OK) {
            return VOS_ERR;
        }
    }
    printf("step 8\n");
    json_decref(piload);
    printf("step 9\n");
    (void)fclose(fp);
    return VOS_OK;
}
// 写断面文件 代码还未调试完成
void write_section_file(void)
{
    FILE *fp = NULL;
    json_t *piload = NULL;
    char *test = NULL;
    piload = json_object();
    fp = fopen("/mnt/internal_storage/sg_devagentsection", "w+");
    if (NULL == fp) {
        printf("Failed to open the file !\n");
        // exit(0);
    }
    json_object_set_new(piload, "rebootReason", json_integer(m_devSection.rebootReason));
    json_object_set_new(piload, "jobid", json_integer(m_devSection.jobid));
    json_object_set_new(piload, "midid", json_integer(m_devSection.midid));
    json_object_set_new(piload, "version", json_string(m_devSection.version));
    test = json_dumps(piload, JSON_PRESERVE_ORDER);
    fprintf(fp, "%s\n", test);
    // printf("write_jsonstr=%s\n",test);
    if (NULL != test) {
        free(test);
    }
    json_decref(piload);
    (void)fclose(fp);
}

// 读周期参数文件
int read_period_file(void)
{
    int filesize = 0;
    FILE *fp = NULL;
    char *buf = NULL;
    char real_path[PATH_MAX] = { 0 };
    json_t *piload = NULL;

    filesize = getfilesize("/mnt/internal_storage/sg_period_params");
    if (filesize <= 0) {
        return VOS_ERR;
    }

    buf = (char *)VOS_Malloc(MID_SGDEV, filesize + 1);
    if (buf == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "Error - unable to allocate required memory\n");
        return VOS_ERR;
    }
    (void)memset_s(buf, filesize + 1, 0, filesize + 1);

    if (realpath("/mnt/internal_storage/sg_period_params", real_path) == NULL) {
        return -1;
    }

    fp = fopen(real_path, "r");
    if (fp == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "Failed to open the file!\n");
        (void)VOS_Free(buf);
        return VOS_ERR;
    }

    if (!fread(buf, filesize, 1, fp)) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "read file error!\n");
        (void)fclose(fp);
        (void)VOS_Free(buf);
        return VOS_ERR;
    }

    piload = load_json(buf);
    (void)VOS_Free(buf);

    if (!json_is_object(piload)) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "%s,%d\n", __FILE__, __LINE__);
    }
    if (json_into_uint32_t(&m_devperiod.appperiod, piload, "appperiod") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_uint32_t(&m_devperiod.containerperiod, piload, "containerperiod") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_uint32_t(&m_devperiod.devperiod, piload, "devperiod") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_uint32_t(&m_devperiod.devheartbeatperiod, piload, "devheartbeatperiod") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    (void)json_decref(piload);

    return VOS_OK;
}

// 写周期参数文件
void sg_write_period_file(rep_period_s *paraobj)
{
    FILE *fp = NULL;
    json_t *piload = NULL;
    char *test = NULL;
    piload = json_object();
    fp = fopen("/mnt/internal_storage/sg_period_params", "w+");
    if (NULL == fp) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "Failed to open the file!\n");
    }
    json_object_set_new(piload, "appperiod", json_integer(paraobj->appPeriod));
    json_object_set_new(piload, "containerperiod", json_integer(paraobj->conPeriod));
    json_object_set_new(piload, "devperiod", json_integer(paraobj->devPeriod));
    json_object_set_new(piload, "devheartbeatperiod", json_integer(paraobj->heartPeriod));
    test = json_dumps(piload, JSON_PRESERVE_ORDER);
    if (NULL != test) {
        fprintf(fp, "%s\n", test);
        free(test);
    }
    json_decref(piload);
    (void)fclose(fp);
}
// 手动写参数文件
void write_param_file(void)
{
    FILE *fp = NULL;
    json_t *piload = NULL;
    char *test = NULL;
    piload = json_object();
    fp = fopen("/mnt/internal_storage/sg_devagentparams", "w+");
    if (NULL == fp) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "Failed to open the file!\n");
    }
    m_devParam.startmode = MODE_MQTT;
    sprintf_s(m_devParam.mqtttopicversion, DATA_BUF_F32_SIZE, "%s", "v1");
    sprintf_s(m_devParam.ip, DATA_BUF_F32_SIZE, "%s", "192.168.100.1");
    m_devParam.port = 1883;
    sprintf_s(m_devParam.devid, DATA_BUF_F32_SIZE, "%s", "T231234567890ABCDEFGHIJKLMN");
    sprintf_s(m_devParam.clientid, DATA_BUF_F32_SIZE, "%s", "1602733326565@ClientId");
    sprintf_s(m_devParam.user, DATA_BUF_F32_SIZE, "%s", "1602733320528@UserName");
    sprintf_s(m_devParam.password, DATA_BUF_F32_SIZE, "%s", "abc12321cba");

    json_object_set_new(piload, "startmode", json_integer(m_devParam.startmode));
    json_object_set_new(piload, "mqtttopicversion", json_string(m_devParam.mqtttopicversion));
    json_object_set_new(piload, "devid", json_string(m_devParam.devid));
    json_object_set_new(piload, "ip", json_string(m_devParam.ip));
    json_object_set_new(piload, "port", json_integer(m_devParam.port));
    json_object_set_new(piload, "clientid", json_string(m_devParam.clientid));
    json_object_set_new(piload, "user", json_string(m_devParam.user));
    json_object_set_new(piload, "password", json_string(m_devParam.password));
    test = json_dumps(piload, JSON_PRESERVE_ORDER);
    fprintf(fp, "%s\n", test);
    if (NULL != test) {
        free(test);
    }
    json_decref(piload);
    (void)fclose(fp);
}
// 读专检参数文件
int read_param_file(void)
{
    int filesize = 0;
    FILE *fp = NULL;
    char *buf = NULL;
    char real_path[PATH_MAX] = { 0 };
    json_t *piload = NULL;

    filesize = getfilesize("/mnt/internal_storage/sg_devagentparams");
    if (filesize <= 0) {
        return VOS_ERR;
    }

    buf = (char *)VOS_Malloc(MID_SGDEV, filesize + 1);
    if (buf == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "Error - unable to allocate required memory\n");
        return VOS_ERR;
    }
    (void)memset_s(buf, filesize + 1, 0, filesize + 1);

    if (realpath("/mnt/internal_storage/sg_devagentparams", real_path) == NULL) {
        return -1;
    }

    fp = fopen(real_path, "r");
    if (fp == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "Failed to open the file !\n");
        (void)VOS_Free(buf);
        return VOS_ERR;
    }

    if (!fread(buf, filesize, 1, fp)) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "read file error!\n");
        (void)fclose(fp);
        (void)VOS_Free(buf);
        return VOS_ERR;
    }

    piload = load_json(buf); // 将字符串转为json
    (void)VOS_Free(buf);

    if (!json_is_object(piload)) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "%s,%d\n", __FILE__, __LINE__);
    }
    if (json_into_uint8_t(&m_devParam.startmode, piload, "startmode") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_string(m_devParam.mqtttopicversion, piload, "mqtttopicversion") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_string(m_devParam.devid, piload, "devid") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_string(m_devParam.ip, piload, "ip") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_uint32_t(&m_devParam.port, piload, "port") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_string(m_devParam.clientid, piload, "clientid") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_string(m_devParam.user, piload, "user") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_string(m_devParam.password, piload, "password") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    (void)json_decref(piload);
    return VOS_OK;
}
// 返回设备参数
sg_dev_param_info_s sg_get_param(void)
{
    return m_devParam;
}
// 返回周期参数
sg_period_info_s sg_get_period(void)
{
    return m_devperiod;
}
// 获取断面
sg_dev_section_info_s sg_get_section(void)
{
    m_devSection.rebootReason = 0;
    return m_devSection;
}


