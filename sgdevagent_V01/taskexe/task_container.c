#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "thread_business_Interact.h"
#include "vm_public.h"
#include "vm_rpc_api.h"
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
#include "mqtt_pub.h"
#include "mqtt_container.h"
#include "mqtt_dev.h"
#include "sgdev_queue.h"
#include "sgdev_param.h"
#include "sgdev_curl.h"
#include "sgdev_struct.h"
#include "task_link.h"
#include "task_container.h"
#include "timer_pack.h"

int sg_container_install(container_install_cmd_s *cmdobj, char *errmsg)
{
    int ret = VOS_OK;
    INSTALL_OVA_PARA_NORTH_SG_S install_info = {0};
    dev_status_reply_s status = { 0 };
    status.jobId = cmdobj->jobId;
    char left_obj[DATA64_LEN] = {0};
    char right_obj[DATA64_LEN] = { 0 };
    int i = 0;
    int cpus = cmdobj->cfgCpu.cpus;
    memcpy_s(install_info.container_name, DATA64_LEN, cmdobj->container, strlen(cmdobj->container) + 1);  //容器名称赋值	 
    memcpy_s(install_info.ova_name, DATA64_LEN, cmdobj->image.name, strlen(cmdobj->image.name) + 1);  //镜像名称赋值
    install_info.disk_size = cmdobj->cfgDisk.disk;
    install_info.disk_threshold = cmdobj->cfgDisk.diskLmt;
    install_info.mem_size = cmdobj->cfgMem.memory;
    install_info.mem_threshold = cmdobj->cfgMem.memLmt;
    install_info.cpu_threshold = cmdobj->cfgCpu.cpuLmt;
    install_info.cpu_mask = 0;
    for (i = 0; i < cpus; i++) {
        install_info.cpu_mask |= i;
    }
    if (strlen(cmdobj->port) != 0) {
        install_info.port_map.add_type = PORT_ADD_OVERWRITE;
        install_info.port_map.num = 1;
        if (sg_str_colon(cmdobj->port, strlen(cmdobj->port), left_obj, right_obj) != VOS_OK) {
            sprintf_s(errmsg, DATA128_LEN, "port is not correct");
            return VOS_ERR;
        }
        install_info.port_map.port_node[0].host_port = atoi(left_obj);
        install_info.port_map.port_node[0].container_port = atoi(right_obj);
    }
    if (cmdobj->dev_len > 0) {
        install_info.map_dev.dev_num = cmdobj->dev_len;
        install_info.map_dev.dev_list = (install_dev_node*)VOS_Malloc(MID_SGDEV, sizeof(install_dev_node) * cmdobj->dev_len);            //记得释放
        if (install_info.map_dev.dev_list != NULL) {
            for (i = 0; i < cmdobj->dev_len; i++)
            {
                if (sg_str_colon(cmdobj->dev[i], strlen(cmdobj->port), left_obj, right_obj) != VOS_OK) {
                    sprintf_s(errmsg, DATA128_LEN, "dev map is not correct");
                    return VOS_ERR;
                }
                sprintf_s(install_info.dev_map.dev_list[i].host_dev, 
                    CONTAINER_DEV_NAME_MAX_LEN, "%s", left_obj);
                sprintf_s(install_info.dev_map.dev_list[i].map_dev,
                    CONTAINER_DEV_NAME_MAX_LEN, "%s", right_obj);
            }
        }   
    }
    for (i = 0; i < cmdobj->mount_len; i++)
    {
        install_info.mount_dir_add.num = cmdobj->mount_len;
        install_info.mount_dir_add.add_type = MOUNT_DIR_ADD_OVERWRITE;
        install_info.mount_dir_add.mount_dir = (mount_dir_cfg_s*)VOS_Malloc(MID_SGDEV, sizeof(mount_dir_cfg_s) * cmdobj->mount_len);            //记得释放
        if (install_info.mount_dir_add.mount_dir != NULL) {
            for (i = 0; i < cmdobj->mount_len; i++)
            {
                if (sg_str_colon(cmdobj->mount[i], strlen(cmdobj->port), left_obj, right_obj) != VOS_OK) {
                    sprintf_s(errmsg, DATA128_LEN, "dev map is not correct");
                    return VOS_ERR;
                }
                sprintf_s(install_info.mount_dir_add.mount_dir[i].host_dir,
                    VM_PATH_MAX_128, "%s", left_obj);
                sprintf_s(install_info.mount_dir_add.mount_dir[i].contaienr_dir,
                    VM_PATH_MAX_128, "%s", right_obj);
            }
        }
    }

    if (strlen(cmdobj->image.sign.name) != 0) {
        install_info.verify = true;
    }

    if (!vm_rpc_container_install_north_sg(&install_info, errmsg, VM_RET_STRING_SIZE)) {
        ret = VOS_ERR;
    }

    VOS_Free(install_info.map_dev.dev_list);
    VOS_Free(install_info.mount_dir_add.mount_dir);

    return ret;



}