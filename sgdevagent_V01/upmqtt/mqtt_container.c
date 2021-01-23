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
#include "mqtt_container.h"

//解析withAPP对象
static int sg_getwithapp(json_t *obj, with_app_info_s *withappobj)
{
    json_t *withApp_file_info = NULL;
    json_t *withApp_cfgCpu_info = NULL;
    json_t *withApp_cfgMem_info = NULL;
    if (json_is_object(obj)) {
        if (json_into_string(withappobj->version, obj, "version") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_string(withappobj->enable, obj, "enable") != VOS_OK) {
            return VOS_ERR;
        }
        withApp_file_info = json_object_get(obj, "file");
        if (sg_getfile(withApp_file_info, &withappobj->file) != VOS_OK) {
            return VOS_ERR;
        }
        withApp_cfgCpu_info = json_object_get(obj, "cfgCpu");
        if (withApp_cfgCpu_info != NULL && json_is_object(withApp_cfgCpu_info)) {
            if (json_into_uint32_t(&withappobj->cfgCpu.cpus, withApp_cfgCpu_info, "cpus") != VOS_OK) {
                return VOS_ERR;
            }
            if (json_into_uint32_t(&withappobj->cfgCpu.cpuLmt, withApp_cfgCpu_info, "cpuLmt") != VOS_OK) {
                return VOS_ERR;
            }
        }
        withApp_cfgMem_info = json_object_get(obj, "cfgMem");
        if (withApp_cfgMem_info != NULL && json_is_object(withApp_cfgMem_info)) {
            if (json_into_uint32_t(&withappobj->cfgMem.memory, withApp_cfgMem_info, "memory") != VOS_OK) {
                return VOS_ERR;
            }
            if (json_into_uint32_t(&withappobj->cfgMem.memLmt, withApp_cfgMem_info, "memLmt") != VOS_OK) {
                return VOS_ERR;
            }
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}

//容器安装控制 
int sg_unpack_container_install_cmd(json_t *obj, container_install_cmd_s *cmdobj)
{
    int i = 0;
    json_t *image_info = NULL;
    json_t *withAPP_info = NULL;
    json_t *cfgCpu_info = NULL;
    json_t *cfgMem_info = NULL;
    json_t *cfgDisk_info = NULL;
    json_t *mount_info = NULL;
    json_t *dev_info = NULL;

    if (json_into_int32_t(&cmdobj->jobId, obj, "jobId") != VOS_OK) {      //1层,先解析最外层的数据
        return VOS_ERR;
    }
    json_into_uint32_t(&cmdobj->policy, obj, "policy");
    if (json_into_string(cmdobj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    json_into_string(cmdobj->port, obj, "port");
    mount_info = json_object_get(obj, "mount");
    if (mount_info != NULL) {
        cmdobj->mount_len = json_array_size(mount_info);
        for (i = 0; i < cmdobj->mount_len; i++) {
            json_into_array_string(cmdobj->mount[i],json_array_get(mount_info, i));
        }
    }

    dev_info = json_object_get(obj, "dev");
    if (dev_info != NULL) {
        cmdobj->dev_len = json_array_size(dev_info);
        for (i = 0; i < cmdobj->dev_len; i++) {
            json_into_array_string(cmdobj->dev[i], json_array_get(dev_info, i));
        }
    }
    image_info = json_object_get(obj, "image");        //2层,再解析image对象中的数据
    if (image_info != NULL) {
        if (sg_getfile(image_info, &cmdobj->image) != VOS_OK) {
            return VOS_ERR;
        }
    }

    withAPP_info = json_object_get(obj, "withAPP");  //2层,解析withAPP_info对象中的数据
    if (withAPP_info != NULL) {
        if (sg_getwithapp(withAPP_info, &cmdobj->withAPP) != VOS_OK) {
            return VOS_ERR;
        }
    }

    cfgCpu_info = json_object_get(obj, "cfgCpu");   //2层,解析cfgCpu_info对象中的数据
    if (cfgCpu_info != NULL) {
        if (sg_getcfgcpu(cfgCpu_info, &cmdobj->cfgCpu) != VOS_OK) {
            return VOS_ERR;
        }
    }

    cfgMem_info = json_object_get(obj, "cfgMem");                                   //2层,解析cfgMem_info对象中的数据
    if (cfgMem_info != NULL) {
        if (sg_getcfgmem(cfgMem_info, &cmdobj->cfgMem) != VOS_OK) {
            return VOS_ERR;
        }
    }

    cfgDisk_info = json_object_get(obj, "cfgDisk");                                 //2层,解析cfgDisk_info对象中的数据
    if (cfgDisk_info != NULL) {
        if (sg_getcfgdisk(cfgDisk_info, &cmdobj->cfgDisk) != VOS_OK) {
            return VOS_ERR;
        }
    }
    return VOS_OK;
}

//容器安装控制命令应答
void sg_pack_container_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CON_INSTALL, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
/*****************************************************************************
函 数 名  : sg_unpack_container_control_cmd
功能描述  : 容器控制 (包括启动，停止，删除)
输入参数  : json_t *obj
输出参数  : *msg
返 回 值  :
调用函数  :
被调函数  :
*****************************************************************************/
int sg_unpack_container_control_cmd(json_t *obj, char *msg)
{
    if (NULL != msg) {
        json_into_string(msg, obj, "container");
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}
//容器启动控制命令应答
void sg_pack_container_start_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CON_START, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//容器停止控制应答
void sg_pack_container_stop_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CON_STOP, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//容器删除控制应答
void sg_pack_container_remove_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CON_REMOVE, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//容器配置修改命令
int sg_unpack_container_param_set_cmd(json_t *obj, container_conf_cmd_s *cmdobj)
{
    uint16_t i = 0;
    uint16_t len = 0;
    json_t *cfgCpu_info = NULL;
    json_t *cfgMem_info = NULL;
    json_t *cfgDisk_info = NULL;
    json_t *mount_info = NULL;
    json_t *dev_info = NULL;
    json_t *mount_table = NULL;
    json_t *dev_table = NULL;
    char *array_mount_str = NULL;
    char *array_dev_str = NULL;
    if (json_into_string(cmdobj->container, obj, "container") != VOS_OK) {
        return VOS_ERR;
    }
    json_into_string(cmdobj->port, obj, "port");
    cfgCpu_info = json_object_get(obj, "cfgCpu");
    if (sg_getcfgcpu(cfgCpu_info, &cmdobj->cfgCpu) != VOS_OK) {
        return VOS_ERR;
    }
    cfgMem_info = json_object_get(obj, "cfgMem");
    if (sg_getcfgmem(cfgMem_info, &cmdobj->cfgMem) != VOS_OK) {
        return VOS_ERR;
    }
    cfgDisk_info = json_object_get(obj, "cfgDisk");
    if (sg_getcfgdisk(cfgDisk_info, &cmdobj->cfgDisk) != VOS_OK) {
        return VOS_ERR;
    }
    mount_info = json_object_get(obj, "mount");
    len = cmdobj->mount_len;
    for (i = 0; i < len; i++) {
        mount_table = json_array_get(mount_info, i);
        array_mount_str = json_string_value(mount_table);
        memcpy_s(cmdobj->mount[i], MSG_ARRVD_MAX_LEN, array_mount_str, strlen(array_mount_str) + 1);
    }
    dev_info = json_object_get(obj, "dev");
    len = cmdobj->dev_len;
    for (i = 0; i < len; i++) {
        dev_table = json_array_get(dev_info, i);
        array_dev_str = json_string_value(dev_table);
        memcpy_s(cmdobj->dev[i], MSG_ARRVD_MAX_LEN, array_dev_str, strlen(array_dev_str) + 1);
    }
    return VOS_OK;
}

//容器配置修改应答 无param
void sg_pack_container_param_set_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CON_SET_CONFIG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

static void sg_setmount(json_t *mountsobj_s, container_conf_cmd_s infobj)
{
    uint16_t t = 0;
    uint16_t lent = 0;
    json_t *mountobj_s = NULL;
    mountsobj_s = json_array();
    if (NULL != mountsobj_s) {
        lent = infobj.mount_len;
        for (t = 0; t < lent; t++) {
            mountobj_s = json_object();
            if (NULL != mountobj_s) {
                mountobj_s = json_string(infobj.mount[t]);
                json_array_append_new(mountsobj_s, mountobj_s);
            }
        }
    }
}

static void sg_setdev(json_t *devsobj_s, container_conf_cmd_s infobj)
{
    uint16_t t = 0;
    uint16_t lent = 0;
    json_t *devobj_s = NULL;
    devsobj_s = json_array();
    if (NULL != devsobj_s) {
        lent = infobj.dev_len;
        for (t = 0; t < lent; t++) {
            devobj_s = json_object();
            if (NULL != devobj_s) {
                devobj_s = json_string(infobj.dev[t]);
                json_array_append_new(devsobj_s, devobj_s);
            }
        }
    }
}

//容器配置状态查询应答  优化比较复杂——先略过-------------------------------------------------------------------------------
void sg_pack_container_param_get_reply(uint16_t code, int32_t mid, const char *error_msg, container_config_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    json_t *param = NULL;
    json_t *contPara_obj = NULL;
    json_t *contParas_obj = NULL;
    json_t *cfgCpuobj = NULL;
    json_t *cfgMemobj = NULL;
    json_t *cfgDiskobj = NULL;
    json_t *mountobj = NULL;
    json_t *devobj = NULL;
    param = json_object();
    contParas_obj = json_array();
    if (contParas_obj != NULL) {
        for (i = 0; i < statusobj->contPara_len; i++) {
            contPara_obj = json_object();
            cfgCpuobj = json_object();
            cfgMemobj = json_object();
            cfgDiskobj = json_object();
            mountobj = json_object();
            devobj = json_object();
            if (contPara_obj != NULL) {
                json_object_set_new(contPara_obj, "container", json_string(statusobj->contPara[i].container));
                if (strlen(statusobj->contPara[i].port) != 0) {
                    json_object_set_new(contPara_obj, "port", json_string(statusobj->contPara[i].port));
                }
                if (cfgCpuobj != NULL) {
                    json_object_set_new(cfgCpuobj, "cpus", json_integer(statusobj->contPara[i].cfgCpu.cpus));
                    json_object_set_new(cfgCpuobj, "cpuLmt", json_integer(statusobj->contPara[i].cfgCpu.cpuLmt));
                }
                json_object_set_new(contPara_obj, "cfgCpu", cfgCpuobj);
                if (cfgMemobj != NULL) {
                    json_object_set_new(cfgMemobj, "memory", json_integer(statusobj->contPara[i].cfgMem.memory));
                    json_object_set_new(cfgMemobj, "memLmt", json_integer(statusobj->contPara[i].cfgMem.memLmt));
                }
                json_object_set_new(contPara_obj, "cfgMem", cfgMemobj);

                if (cfgDiskobj != NULL) {
                    json_object_set_new(cfgDiskobj, "disk", json_integer(statusobj->contPara[i].cfgDisk.disk));
                    json_object_set_new(cfgDiskobj, "diskLmt", json_integer(statusobj->contPara[i].cfgDisk.diskLmt));
                }
                json_object_set_new(contPara_obj, "cfgDisk", cfgDiskobj);
                sg_setmount(mountobj, statusobj->contPara[i]);
                json_object_set_new(contPara_obj, "mount", mountobj);
                sg_setdev(devobj, statusobj->contPara[i]);
                json_object_set_new(contPara_obj, "dev", devobj);
            }
            json_array_append_new(contParas_obj, contPara_obj);
        }
    }
    json_object_set_new(param, "contPara", contParas_obj);
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CON_GET_CONFIG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (mssm != NULL) {
        free(mssm);
    }
    if (param != NULL) {
        json_decref(param);
    }
}

//容器状态查询应答  容器状态上报
int sg_pack_container_status_get_reply(char * type, uint16_t code, int32_t mid,
    const char *error_msg, container_status_reply_s *statusobj, char *msg)
{
    int num = 0;
    int ret = VOS_OK;
    char *mssm = NULL;
    json_t *param = NULL;
    json_t *params = NULL;
    if (statusobj == NULL) {
        printf("statusobj == NULL!\n");
        ret = VOS_ERR;
    } else {
        params = json_array();
        for (num = 0; num < statusobj->container_len; num++) {
            param = json_object();
            json_object_set_new(param, "container", json_string(statusobj[num].container));
            json_object_set_new(param, "version", json_string(statusobj[num].version));
            json_object_set_new(param, "state", json_string(statusobj[num].state));
            json_object_set_new(param, "cpuRate", json_integer(statusobj[num].cpuRate));
            json_object_set_new(param, "memUsed", json_integer(statusobj[num].memUsed));
            json_object_set_new(param, "diskUsed", json_integer(statusobj[num].diskUsed));
            json_object_set_new(param, "ip", json_string(statusobj[num].ip));
            json_object_set_new(param, "created", json_string(statusobj[num].created));
            json_object_set_new(param, "started", json_string(statusobj[num].started));
            json_object_set_new(param, "lifeTime", json_integer(statusobj[num].lifeTime));
            json_object_set_new(param, "image", json_string(statusobj[num].image));
            json_array_append_new(params, param);
        }
    }
    mssm = sg_pack_json_msg_header(code, mid, type, error_msg, params);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
    return ret;
}

//容器事件上报
void sg_pack_container_event_pack(uint16_t code, int32_t mid, const char *error_msg, container_event_report_s *statusobj, char *msg)
{
    json_t *param = NULL;
    param = json_object();
    json_object_set_new(param, "container", json_string(statusobj->container));
    json_object_set_new(param, "event", json_string(statusobj->event));
    if (strlen(statusobj->msg) != 0) {
        json_object_set_new(param, "msg", json_string(statusobj->msg));
    }
    char *mssm = sg_pack_json_msg_header(code, mid, EVENT_CON_ALARM, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//容器升级命令
int sg_unpack_container_upgrade_cmd(json_t *obj, container_upgrade_cmd_s *cmdobj)
{
    json_t* file_info = NULL;
    if (json_into_int32_t(&cmdobj->jobId, obj, "jobId") != VOS_OK) {
        return VOS_ERR;
    }
    json_into_uint32_t(&cmdobj->policy, obj, "policy");
    if (json_into_string(cmdobj->version, obj, "version") != VOS_OK) {
        return VOS_ERR;
    }
    file_info = json_object_get(obj, "file");         //再解析file对象中的数据
    if (sg_getfile(file_info, &cmdobj->file) != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
///容器升级命令应答 
void sg_pack_container_upgrade_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CON_UPGRADE, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//容器日志召回命令
int sg_unpack_container_log_get_cmd(json_t *obj, container_log_recall_cmd_s *cmdobj)
{
    json_into_string(cmdobj->container, obj, "container");
    if (json_into_string(cmdobj->url, obj, "url") != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
//容器日志召回应答
void sg_pack_container_log_get_reply(uint16_t code, int32_t mid, const char *error_msg, file_info_s *file, char *msg)
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
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CON_LOG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}



