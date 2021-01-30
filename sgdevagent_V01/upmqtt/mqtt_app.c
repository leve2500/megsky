#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"

#include "sgdev_struct.h"
#include "sgdev_param.h"
#include "mqtt_json.h"
#include "mqtt_app.h"

// Ӧ�ð�װ��������
int sg_unpack_app_install_cmd(json_t *obj, app_install_cmd_s *cmd_obj)
{
    json_t *file_info = NULL;
    json_t *cfgCpu_info = NULL;
    json_t *cfgMem_info = NULL;
    if (json_into_int32_t(&cmd_obj->jobId, obj, "jobId") != VOS_OK) {
        return VOS_ERR;
    }
    (void)json_into_uint32_t(&cmd_obj->policy, obj, "policy");
    if (json_into_string(cmd_obj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmd_obj->version, obj, "version") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmd_obj->enable, obj, "enable") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmd_obj->app, obj, "app") != VOS_OK) {
        return VOS_ERR;
    }
    file_info = json_object_get(obj, "file");
    if (sg_getfile(file_info, &cmd_obj->file) != VOS_OK) {
        return VOS_ERR;
    }
    cfgCpu_info = json_object_get(obj, "cfgCpu");
    (void)sg_get_cfgcpu(cfgCpu_info, &cmd_obj->cfgCpu);

    cfgMem_info = json_object_get(obj, "cfgMem");
    (void)sg_get_cfgmem(cfgMem_info, &cmd_obj->cfgMem);
    return VOS_OK;
}

// Ӧ�ð�װ��������Ӧ��
int sg_pack_app_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    char *mssm = NULL;
    mssm = sg_pack_json_msg_header(code, mid, CMD_APP_INSTALL, error_msg, NULL);
    if (mssm == NULL) {
        return VOS_ERR;
    }
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    free(mssm); 
    return VOS_OK;
}

// Ӧ�ÿ������� ���� ֹͣ ж�� ʹ�� ȥʹ�� �ϳ�һ��
int sg_unpack_app_control_cmd(json_t *obj, app_control_cmd_s *cmd_obj)
{
    if (json_into_string(cmd_obj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmd_obj->app, obj, "app") != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
// Ӧ������Ӧ�� ��param
int sg_pack_app_start_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    char *mssm = NULL;
    mssm = sg_pack_json_msg_header(code, mid, CMD_APP_START, error_msg, NULL);
    if (mssm == NULL) {
        return VOS_ERR;
    }
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    free(mssm);
    return VOS_OK;
}

// Ӧ��ֹͣӦ�� ��param
int sg_pack_app_stop_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    char *mssm = NULL;
    mssm = sg_pack_json_msg_header(code, mid, CMD_APP_STOP, error_msg, NULL);
    if (mssm == NULL) {
        return VOS_ERR;
    }
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    free(mssm);
    return VOS_OK;
}
// Ӧ��ж��Ӧ�� ��param
int sg_pack_app_uninstall_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    char *mssm = NULL;
    mssm = sg_pack_json_msg_header(code, mid, CMD_APP_REMOVE, error_msg, NULL);
    if (mssm == NULL) {
        return VOS_ERR;
    }
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    free(mssm);
    return VOS_OK;
}
// Ӧ��ʹ��Ӧ�� ��param
int sg_pack_app_enable_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    char *mssm = NULL;
    mssm = sg_pack_json_msg_header(code, mid, CMD_APP_ENABLE, error_msg, NULL);
    if (mssm == NULL) {
        return VOS_ERR;
    }
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    free(mssm);
    return VOS_OK;
}
// Ӧ��ȥʹ��Ӧ�� ��param
int sg_pack_app_unenble_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    char *mssm = NULL;
    mssm = sg_pack_json_msg_header(code, mid, CMD_APP_UNENABLE, error_msg, NULL);
    if (mssm == NULL) {
        return VOS_ERR;
    }
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    free(mssm);
    return VOS_OK;
}
// Ӧ�������޸�����
int sg_unpack_app_param_set_cmd(json_t *obj, app_conf_cmd_s *cmd_obj)
{
    json_t *cfgCpu_info = NULL;
    json_t *cfgMem_info = NULL;
    
    if (json_into_string(cmd_obj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmd_obj->app, obj, "app") != VOS_OK) {
        return VOS_ERR;
    }
    cfgCpu_info = json_object_get(obj, "cfgCpu");
    if (json_is_object(cfgCpu_info)) {
        if (json_into_uint32_t(&cmd_obj->cfgCpu.cpus, cfgCpu_info, "cpus") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_uint32_t(&cmd_obj->cfgCpu.cpuLmt, cfgCpu_info, "cpuLmt") != VOS_OK) {
            return VOS_ERR;
        }
    }
    cfgMem_info = json_object_get(obj, "cfgMem");
    if (json_is_object(cfgMem_info)) {
        if (json_into_uint32_t(&cmd_obj->cfgMem.memory, cfgMem_info, "memory") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_uint32_t(&cmd_obj->cfgMem.memLmt, cfgMem_info, "memLmt") != VOS_OK) {
            return VOS_ERR;
        }
    }
    return VOS_OK;
}
// Ӧ�������޸�Ӧ�� ��param
int sg_pack_app_param_set_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    char *mssm = NULL;
    mssm = sg_pack_json_msg_header(code, mid, CMD_APP_SET_CONFIG, error_msg, NULL);
    if (mssm == NULL) {
        return VOS_ERR;
    }
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    free(mssm);
    return VOS_OK;
}

// Ӧ������״̬��ѯ����
int sg_unpack_app_param_get(json_t *obj, char *msg)
{
    if (obj == NULL) {
        return VOS_ERR;
    }
    if (json_into_string(msg, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}

// Ӧ�����ò�ѯӦ��
void sg_pack_app_param_get_reply(uint16_t code, int32_t mid, const char *error_msg,
        app_conf_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    json_t *param = NULL;
    json_t *appCfgsobj = NULL;
    json_t *appCfgsobjs = NULL;
    json_t *cfgCpuobj = NULL;
    json_t *cfgMemobj = NULL;

    param = json_object();
    appCfgsobjs = json_array();
    if (appCfgsobjs != NULL) {
        for (i = 0; i < statusobj->app_num; i++) {
            appCfgsobj = json_object();
            cfgCpuobj = json_object();
            cfgMemobj = json_object();
            if (appCfgsobj != NULL) {
                (void)json_object_set_new(appCfgsobj, "app", json_string(statusobj->appCfgs[i].app));
                if (cfgCpuobj != NULL) {
                    (void)json_object_set_new(cfgCpuobj, "cpus", json_integer(statusobj->appCfgs[i].cfgCpu.cpus));
                    (void)json_object_set_new(cfgCpuobj, "cpuLmt", json_integer(statusobj->appCfgs[i].cfgCpu.cpuLmt));
                }
                (void)json_object_set_new(appCfgsobj, "cfgCpu", cfgCpuobj);
                if (cfgMemobj != NULL) {
                    (void)json_object_set_new(cfgMemobj, "memory", json_integer(statusobj->appCfgs[i].cfgMem.memory));
                    (void)json_object_set_new(cfgMemobj, "memLmt", json_integer(statusobj->appCfgs[i].cfgMem.memLmt));
                }
                (void)json_object_set_new(appCfgsobj, "cfgMem", cfgMemobj);
                json_array_append_new(appCfgsobjs, appCfgsobj);
            }
        }
    }
    (void)json_object_set_new(param, "container", json_string(statusobj->container));
    (void)json_object_set_new(param, "appCfgs", appCfgsobjs);
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_GET_CONFIG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (mssm != NULL) {
        free(mssm);
    }
    if (param != NULL) {
        json_decref(param);
    }
}

// Ӧ��״̬��ѯ����
int sg_unpack_app_status_get(json_t *obj, char *msg)
{
    if (obj == NULL) {
        return VOS_ERR;
    }
    if (json_into_string(msg, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}

// ��ȡapps�ṹ�������е�process�ṹ����������
static void sg_getprocess(json_t *appsobj_s, process_info_s *infobj, uint32_t process_num)
{
    uint16_t t = 0;
    json_t *appsobj = NULL;

    if(infobj == NULL) {
        return;
    }
    if (appsobj_s != NULL) {
        for (t = 0; t < process_num; t++) {
            appsobj = json_object();
            if (appsobj != NULL) {
                (void)json_object_set_new(appsobj, "srvIndex", json_integer(infobj[t].srvIndex));
                (void)json_object_set_new(appsobj, "srvName", json_string(infobj[t].srvName));
                (void)json_object_set_new(appsobj, "srvEnable", json_string(infobj[t].srvEnable));
                (void)json_object_set_new(appsobj, "srvStatus", json_string(infobj[t].srvStatus));
                (void)json_object_set_new(appsobj, "cpuLmt", json_integer(infobj[t].cpuLmt));
                (void)json_object_set_new(appsobj, "cpuRate", json_integer(infobj[t].cpuRate));
                (void)json_object_set_new(appsobj, "memLmt", json_integer(infobj[t].memLmt));
                (void)json_object_set_new(appsobj, "memUsed", json_integer(infobj[t].memUsed));
                (void)json_object_set_new(appsobj, "startTime", json_string(infobj[t].startTime));
                json_array_append_new(appsobj_s, appsobj);
            }
        }
    }
}
// Ӧ��״̬��ѯӦ��
void sg_pack_app_status_get_reply(uint16_t code, int32_t mid, const char *error_msg,
        app_inq_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    uint16_t len = 0;
    json_t *param = NULL;
    json_t *appobj = NULL;
    json_t *appobjs = NULL;
    json_t *getprocessobjs = NULL;

    param = json_object();
    appobjs = json_array();
    if (appobjs != NULL) {
        len = statusobj->apps_num;
        for (i = 0; i < len; i++) {
            appobj = json_object();
            getprocessobjs = json_array();
            if (appobj != NULL) {
                (void)json_object_set_new(appobj, "app", json_string(statusobj->apps[i].app));
                (void)json_object_set_new(appobj, "version", json_string(statusobj->apps[i].version));
                (void)json_object_set_new(appobj, "appHash", json_string(statusobj->apps[i].appHash));
                (void)json_object_set_new(appobj, "srvNumber", json_integer(statusobj->apps[i].srvNumber));
                sg_getprocess(getprocessobjs, statusobj->apps[i].process, statusobj->apps[i].srvNumber);
                (void)json_object_set_new(appobj, "process", getprocessobjs);
                json_array_append_new(appobjs, appobj);
            }
        }
    }
    (void)json_object_set_new(param, "container", json_string(statusobj->container));
    (void)json_object_set_new(param, "apps", appobjs);
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_STATUS, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (mssm != NULL) {
        free(mssm);
    }
    if (param != NULL) {
        json_decref(param);
    }
}
// Ӧ��״̬�ϱ�
void sg_pack_app_status_get_data(uint16_t code, int32_t mid, const char *error_msg,
        app_status_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    uint16_t len = 0;
    json_t *param = NULL;
    json_t *appobj = NULL;
    json_t *appobjs = NULL;
    json_t *getprocessobjs = NULL;

    param = json_object();
    appobjs = json_array();
    if (appobjs != NULL) {
        len = statusobj->apps_num;
        for (i = 0; i < len; i++) {
            appobj = json_object();
            getprocessobjs = json_object();
            if (appobj != NULL) {
                (void)json_object_set_new(appobj, "app", json_string(statusobj->apps[i].app));
                (void)json_object_set_new(appobj, "version", json_string(statusobj->apps[i].version));
                (void)json_object_set_new(appobj, "appHash", json_string(statusobj->apps[i].appHash));
                (void)json_object_set_new(appobj, "srvNumber", json_integer(statusobj->apps[i].srvNumber));
                sg_getprocess(getprocessobjs, statusobj->apps[i].process, statusobj->apps[i].srvNumber);
                (void)json_object_set_new(appobj, "process", getprocessobjs);
                json_array_append_new(appobjs, appobj);
            }
        }
    }
    (void)json_object_set_new(param, "container", json_string(statusobj->container));
    (void)json_object_set_new(param, "apps", appobjs);
    char *mssm = sg_pack_json_msg_header(code, mid, REP_APP_STATUS, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (mssm != NULL) {
        free(mssm);
    }
    if (param != NULL) {
        json_decref(param);
    }
}
// Ӧ���¼��ϱ�
void sg_pack_app_event_pack(uint16_t code, int32_t mid, const char *error_msg, app_event_reply_s *statusobj, char *msg)
{
    char *mssm = NULL;
    json_t *param = NULL;

    param = json_object();
    (void)json_object_set_new(param, "container", json_string(statusobj->container));
    if (strlen(statusobj->app) != 0) {
        (void)json_object_set_new(param, "app", json_string(statusobj->app));
    }
    (void)json_object_set_new(param, "event", json_string(statusobj->event));
    if (strlen(statusobj->msg) != 0) {
        (void)json_object_set_new(param, "msg", json_string(statusobj->msg));
    }
    mssm = sg_pack_json_msg_header(code, mid, EVENT_APP_ALARM, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (mssm != NULL) {
        free(mssm);
    }
    if (param != NULL) {
        json_decref(param);
    }
}
// Ӧ����������
int sg_unpack_app_upgrade_cmd(json_t *obj, app_upgrade_cmd_s *cmd_obj)
{
    json_t* file_info = NULL;
    if (json_into_int32_t(&cmd_obj->jobId, obj, "jobId") != VOS_OK) {
        return VOS_ERR;
    }
    json_into_uint32_t(&cmd_obj->policy, obj, "policy");
    if (json_into_string(cmd_obj->version, obj, "version") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmd_obj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    file_info = json_object_get(obj, "file");
    if (sg_getfile(file_info, &cmd_obj->file) != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
// Ӧ����������Ӧ�� ��param
int sg_pack_app_upgrade_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    char *mssm = NULL;

    mssm = sg_pack_json_msg_header(code, mid, CMD_APP_UPGRADE, error_msg, NULL);
    if (mssm == NULL) {
        return VOS_ERR;
    }
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    free(mssm);
    return VOS_OK;
}
// Ӧ����־�ٻ�����
int sg_unpack_app_log_get_cmd(json_t *obj, app_log_recall_cmd_s *cmd_obj)
{
    if (json_into_string(cmd_obj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmd_obj->url, obj, "url") != VOS_OK) {
        return VOS_ERR;
    }
    (void)json_into_string(cmd_obj->app, obj, "app");
    return VOS_OK;
}
// Ӧ����־�ٻ�Ӧ��
void sg_pack_app_log_get_reply(uint16_t code, int32_t mid, const char *error_msg, file_info_s *file, char *msg)
{
    json_t *param = NULL;
    json_t *sign_object = NULL;

    param = json_object();
    sign_object = json_object();
    if (sign_object != NULL) {
        (void)json_object_set_new(sign_object, "name", json_string(file->sign.name));
        if (strlen(file->sign.url) != 0) {
            (void)json_object_set_new(sign_object, "url", json_string(file->sign.url));
        }
        if (file->sign.size != 0) {
            (void)json_object_set_new(sign_object, "size", json_integer(file->sign.size));
        }
        if (strlen(file->sign.md5) != 0) {
            (void)json_object_set_new(sign_object, "md5", json_string(file->sign.md5));
        }
    }
    (void)json_object_set_new(param, "name", json_string(file->name));
    if (strlen(file->fileType) != 0) {
        (void)json_object_set_new(param, "fileType", json_string(file->fileType));
    }
    if (strlen(file->url) != 0) {
        (void)json_object_set_new(param, "url", json_string(file->url));
    }
    (void)json_object_set_new(param, "size", json_integer(file->size));
    (void)json_object_set_new(param, "md5", json_string(file->md5));
    (void)json_object_set_new(param, "sign", sign_object);
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_LOG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (mssm != NULL) {
        free(mssm);
    }
    if (param != NULL) {
        json_decref(param);
    }
}

