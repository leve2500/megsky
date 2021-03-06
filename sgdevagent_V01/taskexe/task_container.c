#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"
#include "ssp_mid.h"

#include "vm_rpc_api.h"
#include "sysman_rpc_api.h"
#include "app_management_service_api.h"

#include <glib-object.h>
#include <thrift/c_glib/thrift.h>

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
#include "thread_interact.h"

int sg_container_install(container_install_cmd_s *cmd_obj, char *errmsg)
{
    int ret = VOS_OK;
    INSTALL_OVA_PARA_NORTH_SG_S install_info = {0};
    dev_status_reply_s status = { 0 };
    status.jobId = cmd_obj->jobId;
    char left_obj[DATA_BUF_F64_SIZE] = {0};
    char right_obj[DATA_BUF_F64_SIZE] = { 0 };
    int i = 0;   
    install_info.container_name = (char *)VOS_Malloc(MID_SGDEV, strlen(cmd_obj->container) + 1);
    install_info.ova_name = (char *)VOS_Malloc(MID_SGDEV, strlen(cmd_obj->image.name) + 1);

    if (install_info.container_name == NULL) {
        return VOS_ERR;
    }

    if (install_info.ova_name == NULL) {
        return VOS_ERR;
    }

    int cpus = cmd_obj->cfgCpu.cpus;
    memcpy_s(install_info.container_name, DATA_BUF_F64_SIZE, cmd_obj->container, strlen(cmd_obj->container) + 1);  //容器名称赋值	 
    memcpy_s(install_info.ova_name, DATA_BUF_F64_SIZE, cmd_obj->image.name, strlen(cmd_obj->image.name) + 1);  //镜像名称赋值
    install_info.disk_size = cmd_obj->cfgDisk.disk;
    install_info.disk_threshold = cmd_obj->cfgDisk.diskLmt;
    install_info.mem_size = cmd_obj->cfgMem.memory;
    install_info.mem_threshold = cmd_obj->cfgMem.memLmt;
    install_info.cpu_threshold = cmd_obj->cfgCpu.cpuLmt;
    install_info.cpu_mask = 0;
    for (i = 0; i < cpus; i++) {
        install_info.cpu_mask |= i;
    }
    if (strlen(cmd_obj->port) != 0) {
        install_info.port_map.add_type = PORT_ADD_OVERWRITE;
        install_info.port_map.num = 1;
        if (sg_str_colon(cmd_obj->port, strlen(cmd_obj->port), left_obj, right_obj) != VOS_OK) {
            sprintf_s(errmsg, DATA_BUF_F128_SIZE, "port is not correct");
            return VOS_ERR;
        }
        install_info.port_map.port_node[0].host_port = atoi(left_obj);
        install_info.port_map.port_node[0].container_port = atoi(right_obj);
    }
    if (cmd_obj->dev_len > 0) {
        install_info.dev_map.dev_num = cmd_obj->dev_len;
        install_info.dev_map.dev_list = (install_dev_node*)VOS_Malloc(MID_SGDEV, sizeof(install_dev_node) * cmd_obj->dev_len);            //记得释放
        if (install_info.dev_map.dev_list != NULL) {
            for (i = 0; i < cmd_obj->dev_len; i++)   
            {
                if (sg_str_colon(cmd_obj->dev[i], strlen(cmd_obj->port), left_obj, right_obj) != VOS_OK) {
                    sprintf_s(errmsg, DATA_BUF_F128_SIZE, "dev map is not correct");
                    return VOS_ERR;
                }
                sprintf_s(install_info.dev_map.dev_list[i].host_dev, 
                    CONTAINER_DEV_NAME_MAX_LEN, "%s", left_obj);
                sprintf_s(install_info.dev_map.dev_list[i].map_dev,
                    CONTAINER_DEV_NAME_MAX_LEN, "%s", right_obj);
            }
        }   
    }
    for (i = 0; i < cmd_obj->mount_len; i++)
    {
        install_info.mount_dir_add.num = cmd_obj->mount_len;
        install_info.mount_dir_add.add_type = MOUNT_DIR_ADD_OVERWRITE;
        install_info.mount_dir_add.mount_dir = (mount_dir_cfg_s*)VOS_Malloc(MID_SGDEV, sizeof(mount_dir_cfg_s) * cmd_obj->mount_len);            //记得释放
        if (install_info.mount_dir_add.mount_dir != NULL) {
            for (i = 0; i < cmd_obj->mount_len; i++) //不能再用i
            {
                if (sg_str_colon(cmd_obj->mount[i], strlen(cmd_obj->port), left_obj, right_obj) != VOS_OK) {
                    sprintf_s(errmsg, DATA_BUF_F128_SIZE, "dev map is not correct");
                    return VOS_ERR;
                }
                sprintf_s(install_info.mount_dir_add.mount_dir[i].host_dir,
                    VM_PATH_MAX_128, "%s", left_obj);
                sprintf_s(install_info.mount_dir_add.mount_dir[i].container_dir,
                    VM_PATH_MAX_128, "%s", right_obj);
            }
        }
    }

    if (strlen(cmd_obj->image.sign.name) != 0) {
        install_info.verify = true;
    }

    if (!vm_rpc_container_install_north_sg(&install_info, errmsg, VM_RET_STRING_SIZE)) {
        ret = VOS_ERR;
    }

    VOS_Free(install_info.dev_map.dev_list);
    VOS_Free(install_info.mount_dir_add.mount_dir);

    return ret;



}

int sg_container_update(container_install_cmd_s *cmd_obj, char *errmsg)
{

}