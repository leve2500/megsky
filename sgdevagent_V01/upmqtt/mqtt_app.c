
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "vos_typdef.h"
#include "sgdev_struct.h"
#include "sgdev_param.h"
#include "mqtt_json.h"
#include "mqtt_app.h"
//应用安装控制命令
int sg_unpack_app_install_cmd(json_t *obj, app_install_cmd_s *cmdobj)
{
    json_t *file_info = NULL;
    json_t *cfgCpu_info = NULL;
    json_t *cfgMem_info = NULL;
    if (json_into_int32_t(&cmdobj->jobId, obj, "jobId") != VOS_OK) {
        return VOS_ERR;
    }
    json_into_uint32_t(&cmdobj->policy, obj, "policy");
    if (json_into_string(cmdobj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmdobj->version, obj, "version") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmdobj->enable, obj, "enable") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmdobj->app, obj, "app") != VOS_OK) {
        return VOS_ERR;
    }
    file_info = json_object_get(obj, "file");
    if (sg_getfile(file_info, &cmdobj->file) != VOS_OK) {
        return VOS_ERR;
    }
    cfgCpu_info = json_object_get(obj, "cfgCpu");
    if (sg_getcfgcpu(cfgCpu_info, &cmdobj->cfgCpu) != VOS_OK) {
        return VOS_ERR;
    }
    cfgMem_info = json_object_get(obj, "cfgMem");
    if (sg_getcfgmem(cfgMem_info, &cmdobj->cfgMem) != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}

//应用安装控制命令应答
void sg_pack_app_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_INSTALL, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//应用控制命令  启动  停止 卸载 使能  去使能  合成一个
int sg_unpack_app_control_cmd(json_t *obj, app_control_cmd_s *cmdobj)
{
    if (json_into_string(cmdobj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmdobj->app, obj, "app") != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
//应用启动应答 无param
void sg_pack_app_start_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_START, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//应用停止应答 无param
void sg_pack_app_stop_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_STOP, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//应用卸载应答 无param
void sg_pack_app_uninstall_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_REMOVE, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
////应用使能应答 无param
void sg_pack_app_enable_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_ENABLE, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//应用去使能应答 无param
void sg_pack_app_unenble_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_UNENABLE, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//应用配置修改命令
int sg_unpack_app_param_set_cmd(json_t *obj, app_conf_cmd_s *cmdobj)
{
    json_t *cfgCpu_info = NULL;
    json_t *cfgMem_info = NULL;
    if (json_into_string(cmdobj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmdobj->app, obj, "app") != VOS_OK) {
        return VOS_ERR;
    }
    cfgCpu_info = json_object_get(obj, "cfgCpu");
    if (json_is_object(cfgCpu_info)) {
        if (json_into_uint32_t(&cmdobj->cfgCpu.cpus, cfgCpu_info, "cpus") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_uint32_t(&cmdobj->cfgCpu.cpuLmt, cfgCpu_info, "cpuLmt") != VOS_OK) {
            return VOS_ERR;
        }
    }
    cfgMem_info = json_object_get(obj, "cfgMem");
    if (json_is_object(cfgMem_info)) {
        if (json_into_uint32_t(&cmdobj->cfgMem.memory, cfgMem_info, "memory") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_uint32_t(&cmdobj->cfgMem.memLmt, cfgMem_info, "memLmt") != VOS_OK) {
            return VOS_ERR;
        }
    }
    return VOS_OK;
}
//应用配置修改应答
void sg_pack_app_param_set_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_SET_CONFIG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//应用配置状态查询命令
int sg_unpack_app_param_get(json_t *obj, char *msg)
{
    if (NULL != msg) {
        json_into_string(msg, obj, "container");
        return VOS_OK;
    } else {
        return VOS_ERR;
    }
}

//应用配置查询应答
void sg_pack_app_param_get_reply(uint16_t code, int32_t mid, const char *error_msg, app_conf_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    uint16_t len = 0;
    json_t *param = NULL;
    json_t *appCfgsobj = NULL;
    json_t *appCfgsobjs = NULL;
    json_t *cfgCpuobj = NULL;
    json_t *cfgMemobj = NULL;
    param = json_object();
    appCfgsobjs = json_array();
    if (NULL != appCfgsobjs) {
        len = statusobj->app_num;
        for (i = 0; i < len; i++) {
            appCfgsobj = json_object();
            cfgCpuobj = json_object();
            cfgMemobj = json_object();
            if (NULL != appCfgsobj) {
                json_object_set_new(appCfgsobj, "app", json_string(statusobj->appCfgs[i].app));
                if (NULL != cfgCpuobj) {
                    json_object_set_new(cfgCpuobj, "cpus", json_integer(statusobj->appCfgs[i].cfgCpu.cpus));
                    json_object_set_new(cfgCpuobj, "cpuLmt", json_integer(statusobj->appCfgs[i].cfgCpu.cpuLmt));
                }
                json_object_set_new(appCfgsobj, "cfgCpu", cfgCpuobj);
                if (NULL != cfgMemobj) {
                    json_object_set_new(cfgMemobj, "memory", json_integer(statusobj->appCfgs[i].cfgMem.memory));
                    json_object_set_new(cfgMemobj, "memLmt", json_integer(statusobj->appCfgs[i].cfgMem.memLmt));
                }
                json_object_set_new(appCfgsobj, "cfgMem", cfgMemobj);
                json_array_append_new(appCfgsobjs, appCfgsobj);
            }
        }
    }
    json_object_set_new(param, "container", json_string(statusobj->container));
    json_object_set_new(param, "appCfgs", appCfgsobjs);
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_GET_CONFIG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//应用状态查询命令
int sg_unpack_app_status_get(json_t *obj, char *msg)
{
    if (NULL != msg) {
        json_into_string(msg, obj, "container");
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}

//获取apps结构体数组中的process结构体数组内容
static void sg_getprocess(json_t *appsobj_s, process_info_s *infobj, uint32_t process_num)
{
    uint16_t t = 0;
    json_t *appsobj = NULL;
    if(infobj == NULL) {
        return;
    }
    if (NULL != appsobj_s) {
        for (t = 0; t < process_num; t++) {
            appsobj = json_object();
            if (NULL != appsobj) {
                json_object_set_new(appsobj, "srvIndex", json_integer(infobj[t].srvIndex));
                json_object_set_new(appsobj, "srvName", json_string(infobj[t].srvName));
                json_object_set_new(appsobj, "srvEnable", json_string(infobj[t].srvEnable));
                json_object_set_new(appsobj, "srvStatus", json_string(infobj[t].srvStatus));
                json_object_set_new(appsobj, "cpuLmt", json_integer(infobj[t].cpuLmt));
                json_object_set_new(appsobj, "cpuRate", json_integer(infobj[t].cpuRate));
                json_object_set_new(appsobj, "memLmt", json_integer(infobj[t].memLmt));
                json_object_set_new(appsobj, "memUsed", json_integer(infobj[t].memUsed));
                json_object_set_new(appsobj, "startTime", json_string(infobj[t].startTime));
                json_array_append_new(appsobj_s, appsobj);
            }
        }
    }
}
//应用状态查询应答
void sg_pack_app_status_get_reply(uint16_t code, int32_t mid, const char *error_msg, app_inq_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    uint16_t len = 0;
    json_t *param = NULL;
    json_t *appobj = NULL;
    json_t *appobjs = NULL;
    json_t *getprocessobjs = NULL;
    param = json_object();
    appobjs = json_array();
    if (NULL != appobjs) {
        len = statusobj->apps_num;
        for (i = 0; i < len; i++) {
            appobj = json_object();
            getprocessobjs = json_array();
            if (NULL != appobj) {
                json_object_set_new(appobj, "app", json_string(statusobj->apps[i].app));
                json_object_set_new(appobj, "version", json_string(statusobj->apps[i].version));
                json_object_set_new(appobj, "appHash", json_string(statusobj->apps[i].appHash));
                json_object_set_new(appobj, "srvNumber", json_integer(statusobj->apps[i].srvNumber));
                sg_getprocess(getprocessobjs, statusobj->apps[i].process, statusobj->apps[i].srvNumber);
                json_object_set_new(appobj, "process", getprocessobjs);
                json_array_append_new(appobjs, appobj);
            }
        }
    }
    json_object_set_new(param, "container", json_string(statusobj->container));
    json_object_set_new(param, "apps", appobjs);
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_STATUS, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//应用状态上报
void sg_pack_app_status_get_data(uint16_t code, int32_t mid, const char *error_msg, app_status_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    uint16_t len = 0;
    json_t *param = NULL;
    json_t *appobj = NULL;
    json_t *appobjs = NULL;
    json_t *getprocessobjs = NULL;
    param = json_object();
    appobjs = json_array();
    if (NULL != appobjs) {
        len = statusobj->apps_num;
        for (i = 0; i < len; i++) {
            appobj = json_object();
            getprocessobjs = json_object();
            if (NULL != appobj) {
                json_object_set_new(appobj, "app", json_string(statusobj->apps[i].app));
                json_object_set_new(appobj, "version", json_string(statusobj->apps[i].version));
                json_object_set_new(appobj, "appHash", json_string(statusobj->apps[i].appHash));
                json_object_set_new(appobj, "srvNumber", json_integer(statusobj->apps[i].srvNumber));
                sg_getprocess(getprocessobjs, statusobj->apps[i].process, statusobj->apps[i].srvNumber);
                json_object_set_new(appobj, "process", getprocessobjs);
                json_array_append_new(appobjs, appobj);
            }
        }
    }
    json_object_set_new(param, "container", json_string(statusobj->container));
    json_object_set_new(param, "apps", appobjs);
    char *mssm = sg_pack_json_msg_header(code, mid, REP_APP_STATUS, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//应用事件上报
void sg_pack_app_event_pack(uint16_t code, int32_t mid, const char *error_msg, app_event_reply_s *statusobj, char *msg)
{
    char *mssm = NULL;
    json_t *param = NULL;
    param = json_object();
    json_object_set_new(param, "container", json_string(statusobj->container));
    if (strlen(statusobj->app) != 0) {
        json_object_set_new(param, "app", json_string(statusobj->app));
    }
    json_object_set_new(param, "event", json_string(statusobj->event));
    if (strlen(statusobj->msg) != 0) {
        json_object_set_new(param, "msg", json_string(statusobj->msg));
    }
    mssm = sg_pack_json_msg_header(code, mid, EVENT_APP_ALARM, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//应用升级命令
int sg_unpack_app_upgrade_cmd(json_t *obj, app_upgrade_cmd_s *cmdobj)
{
    json_t* file_info = NULL;
    //************************开始解析数据*****************************************
    if (json_into_int32_t(&cmdobj->jobId, obj, "jobId") != VOS_OK) {
        return VOS_ERR;
    }
    json_into_uint32_t(&cmdobj->policy, obj, "policy");
    if (json_into_string(cmdobj->version, obj, "version") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmdobj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    file_info = json_object_get(obj, "file");
    if (sg_getfile(file_info, &cmdobj->file) != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
//应用升级命令应答
void sg_pack_app_upgrade_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    param = json_object();
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_UPGRADE, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//应用日志召回命令
int sg_unpack_app_log_get_cmd(json_t *obj, app_log_recall_cmd_s *cmdobj)
{
    if (json_into_string(cmdobj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    if (json_into_string(cmdobj->url, obj, "url") != VOS_OK) {
        return VOS_ERR;
    }
    json_into_string(cmdobj->app, obj, "app");
    return VOS_OK;
}
//应用日志召回应答
void sg_pack_app_log_get_reply(uint16_t code, int32_t mid, const char *error_msg, file_info_s *file, char *msg)
{
    json_t *param = NULL;
    json_t *sign_object = NULL;
    param = json_object();
    sign_object = json_object();
    if (NULL != sign_object) {
        json_object_set_new(sign_object, "name", json_string(file->sign.name));
        if (strlen(file->sign.url) != 0) {
            json_object_set_new(sign_object, "url", json_string(file->sign.url));
        }
        if (file->sign.size != 0) {
            json_object_set_new(sign_object, "size", json_integer(file->sign.size));
        }
        if (strlen(file->sign.md5) != 0) {
            json_object_set_new(sign_object, "md5", json_string(file->sign.md5));
        }
    }
    json_object_set_new(param, "name", json_string(file->name));
    if (strlen(file->fileType) != 0) {
        json_object_set_new(param, "fileType", json_string(file->fileType));
    }
    if (strlen(file->url) != 0) {
        json_object_set_new(param, "url", json_string(file->url));
    }
    json_object_set_new(param, "size", json_integer(file->size));
    json_object_set_new(param, "md5", json_string(file->md5));
    json_object_set_new(param, "sign", sign_object);
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_APP_LOG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

