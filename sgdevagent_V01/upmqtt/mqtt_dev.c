#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "vos_typdef.h"
#include "sgdev_struct.h"
#include "sgdev_param.h"
#include "mqtt_json.h"
#include "mqtt_dev.h"




//A.1  设备信息字段：dev组帧定义  
static int json_dev_set_new(json_t *obj, dev_info_s *dev_str)
{
    if (NULL != obj) {
        json_object_set_new(obj, "devType", json_string(dev_str->devType));
        json_object_set_new(obj, "devName", json_string(dev_str->devName));
        printf("\ndevName = %s\n", dev_str->devName);

        json_object_set_new(obj, "mfgInfo", json_string(dev_str->mfgInfo));
        json_object_set_new(obj, "devStatus", json_string(dev_str->devStatus));
        json_object_set_new(obj, "hardVersion", json_string(dev_str->hardVersion));
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}
//A.2 CPU 信息字段:cpu组帧定义  
static int json_cpu_set_new(json_t *obj, cpu_info_s *cpu_str)
{
    if (NULL != obj) {
        json_object_set_new(obj, "cpus", json_integer(cpu_str->cpus));
        json_object_set_new(obj, "frequency", json_real(cpu_str->frequency));
        if (cpu_str->cache != 0) {
            json_object_set_new(obj, "cache", json_integer(cpu_str->cache));
        }
        if (strlen(cpu_str->arch) != 0) {
            json_object_set_new(obj, "arch", json_string(cpu_str->arch));
        }
        if (cpu_str->cpuLmt != 0) {
            json_object_set_new(obj, "cpuLmt", json_integer(cpu_str->cpuLmt));
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}

//A.3内存信息字段:mem组帧定义  
static int json_mem_set_new(json_t *obj, mem_info_s *mem_str)
{
    if (NULL != obj) {
        json_object_set_new(obj, "phy", json_integer(mem_str->phy));
        if (mem_str->virt != 0) {
            json_object_set_new(obj, "virt", json_integer(mem_str->virt));
        }
        if (mem_str->memLmt != 0) {
            json_object_set_new(obj, "memLmt", json_integer(mem_str->memLmt));
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}
//A.4  硬盘信息字段:disk组帧定义  
static int json_disk_set_new(json_t *obj, disk_info_s *disk_str)
{
    if (NULL != obj) {
        json_object_set_new(obj, "disk", json_integer(disk_str->disk));
        if (disk_str->diskLmt != 0) {
            json_object_set_new(obj, "diskLmt", json_integer(disk_str->diskLmt));
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}
//A.5  外接设备信息字段:links组帧定义  
static int json_links_set_new(json_t *obj, link_info_s *link_str)
{
    if (obj != NULL) {
        if (strlen(link_str->id) != 0) {
            json_object_set_new(obj, "id", json_string(link_str->id));
        }
        if (strlen(link_str->mac) != 0) {
            json_object_set_new(obj, "mac", json_string(link_str->mac));
        }
        json_object_set_new(obj, "name", json_string(link_str->name));
        json_object_set_new(obj, "type", json_string(link_str->type));
    } else {
        return VOS_ERR;
    }
    if (link_str == NULL) {
        printf("json_links_set_new: link_str is NULL. \n");
        return VOS_ERR;
    }
    if (obj == NULL) {
        printf("json_links_set_new: obj is NULL. \n");
        return VOS_ERR;
    }
    json_object_set_new(obj, "id", json_string(link_str->id));
    json_object_set_new(obj, "mac", json_string(link_str->mac));
    json_object_set_new(obj, "name", json_string(link_str->name));
    json_object_set_new(obj, "type", json_string(link_str->type));
    return VOS_OK;
}
//A.6  操作系统信息字段:os组帧定义  
static int json_os_set_new(json_t *obj, os_info_s *os_str)
{
    if (NULL != obj) {
        json_object_set_new(obj, "distro", json_string(os_str->distro));
        json_object_set_new(obj, "version", json_string(os_str->version));
        json_object_set_new(obj, "kernel", json_string(os_str->kernel));
        json_object_set_new(obj, "softVersion", json_string(os_str->softVersion));
        json_object_set_new(obj, "patchVersion", json_string(os_str->patchVersion));
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}

//temp:温度监控组帧定义  
static int json_temp_set_new(json_t *obj, temp_info_s *temp_str)
{
    if (NULL != obj) {
        json_object_set_new(obj, "temLow", json_integer(temp_str->temLow));
        json_object_set_new(obj, "temHigh", json_integer(temp_str->temHigh));
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}
//状态主动上报的上报时间间隔组帧定义  
static int json_rep_set_new(json_t *obj, rep_period_s *rep_str)
{
    if (NULL != obj) {
        if (rep_str->devPeriod != 0) {
            json_object_set_new(obj, "devPeriod", json_integer(rep_str->devPeriod));
        }
        if (rep_str->conPeriod != 0) {
            json_object_set_new(obj, "conPeriod", json_integer(rep_str->conPeriod));
        }
        if (rep_str->appPeriod != 0) {
            json_object_set_new(obj, "appPeriod", json_integer(rep_str->appPeriod));
        }
        if (rep_str->heartPeriod != 0) {
            json_object_set_new(obj, "heartPeriod", json_integer(rep_str->heartPeriod));
        }
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}

static void sg_pack_dev(json_t *obj, dev_acc_req_s *devaccobj)
{
    json_t *dev_object = NULL;
    json_t *cpu_object = NULL;
    json_t *mem_object = NULL;
    json_t *disk_object = NULL;
    json_t *os_object = NULL;
    dev_object = json_object();
    if (!json_dev_set_new(dev_object, &devaccobj->dev)) {
        json_object_set_new(obj, "dev", dev_object);
    }
    cpu_object = json_object();
    if (!json_cpu_set_new(cpu_object, &devaccobj->cpu)) {
        json_object_set_new(obj, "cpu", cpu_object);
    }
    mem_object = json_object();
    if (!json_mem_set_new(mem_object, &devaccobj->mem)) {
        json_object_set_new(obj, "mem", mem_object);
    }
    disk_object = json_object();
    if (!json_disk_set_new(disk_object, &devaccobj->disk)) {
        json_object_set_new(obj, "disk", disk_object);
    }
    os_object = json_object();
    if (!json_os_set_new(os_object, &devaccobj->os)) {
        json_object_set_new(obj, "os", os_object);
    }
}
/*****************************************************************************
函 数 名  : sg_pack_dev_linkup_data
功能描述  : 设备接入请求
输入参数  : linkupobj
输出参数  : *msg
返 回 值  :
调用函数  :
被调函数  :
*****************************************************************************/
void sg_pack_dev_linkup_data(dev_acc_req_s *linkupobj, char *msg)
{
    printf("megsky--test :sg_pack_dev_linkup_data \n");
    uint16_t i = 0;
    uint16_t code = CODE_NULL;
    int32_t mid = 0;	//0代表不传mid
    json_t *param = NULL;
    json_t *linkobj = NULL;
    json_t *linksobj = NULL;

    srand((unsigned)time(NULL));
    mid = rand();
    param = json_object();
    linksobj = json_array();
    if (NULL != linksobj) {
        for (i = 0; i < linkupobj->link_len; i++) {
            linkobj = json_object();
            if (!json_links_set_new(linkobj, &linkupobj->links[i])) {
                json_array_append_new(linksobj, linkobj);
            }
        }
    }
    sg_pack_dev(param, linkupobj);
    json_object_set_new(param, "links", linksobj);
    char *mssm = sg_pack_json_msg_header(code, mid, EVENT_LINKUP, NULL, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//设备接入应答  （未用）
// void sg_pack_dev_linkup_reply(const char *error_msg, char *msg)
// {
//     uint16_t code = CODE_NULL;
//     int32_t mid = 0;	//0代表不传mid
//     json_t *param = NULL;
//     param = json_object();
//     char *mssm = sg_pack_json_msg_header(code, mid, EVENT_LINKUP, error_msg, param);
//     memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) +1); 
//     if (NULL != mssm) {
//         free(mssm);
//     }
// }

/*****************************************************************************
 函 数 名  : sg_pack_dev_linkdown_data
 功能描述  : 设备主动断开上报
 输入参数  : linkdownobj
 输出参数  : *msg
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
void sg_pack_dev_linkdown_data(char *linkdownobj, const char *error_msg, char *msg)
{
    uint16_t code = CODE_NULL;
    int32_t mid = 0;	//0代表不传mid
    json_t *reasonobj;
    reasonobj = json_object();
    json_object_set_new(reasonobj, "reason", json_string(linkdownobj));
    char *mssm = sg_pack_json_msg_header(code, mid, EVENT_LINKDOWN, error_msg, reasonobj);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
}

//设备心跳请求
int sg_pack_dev_heartbeat_request_data(char *msg)
{
    int ret = VOS_OK;
    time_t now_time;
    now_time = time(NULL);
    json_t *heartbeatobj = NULL;

    heart_request_s heart_request = { 0 };
    sprintf_s(heart_request.type, DATA_BUF_F32_SIZE, "%s", EVENT_HEARTBEAT);  //字符串赋值
    // sprintf(heart_request.type,  "%s", EVENT_HEARTBEAT);  //字符串赋值
    heartbeatobj = json_object();
    //获取参数
    sg_dev_param_info_s idparam = sg_get_param();
    json_object_set_new(heartbeatobj, "deviceId", json_string(idparam.devid));
    //timestamp
    ret = sg_mqtttimestr(now_time, heart_request.timestamp, sizeof(heart_request.timestamp), TIME_UTC);    //使用UTC时间
    if (ret != VOS_OK) {
        printf("\nFill json header failed for generate time str.\n");
        return VOS_ERR;
    }
    json_object_set_new(heartbeatobj, "timestamp", json_string(heart_request.timestamp));
    json_object_set_new(heartbeatobj, "type", json_string(heart_request.type));
    char *mssm = json_dumps(heartbeatobj, JSON_PRESERVE_ORDER);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    printf("msg=%s\n", msg);
    if (NULL != mssm) {
        free(mssm);
    }
    json_decref(heartbeatobj);
    return VOS_OK;
    //deviceId string 是 边设备唯一标识
    //	timestamp string 是 消息发送的时间戳，CST 时间, 精度到秒
    //	type string 是 EVENT_HEARTBEAT
    //不需要调用 pack_json_msg_header(code, mid, type, param);
}

//设备心跳应答
int sg_pack_dev_heartbeat_response_data(char *msg)
{
    int ret = VOS_OK;
    time_t now_time;
    now_time = time(NULL);
    json_t *heartbeatobj = NULL;
    heart_reply_s heart_reply = { 0 };
    heart_reply.code = REQUEST_SUCCESS;
    sprintf_s(heart_reply.type, DATA_BUF_F32_SIZE, "%s", EVENT_HEARTBEAT);  //字符串赋值
    // sprintf(heart_reply.type,  "%s", EVENT_HEARTBEAT);  //字符串赋值
    heartbeatobj = json_object();
    //获取参数
    sg_dev_param_info_s idparam = sg_get_param();
    json_object_set_new(heartbeatobj, "deviceId", json_string(idparam.devid));
    //timestamp
    ret = sg_mqtttimestr(now_time, heart_reply.timestamp, sizeof(heart_reply.timestamp), 1);    //使用UTC时间
    if (ret != VOS_OK)
    {
        printf("Fill json header failed for generate time str.\n");
        return VOS_ERR;
    }
    json_object_set_new(heartbeatobj, "timestamp", json_string(heart_reply.timestamp));
    json_object_set_new(heartbeatobj, "type", json_string(heart_reply.type));
    json_object_set_new(heartbeatobj, "code", json_integer(heart_reply.code));
    char *mssm = json_dumps(heartbeatobj, JSON_PRESERVE_ORDER);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    printf("msg=%s\n", msg);
    if (NULL != mssm) {
        free(mssm);
    }
    json_decref(heartbeatobj);
    return VOS_OK;
}

/*****************************************************************************
函 数 名  : sg_unpack_dev_install_cmd
功能描述  : 设备升级命令
输入参数  : obj
输出参数  : *cmdobj
返 回 值  :
调用函数  :
被调函数  :
*****************************************************************************/
int sg_unpack_dev_install_cmd(json_t *obj, device_upgrade_s *cmdobj)
{
    json_t* file_info = NULL;
    //************************开始解析数据*****************************************
    if (json_into_int32_t(&cmdobj->jobId, obj, "jobId") != VOS_OK) {          //必选为是
        return VOS_ERR;
    }
    json_into_uint32_t(&cmdobj->policy, obj, "policy");                //必选为否
    if (json_into_string(cmdobj->version, obj, "version") != VOS_OK) {                      //必选为是
        return VOS_ERR;
    }
    if (json_into_uint8_t(&cmdobj->upgradeType, obj, "upgradeType") != VOS_OK) {
        return VOS_ERR;
    }
    file_info = json_object_get(obj, "file");        //再解析file对象中的数据
    if (sg_getfile(file_info, &cmdobj->file) != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}

//设备升级命令应答  无param
void sg_pack_dev_install_cmd(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;  //param未赋值表示 param为空 不组帧
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_SYS_UPGRADE, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }

}
//设备升级状态查询命令   容器应用也一样
int32_t sg_unpack_dev_install_query(json_t *obj)
{
    int32_t jobid = 0;
    json_into_int32_t(&jobid, obj, "jobId");
    return jobid;
}

/*****************************************************************************
函 数 名  : sg_pack_dev_install_query
功能描述  : 设备升级状态查询应答 (容器应用也一样)
输入参数  : code mid *statusobj
输出参数  : *msg
返 回 值  :
调用函数  :
被调函数  :
*****************************************************************************/
int sg_pack_dev_install_query(uint16_t code, int32_t mid, const char *error_msg, dev_status_reply_s statusobj, char *msg)
{
    json_t *param = NULL;
    param = json_object();
    json_object_set_new(param, "jobId", json_integer(statusobj.jobId));
    json_object_set_new(param, "progress", json_integer(statusobj.progress));
    json_object_set_new(param, "state", json_integer(statusobj.state));
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_STATUS_QUERY, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
    return VOS_OK;
}
//设备升级结果上报 容器和应用也一样
void sg_pack_dev_install_result(dev_upgrede_res_reply_s statusobj, const char *error_msg, char *msg)
{
    uint16_t code = CODE_NULL;
    int32_t mid = 0;	//0代表不传mid
    json_t *param = NULL;
    param = json_object();
    json_object_set_new(param, "jobId", json_integer(statusobj.jobId));
    json_object_set_new(param, "code", json_integer(statusobj.code));
    if (strlen(statusobj.msg) != 0) {
        json_object_set_new(param, "msg", json_string(statusobj.msg));
    }
    char *mssm = sg_pack_json_msg_header(code, mid, REP_JOB_RESULT, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

static int jsonframmem(json_t *obj, dev_sta_reply_s *devstaobj)
{
    int ret = VOS_OK;
    json_object_set_new(obj, "phy", json_integer(devstaobj->mem_used.phy));
    json_object_set_new(obj, "virt", json_integer(devstaobj->mem_used.virt));
    return ret;
}

//设备状态上报
void sg_pack_dev_run_status(dev_sta_reply_s *statusobj, const char *error_msg, char *msg)
{
    uint16_t i = 0;
    uint16_t code = CODE_NULL;
    int32_t mid = 0;	//0代表不传mid
    json_t *param = NULL;
    json_t *mem_object = NULL;
    json_t *linkStateobj = NULL;
    json_t *linkStatesobj = NULL;
    param = json_object();
    mem_object = json_object();
    linkStatesobj = json_array();
    if (NULL != linkStatesobj) {
        for (i = 0; i < statusobj->link_len; i++) {
            linkStateobj = json_object();
            if (NULL != linkStateobj) {
                json_object_set_new(linkStateobj, "name", json_string(statusobj->linkState[i].name));
                json_object_set_new(linkStateobj, "status", json_string(statusobj->linkState[i].status));
                json_array_append_new(linkStatesobj, linkStateobj);
            }
        }
    }
    json_object_set_new(param, "cpuRate", json_integer(statusobj->cpuRate));
    if (jsonframmem(mem_object, statusobj) == VOS_OK) {
        json_object_set_new(param, "memUsed", mem_object);
    } else {
        printf("jsonframmem error!!!\n");
    }

    json_object_set_new(param, "diskUsed", json_integer(statusobj->diskUsed));
    json_object_set_new(param, "tempValue", json_integer(statusobj->tempValue));
    json_object_set_new(param, "devDateTime", json_string(statusobj->devDateTime));
    json_object_set_new(param, "devStDateTime", json_string(statusobj->devStDateTime));
    json_object_set_new(param, "devRunTime", json_integer(statusobj->devRunTime));
    json_object_set_new(param, "linkState", linkStatesobj);
    if (strlen(statusobj->longitude) != 0) {
        json_object_set_new(param, "longitude", json_string(statusobj->longitude));
    }
    if (strlen(statusobj->latitude) != 0) {
        json_object_set_new(param, "latitude", json_string(statusobj->latitude));
    }
    char *mssm = sg_pack_json_msg_header(code, mid, REP_SYS_STATUS, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}

//设备状态查询命令应答 
int sg_pack_dev_run_status_reply(uint16_t code, int32_t mid, const char *error_msg, dev_sta_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    char *mssm = NULL;
    json_t *param = NULL;
    json_t *mem_object = NULL;
    json_t *linkStateobj = NULL;
    json_t *linkStatesobj = NULL;
    param = json_object();
    mem_object = json_object();
    linkStatesobj = json_array();
    if (NULL != linkStatesobj) {
        for (i = 0; i < statusobj->link_len; i++) {
            linkStateobj = json_object();
            if (NULL != linkStateobj) {
                json_object_set_new(linkStateobj, "name", json_string(statusobj->linkState[i].name));
                json_object_set_new(linkStateobj, "status", json_string(statusobj->linkState[i].status));
                json_array_append_new(linkStatesobj, linkStateobj);
            }
        }
    }
    json_object_set_new(param, "cpuRate", json_integer(statusobj->cpuRate));
    if (!jsonframmem(mem_object, statusobj)) {
        json_object_set_new(param, "memUsed", mem_object);
    }
    json_object_set_new(param, "diskUsed", json_integer(statusobj->diskUsed));
    json_object_set_new(param, "tempValue", json_integer(statusobj->tempValue));
    json_object_set_new(param, "devDateTime", json_string(statusobj->devDateTime));
    json_object_set_new(param, "devStDateTime", json_string(statusobj->devStDateTime));
    json_object_set_new(param, "devRunTime", json_integer(statusobj->devRunTime));
    json_object_set_new(param, "linkState", linkStatesobj);
    if (strlen(statusobj->longitude) != 0) {
        json_object_set_new(param, "longitude", json_string(statusobj->longitude));
    }
    if (strlen(statusobj->latitude) != 0) {
        json_object_set_new(param, "latitude", json_string(statusobj->latitude));
    }
    mssm = sg_pack_json_msg_header(code, mid, CMD_SYS_STATUS, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
    return VOS_OK;
}

static void sg_pack_devinfo(json_t *obj, dev_info_inq_reply_s *devinfoobj)
{
    json_t *dev_object = NULL;
    json_t *cpu_object = NULL;
    json_t *mem_object = NULL;
    json_t *disk_object = NULL;
    json_t *temp_object = NULL;
    json_t *os_object = NULL;
    dev_object = json_object();
    if (!json_dev_set_new(dev_object, &devinfoobj->dev)) {
        json_object_set_new(obj, "dev", dev_object);
    }
    cpu_object = json_object();
    if (!json_cpu_set_new(cpu_object, &devinfoobj->cpu)) {
        json_object_set_new(obj, "cpu", cpu_object);
    }
    mem_object = json_object();
    if (!json_mem_set_new(mem_object, &devinfoobj->mem)) {
        json_object_set_new(obj, "mem", mem_object);
    }
    disk_object = json_object();
    if (!json_disk_set_new(disk_object, &devinfoobj->disk)) {
        json_object_set_new(obj, "disk", disk_object);
    }
    temp_object = json_object();
    if (!json_temp_set_new(temp_object, &devinfoobj->temperature)) {
        json_object_set_new(obj, "temperature", temp_object);
    }
    os_object = json_object();
    if (!json_os_set_new(os_object, &devinfoobj->os)) {
        json_object_set_new(obj, "os", os_object);
    }
}

//设备信息查询命令应答 
int sg_pack_dev_info_reply(uint16_t code, int32_t mid, const char *error_msg, dev_info_inq_reply_s *statusobj, char *msg)
{
    uint16_t i = 0;
    json_t *param = NULL;
    json_t *linkobj = NULL;
    json_t *linksobj = NULL;
    json_t *repPeriodobj = NULL;
    param = json_object();
    linksobj = json_array();
    if (NULL != linksobj) {
        for (i = 0; i < statusobj->link_len; i++) {
            linkobj = json_object();
            if (!json_links_set_new(linkobj, &statusobj->links[i])) {
                json_array_append_new(linksobj, linkobj);
            }
        }
    }
    sg_pack_devinfo(param, statusobj);
    json_object_set_new(param, "links", linksobj);
    repPeriodobj = json_object();
    if (!json_rep_set_new(repPeriodobj, &statusobj->rep_period)) {
        json_object_set_new(param, "repPeriod", repPeriodobj);
    }
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_INFO_QUERY, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
    return VOS_OK;
}
//设备管理参数修改命令
int sg_unpack_dev_set_para_cmd(json_t *obj, dev_man_conf_command_s *paraobj)
{
    json_t* temp_info = NULL;
    json_t* repPeriod_info = NULL;
    json_into_string(paraobj->devName, obj, "devName");       //先解析外层的数据
    json_into_uint32_t(&paraobj->cpuLmt, obj, "cpuLmt");
    json_into_uint32_t(&paraobj->memLmt, obj, "memLmt");
    json_into_uint32_t(&paraobj->diskLmt, obj, "diskLmt");
    temp_info = json_object_get(obj, "temperature");         //再解析temperature对象中的数据
    if (json_is_object(temp_info)) {
        if (json_into_int8_t(&paraobj->temperature.temLow, temp_info, "temLow") != VOS_OK) {
            return VOS_ERR;
        }
        if (json_into_int8_t(&paraobj->temperature.temHigh, temp_info, "temHigh") != VOS_OK) {
            return VOS_ERR;
        }
    }
    repPeriod_info = json_object_get(obj, "repPeriod");      //最后解析repPeriod对象中的数据
    if (json_is_object(repPeriod_info)) {
        json_into_uint32_t(&paraobj->rep_period.devPeriod, repPeriod_info, "devPeriod");
        json_into_uint32_t(&paraobj->rep_period.conPeriod, repPeriod_info, "conPeriod");
        json_into_uint32_t(&paraobj->rep_period.appPeriod, repPeriod_info, "appPeriod");
        json_into_uint32_t(&paraobj->rep_period.heartPeriod, repPeriod_info, "heartPeriod");
    }
    return VOS_OK;
}

//设备管理参数修改命令应答 无param
void sg_pack_dev_set_para_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;  //param未赋值表示 param为空 不组帧
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_SYS_SET_CONFIG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//设备时间同步命令
int sg_unpack_dev_set_time_cmd(json_t *obj, dev_time_command_s *timeobj)
{
    if (json_into_string(timeobj->dateTime, obj, "dateTime") != VOS_OK) {    //先解析外层的数据
        return VOS_ERR;
    }
    if (json_into_string(timeobj->timeZone, obj, "timeZone") != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
//设备时间同步命令应答 无param
void sg_pack_dev_set_time_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;  //param未赋值表示 param为空 不组帧
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_DATETIME_SYN, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//设备事件上报
void sg_pack_dev_event(dev_thing_reply_s *evenobj, const char *error_msg, char *msg)
{
    char *mssm = NULL;
    uint16_t code = CODE_NULL;
    int32_t mid = 0;	        //0代表不传mid
    json_t *param = NULL;
    param = json_object();
    json_object_set_new(param, "event", json_string(evenobj->event));
    if (strlen(evenobj->msg) != 0) {
        json_object_set_new(param, "msg", json_string(evenobj->msg));
    }
    mssm = sg_pack_json_msg_header(code, mid, EVENT_SYS_ALARM, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//设备日志召回
int sg_unpack_dev_log_cmd(json_t *obj, dev_log_recall_s *logobj)
{
    if (json_into_string(logobj->url, obj, "url") != VOS_OK) {    //先解析外层的数据
        return VOS_ERR;
    }
    if (json_into_uint8_t(&logobj->logType, obj, "logType") != VOS_OK) {    //先解析外层的数据
        return VOS_ERR;
    }
    return VOS_OK;
}
//设备日志召回应答
void sg_pack_dev_log_reply(uint16_t code, int32_t mid, const char *error_msg, file_info_s *logobj, char *msg)
{
    json_t *param = NULL;
    json_t *sign_object = NULL;
    param = json_object();
    sign_object = json_object();
    if (NULL != sign_object) {
        json_object_set_new(sign_object, "name", json_string(logobj->sign.name));
        if (strlen(logobj->sign.url) != 0) {
            json_object_set_new(sign_object, "url", json_string(logobj->sign.url));
        }
        if (logobj->sign.size != 0) {
            json_object_set_new(sign_object, "size", json_integer(logobj->sign.size));
        }
        if (strlen(logobj->sign.md5) != 0) {
            json_object_set_new(sign_object, "md5", json_string(logobj->sign.md5));
        }
    }
    json_object_set_new(param, "name", json_string(logobj->name));
    if (strlen(logobj->fileType) != 0) {
        json_object_set_new(param, "fileType", json_string(logobj->fileType));
    }
    if (strlen(logobj->url) != 0) {
        json_object_set_new(param, "url", json_string(logobj->url));
    }
    json_object_set_new(param, "size", json_integer(logobj->size));
    json_object_set_new(param, "md5", json_string(logobj->md5));
    json_object_set_new(param, "sign", sign_object);
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_SYS_LOG, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
//设备控制命令
int sg_unpack_dev_ctrl_cmd(json_t *obj, char *ctrlobj)
{
    if (json_into_string(ctrlobj, obj, "action") != VOS_OK) {
        return VOS_ERR;
    }
    return VOS_OK;
}
//设备控制命令应答 无param
void sg_pack_dev_ctrl_reply(uint16_t code, int32_t mid, const char *error_msg, char *msg)
{
    json_t *param = NULL;  //param未赋值表示 param为空 不组帧
    char *mssm = sg_pack_json_msg_header(code, mid, CMD_CTRL, error_msg, param);
    memcpy_s(msg, MSG_ARRVD_MAX_LEN, mssm, strlen(mssm) + 1);
    if (NULL != mssm) {
        free(mssm);
    }
    if (NULL != param) {
        json_decref(param);
    }
}
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
//解析cfgCpu对象
static int sg_getcfgcpu(json_t *obj, cfg_cpu_info_s *cfgcpuobj)
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
//解析cfgMem对象
static int sg_getcfgmem(json_t *obj, cfg_mem_info_s *cfgmemobj)
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
//解析cfgDisk对象
static int sg_getcfgdisk(json_t *obj, cfg_disk_info_s *cfgdiskobj)
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

