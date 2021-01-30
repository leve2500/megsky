#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"

#include "sgdev_struct.h"
#include "sgdev_debug.h"
#include "sgdev_param.h"

#include "mqtt_json.h"

json_t *load_json(const char *text)
{
    json_t *root = NULL;
    json_error_t error;

    root = json_loads(text, 0, &error);
    if (root) {
        return root;
    } else {
        fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
        return (json_t *)0;
    }
}

int msg_parse(json_t *root)   //�ж���Ϣ���ͣ������Ƿ���code�ж�������֡����Ӧ��֡
{
    json_t *judge_code = NULL;
    if (NULL == root) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "\n can not unpack ,root is null");
        return 0;
    }
    judge_code = json_object_get(root, "code");
    if (NULL == judge_code) {
        return TYPE_REQUEST;
    } else {
        return TYPE_REPLY;
    }
}

//���ݽ������� jsonתΪ����
int json_into_int8_t(int8_t *idata, json_t *jdata, const char *key)
{
    json_int_t value = 0;
    json_t *all_info = NULL;
    all_info = json_object_get(jdata, key);

    if (all_info == NULL || !json_is_integer(all_info)) {
        SGDEV_WARN(SYSLOG_LOG, SGDEV_MODULE, "key = %s, json_object_get fail\n", key);
        return VOS_ERR;
    }

    value = json_integer_value(all_info);

    *idata = (int8_t)value;
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "int8_idata = %d\n", *idata);
    return VOS_OK;
}

int json_into_uint8_t(uint8_t *idata, json_t *jdata, const char *key)
{
    json_int_t value = 0;
    json_t *all_info = NULL;
    all_info = json_object_get(jdata, key);

    if (all_info == NULL || !json_is_integer(all_info)) {
        SGDEV_WARN(SYSLOG_LOG, SGDEV_MODULE, "key = %s, json_object_get fail\n", key);
        return VOS_ERR;
    }

    value = json_integer_value(all_info);

    *idata = (uint8_t)value;
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "uint8_idata = %d\n", *idata);
    return VOS_OK;
}


int json_into_int32_t(int32_t *idata, json_t *jdata, const char *key)
{
    json_int_t value = 0;;
    json_t *all_info = NULL;
    all_info = json_object_get(jdata, key);

    if (all_info == NULL || !json_is_integer(all_info)) {
        SGDEV_WARN(SYSLOG_LOG, SGDEV_MODULE, "key = %s, json_object_get fail\n", key);
        return VOS_ERR;
    }
    value = json_integer_value(all_info);

    *idata = (int32_t)value;
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "int32_idata = %d\n", *idata);
    return VOS_OK;
}

int json_into_uint32_t(uint32_t *idata, json_t *jdata, const char *key)
{
    int ret = VOS_OK;
    json_int_t value = 0;
    json_t *all_info = NULL;
    all_info = json_object_get(jdata, key);

    if (all_info == NULL || !json_is_integer(all_info)) {
        SGDEV_WARN(SYSLOG_LOG, SGDEV_MODULE, "key = %s, json_object_get fail\n", key);
        ret = VOS_ERR;
    }
    value = json_integer_value(all_info);

    *idata = (uint32_t)value;
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "uint32_idata = %d\n", *idata);
    return ret;
}

//���ݽ������� jsonתΪ�ַ���
int json_into_array_string(char *sdata, json_t *jdata)
{
    char *test = NULL;
    
    if (jdata == NULL) {
        return VOS_ERR;
    }

    if (!json_is_string(jdata)) {
        return VOS_ERR;
    }

    test = json_string_value(jdata);
    memcpy_s(sdata, DATA_BUF_F256_SIZE, test, strlen(test) + 1);
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sdata = %s\n", sdata);
    return VOS_OK;
}

//json���ݽ������� jsonתΪ�ַ���
int json_into_string(char *sdata, json_t *jdata, const char *key)
{
    json_t *all_info = NULL;
    char *test       = NULL;

    if (jdata == NULL) {
        return VOS_ERR;
    }

    all_info = json_object_get(jdata, key);
    if (NULL == all_info || !json_is_string(all_info)) {
        SGDEV_WARN(SYSLOG_LOG, SGDEV_MODULE, "key = %s, json_object_get fail\n", key);
        return VOS_ERR;
    }
    test = json_string_value(all_info);
    memcpy_s(sdata, MSG_ARRVD_MAX_LEN, test, strlen(test) + 1);
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "sdata = %s\n", sdata);
    return VOS_OK;
}
/**
 * ��������g_parse_mqtt_json_string_value
 * ���ܣ�json���ݽ�������,��jsonתΪ�ַ�������
 * ���룺json_t *inParams
 * ������char *value, unsigned int value_len
 * ��ע����Ϊ�汾  ��������
 * */
int sg_parse_mqtt_json_string_value(json_t *inParams, const char *key, char *value, unsigned int value_len)
{
    json_t *propJson = NULL;
    errno_t err;
    const char *val = NULL;
    propJson = json_object_get(inParams, key);
    if (propJson == NULL) {
        return VOS_ERR;
    }
    val = json_string_value(propJson);
    if (val == NULL) {
        return VOS_ERR;
    }
    err = strcpy_s(value, value_len, val);
    if (err != EOK) {
        return VOS_ERR;
    }
    return VOS_OK;
}


//����file����  8�����б�ѡ����Ϊ��Ĳ����ж�
int sg_getfile(json_t *obj, file_info_s *fileobj)
{
    json_t* sign_info = NULL;
    if (obj != NULL && json_is_object(obj)) {
        if (json_into_string(fileobj->name, obj, "name") != VOS_OK) {
            return VOS_ERR;
        }
        json_into_string(fileobj->fileType, obj, "fileType");
        json_into_string(fileobj->url, obj, "url");
        if (json_into_uint32_t(&fileobj->size, obj, "size") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_string(fileobj->md5, obj, "md5") != VOS_OK) {
            return VOS_ERR;
        }
        sign_info = json_object_get(obj, "sign");        //������sign�����е�����
        if (sign_info != NULL && json_is_object(sign_info)) {
            if (json_into_string(fileobj->sign.name, sign_info, "name") != VOS_OK) {
                return VOS_ERR;
            }
            json_into_string(fileobj->sign.url, sign_info, "url");
            json_into_uint32_t(&fileobj->sign.size, sign_info, "size");
            json_into_string(fileobj->sign.md5, sign_info, "md5");
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}


/*****************************************************************************
�� �� ��  : MqttTimeStr
��������  : ����ʱ���
�������  :
�������  :
�� �� ֵ  :
���ú���  :
��������  :
*****************************************************************************/
int sg_mqtttimestr(time_t time_val, char *time_buff, size_t buff_len, int use_utc)
{
    int ret     = 0;
    struct tm t = { 0 };
    if (0 == use_utc) {
        (void)localtime_r(&time_val, &t);
    } else {                            //UTCʱ��
        (void)gmtime_r(&time_val, &t);
    }
    ret = sprintf_s(time_buff, buff_len, "%04d-%02d-%02dT%02d:%02d:%02dZ",
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return ret > 0 ? VOS_OK : VOS_ERR;
}


/*****************************************************************************
�� �� ��  : sg_unpack_json_msg_header_request
��������  : ��Ϣͷ��ȡ�ӿ� ������
�������  : root
�������  :
�� �� ֵ  : mqtt_request_header_s
���ú���  :
��������  :
*****************************************************************************/
mqtt_request_header_s *sg_unpack_json_msg_header_request(json_t *root)
{
    mqtt_request_header_s *header = NULL;
    header = (mqtt_request_header_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_request_header_s));
    (void)memset_s(header, sizeof(mqtt_request_header_s), 0, sizeof(mqtt_request_header_s));
    if (NULL == root) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "can not unpack, root is null\n");
        return header;
    }
    json_into_int32_t(&header->mid, root, "mid");
    json_into_string(header->deviceId, root, "deviceId");
    json_into_string(header->timestamp, root, "timestamp");
    json_into_string(header->type, root, "type");
    json_into_int32_t(&header->expire, root, "expire");
    header->param = json_object_get(root, "param");
    return header;
}
/*****************************************************************************
�� �� ��  : sg_unpack_json_msg_header_reply
��������  : ��Ϣͷ��ȡ�ӿ� Ӧ����
�������  : root
�������  :
�� �� ֵ  : mqtt_reply_header_s
���ú���  :
��������  :
*****************************************************************************/
mqtt_reply_header_s *sg_unpack_json_msg_header_reply(json_t *root)
{
    mqtt_reply_header_s *header = NULL;
    header = (mqtt_reply_header_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_reply_header_s));
    (void)memset_s(header, sizeof(mqtt_reply_header_s), 0, sizeof(mqtt_reply_header_s));
    header->param = NULL;
    if (NULL == root) {
        SGDEV_ERROR(SYSLOG_LOG, SGDEV_MODULE, "can not unpack ,root is null\n");
        return header;
    }
    json_into_int32_t(&header->mid, root, "mid");
    json_into_string(header->deviceId, root, "deviceId");
    json_into_string(header->timestamp, root, "timestamp");
    json_into_string(header->type, root, "type");
    json_into_int32_t(&header->code, root, "code");
    json_into_string(header->msg, root, "msg");
    header->param = json_object_get(root, "param");
    return header;
    // VOS_Free(header);
}
/****************************************************************************
�� �� ��  : sg_pack_json_msg_header
��������  : ��Ϣͷ��֡�ӿ�
�������  : code mid type param
�������  : char *
�� �� ֵ  :
���ú���  :
��������  :
*****************************************************************************/
char *sg_pack_json_msg_header(uint16_t code, int32_t mid, const char *type, const char *msg, json_t *param)
{
    time_t now_time;
    now_time = time(NULL);
    char timestamp[DATA_BUF_F32_SIZE] = { 0 };
    char *result = NULL;
    json_t *piload = NULL;
    piload = json_object();
    sg_dev_param_info_s idparam = sg_get_param();
    (void)json_object_set_new(piload, "deviceId", json_string(idparam.devid));  // ��ȡdeviceId
    sg_mqtttimestr(now_time, timestamp, sizeof(timestamp), TIME_UTC);           // ʹ��UTCʱ��
    if (timestamp != NULL) {
        (void)json_object_set_new(piload, "timestamp", json_string(timestamp));
    }
    if (code != CODE_NULL) {
        (void)json_object_set_new(piload, "code", json_integer(code));
    }
    if (mid != 0) {
        (void)json_object_set_new(piload, "mid", json_integer(mid));
    }
    if (type != NULL) {
        (void)json_object_set_new(piload, "type", json_string(type));
    }
    if (msg != NULL) {
        (void)json_object_set_new(piload, "msg", json_string(msg));
    }
    if (param != NULL) {
        (void)json_object_set_new(piload, "param", param);
    }
    result = json_dumps(piload, JSON_PRESERVE_ORDER);
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "result = %s\n", result);
    json_decref(piload);
    return result;
}

//����cfgCpu����
int sg_get_cfgcpu(json_t *obj, cfg_cpu_info_s *cfgcpuobj)
{
    if (json_is_object(obj)) {
        if (json_into_uint32_t(&cfgcpuobj->cpus, obj, "cpus") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_uint32_t(&cfgcpuobj->cpuLmt, obj, "cpuLmt") != VOS_OK) {
            return VOS_ERR;
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}
//����cfgMem����
int sg_get_cfgmem(json_t *obj, cfg_mem_info_s *cfgmemobj)
{
    if (json_is_object(obj)) {
        if (json_into_uint32_t(&cfgmemobj->memory, obj, "memory") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_uint32_t(&cfgmemobj->memLmt, obj, "memLmt") != VOS_OK) {
            return VOS_ERR;
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}
//����cfgDisk����
int sg_get_cfgdisk(json_t *obj, cfg_disk_info_s *cfgdiskobj)
{
    if (json_is_object(obj)) {
        if (json_into_uint32_t(&cfgdiskobj->disk, obj, "disk") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_uint32_t(&cfgdiskobj->diskLmt, obj, "diskLmt") != VOS_OK) {
            return VOS_ERR;
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}