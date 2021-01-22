#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "thread_business_Interact.h"
#include "vm_public.h"
#include "vos_typdef.h"
#include "sysman_rpc_api.h"
#include "app_management_service_api.h"

#include <glib-object.h>
#include <thrift/c_glib/thrift.h>
#include <thrift/c_glib/processor/thrift_multiservice_processor.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol.h>
#include <thrift/c_glib/protocol/thrift_router_protocol.h>  
#include <thrift/c_glib/server/thrift_server.h>
#include <thrift/c_glib/server/thrift_router_server.h>
#include <thrift/c_glib/transport/thrift_router_transport.h>
#include <thrift/c_glib/transport/thrift_kmsg.h>
#include <thrift_service_def.h>

#include "mqtt_json.h"
#include "mqtt_app.h"
#include "mqtt_dev.h"
#include "mqtt_pub.h"
#include "sgdev_queue.h"
#include "sgdev_param.h"
#include "sgdev_struct.h"
#include "sgdev_curl.h"
#include "task_link.h"
#include "task_app.h"
#include "timer_pack.h"


int sg_app_install(app_install_cmd_s cmdobj, char *errmsg)
{
    int cpus = 0;
    int i = 0;
    int ret = VOS_OK;
    dev_status_reply_s status = { 0 };
    APPM_OPERATION_PARA para = { 0 };
    appm_error_message err_msg;

    status.jobId = cmdobj.jobId;
    status.progress = 80;
    status.state = STATUS_EXE_INSTALL;
    set_app_install_status(status);
    memcpy_s(para.lxc_name, DATA64_LEN, cmdobj.container, strlen(cmdobj.container) + 1);  //容器名称赋值	 
    memcpy_s(para.app_name, DATA64_LEN, cmdobj.app, strlen(cmdobj.app) + 1);              //app名称	
    sprintf_s(para.app_file, DATA128_LEN, "%s/%s", DEFAULT_FILE_PATH, cmdobj.file.name);
    if (ssp_calculate_sha256_of_file(para.app_file, para.app_hash, APP_MANAGEMENT_APP_HASH_MAX_LEN + 1)) {
        (void)printf("permission denied.\r\n");
        sprintf_s(errmsg, APP_MANAGEMENT_ERRMSG_MAX_LEN, "permission denied");
        return VOS_ERR;
    }
    // memcpy_s(para.app_file, DATA128_LEN, cmdobj.file.name ,strlen(cmdobj.file.name)+1);  //app文件	 
    cpus = cmdobj.cfgCpu.cpus;
    para.cpu_mask = 0;
    for (i = 0; i < cpus; i++) {
        para.cpu_mask |= i;
    }
    para.cpu_threshold = cmdobj.cfgCpu.cpuLmt;
    para.memory_limit = cmdobj.cfgMem.memory;
    para.memory_threshold = cmdobj.cfgMem.memLmt;
    if (cmdobj.file.sign.size != 0 || strlen(cmdobj.file.sign.name) != 0) { //需要签名
        para.verify = 1;
    }
    printf("sgdevagent**** : sg_app_install para.verify = %d.\n", para.verify);
    if (appm_rpc_transport_open() != VOS_OK) {
        printf("sgdevagent**** : appm_rpc_transport_open failed.\n");
    }
    if (app_management_action_call_app_install(&para, &err_msg) != VOS_OK) {
        sprintf_s(errmsg, APP_MANAGEMENT_ERRMSG_MAX_LEN, "app_install:%s", err_msg.errMsg);
        printf("sgdevagent**** :app_install failed: = %s.\n", err_msg.errMsg);
        appm_rpc_transport_close();
        return VOS_ERR;
    }
    if (strncmp(cmdobj.enable, "1", strlen(cmdobj.enable)) == 0) {
        if (app_management_action_call_app_enable(&para, &err_msg) != VOS_OK) {
            sprintf_s(errmsg, APP_MANAGEMENT_ERRMSG_MAX_LEN, "enable:%s", err_msg.errMsg);
            ret = VOS_ERR;
        }
    } else {
        if (app_management_action_call_app_disable(&para, &err_msg) != VOS_OK)
            sprintf_s(errmsg, APP_MANAGEMENT_ERRMSG_MAX_LEN, "disable:%s", err_msg.errMsg);
        ret = VOS_ERR;
    }
    appm_rpc_transport_close();
    printf("sgdevagent**** : appm_rpc_transport_close.\n");
    return ret;
}

int sg_container_with_app_install(container_install_cmd_s *cmdobj, char *errmsg)
{
    int cpus = 0;
    int i = 0;
    int ret = VOS_OK;
    dev_status_reply_s status = { 0 };
    APPM_OPERATION_PARA para = { 0 };
    appm_error_message err_msg;

    status.jobId = cmdobj->jobId;
    status.progress = 80;
    status.state = STATUS_EXE_INSTALL;

    set_app_install_status(status);
    memcpy_s(para.lxc_name, DATA64_LEN, cmdobj->container, strlen(cmdobj->container) + 1);  //容器名称赋值	 
    memcpy_s(para.app_name, DATA64_LEN, cmdobj->withAPP.file.name, strlen(cmdobj->withAPP.file.name) + 1);   //app名称	未知？
    sprintf_s(para.app_file, DATA128_LEN, "%s/%s", DEFAULT_FILE_PATH, cmdobj->withAPP.file.name);
    // memcpy_s(para.app_file, DATA128_LEN, cmdobj.file.name ,strlen(cmdobj.file.name)+1);  //app文件	 
    cpus = cmdobj->withAPP.cfgCpu.cpus;
    para.cpu_mask = 0;
    for (i = 0; i < cpus; i++) {
        para.cpu_mask |= i;
    }
    para.cpu_threshold = cmdobj->withAPP.cfgCpu.cpuLmt;
    para.memory_limit = cmdobj->withAPP.cfgMem.memory;
    para.memory_threshold = cmdobj->withAPP.cfgMem.memLmt;
    if (cmdobj->withAPP.file.sign.size != 0 || strlen(cmdobj->withAPP.file.sign.name) != 0) { //需要签名
        para.verify = 1;
    }
    if (ssp_calculate_sha256_of_file(para.app_file, para.app_hash, APP_MANAGEMENT_APP_HASH_MAX_LEN + 1)) {
        (void)printf("permission denied.\r\n");
        sprintf_s(errmsg, APP_MANAGEMENT_ERRMSG_MAX_LEN, "permission denied");
        return VOS_ERR;
    }
    if (appm_rpc_transport_open() != VOS_OK) {
        printf("sgdevagent**** : appm_rpc_transport_open failed.\n");
    }
    if (app_management_action_call_app_install(&para, &err_msg) != VOS_OK) {
        sprintf_s(errmsg, APP_MANAGEMENT_ERRMSG_MAX_LEN, "app_install:%s", err_msg.errMsg);
        printf("sgdevagent**** :app_install failed: = %s.\n", err_msg.errMsg);
        appm_rpc_transport_close();
        return VOS_ERR;
    }
    if (strncmp(cmdobj->withAPP.enable, "1", strlen(cmdobj->withAPP.enable)) == 0) {
        if (app_management_action_call_app_enable(&para, &err_msg) != VOS_OK) {
            sprintf_s(errmsg, APP_MANAGEMENT_ERRMSG_MAX_LEN, "enable:%s", err_msg.errMsg);
            ret = VOS_ERR;
        }
    } else {
        if (app_management_action_call_app_disable(&para, &err_msg) != VOS_OK)
            sprintf_s(errmsg, APP_MANAGEMENT_ERRMSG_MAX_LEN, "disable:%s", err_msg.errMsg);
        ret = VOS_ERR;
    }
    appm_rpc_transport_close();
    printf("sgdevagent**** : appm_rpc_transport_close.\n");
    return ret;
}