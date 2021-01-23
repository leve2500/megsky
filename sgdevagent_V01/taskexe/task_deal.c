#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <glib-object.h>
#include <thrift/c_glib/thrift.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"
#include "ssp_mid.h"

#include "vm_public.h"
#include "sysman_rpc_api.h"
#include "app_management_service_api.h"


#include "task_app.h"
#include "task_container.h"
#include "task_deal.h"
#include "task_link.h"

#include "timer_pack.h"
#include "sgdev_curl.h"
#include "sgdev_queue.h"
#include "sgdev_struct.h"
#include "mqtt_json.h"
#include "mqtt_pub.h"

#include "thread_interact.h"

typedef struct dev_usage_status_reply {
    cpuusage_s      cpuoutput_value;                      //CPU 负载（例如 50 表示 50%）
    memoryusage_s   memoutput_value;                      //内存使用情况
}dev_usage_status_s;

//设备升级命令  略过 未加返回值
int sg_handle_dev_install_cmd(int32_t mid, device_upgrade_s cmdobj)
{
    dev_status_reply_s status;
    status.jobId = cmdobj.jobId;
    status.progress = 0;
    status.state = STATUS_PRE_DOWNLOAD;
    set_device_upgrade_status(status);		//设置开始状态
    patch_status_s patch_status = { 0 };
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    //参数校验 ？校验什么？回复
    if (sysman_rpc_transport_open() != VOS_OK) {
        return VOS_ERR;
    }
    software_management_action_call_install_patch(DEFAULT_FILE_PATH, NULL, true, errmsg, SYSMAN_RPC_ERRMSG_MAX);  //待完善
    sysman_rpc_transport_close();

    if (sysman_rpc_transport_open() != VOS_OK) {
        return VOS_ERR;
    }
    software_management_status_call_get_patch_status(&patch_status, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    if (item == NULL) {
        printf("Error - unable to allocate required memory\n");
    }
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_install_cmd(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);        //回答入列

    //下载签名文件

    //下载升级文件

    //判断操作类型
    if (cmdobj.upgradeType == UPGRADEPACH) {
        sg_down_update_patch(cmdobj, errormsg);
    } else if (cmdobj.upgradeType == UPGRADEHOST) {
        sg_down_update_host(cmdobj, errormsg);
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    //保存断面文件
    return VOS_OK;
}
//设备升级命令 断面  略过
void sg_handle_dev_install_section(sg_dev_section_info_s pcfg)
{
    dev_status_reply_s status;
    status.jobId = pcfg.jobid;
    status.progress = 100;
    status.state = STATUS_FINISH_INSTALL;
    set_device_upgrade_status(status);		//设置状态
    //版本判断
    //结果上报
}

static int sg_devusage_status_call_get_threshold(dev_usage_status_s *dev_status)  //代码优化参考 sg_get_devusage_threshold(int *threshold, uint8_t select)
{
    int ret = VOS_OK;
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        ret = VOS_ERR;
    }
    ret = cpuusage_status_call_get_threshold(&dev_status->cpuoutput_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    if (ret != VOS_OK) {
        printf("cpuusage_status_call_get_threshold error! \n");
        ret = VOS_ERR;
    }
    ret = memoryusage_status_call_get_threshold(&dev_status->memoutput_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    if (ret != VOS_OK) {
        printf("memoryusage_status_call_get_threshold error! \n");
        ret = VOS_ERR;
    }
    sysman_rpc_transport_close();
    return ret;
}

//设备状态上报
int sg_handle_dev_inquire_reply(int32_t mid)
{
    int i = 0;
    int ret = VOS_OK;
    uint32_t infoNum = 0;
    uint32_t num = 0;
    long dev_num = 0;
    time_t now_time;
    now_time = time(NULL);
    char timestamp[DATA_BUF_F32_SIZE] = { 0 };
    char errormsg[DATA_BUF_F256_SIZE] = { 0 };
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    dev_usage_status_s devusage = { 0 };
    GNSS_CMD_LOCATION_S gnss_location = { 0 };
    dev_sta_reply_s dev_sta_rep_item = { 0 };
    container_install_dir_info container_install_dir = { 0 };
    storageusage_s *storageoutput_value = NULL;
    mqtt_data_info_s *item = NULL;

    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    if (sg_devusage_status_call_get_threshold(&devusage) != VOS_OK) {
        printf("sg_devusage_status_call_get_threshold error!\n");
        ret = VOS_ERR;
    }
    dev_sta_rep_item.cpuRate = devusage.cpuoutput_value.usage;
    printf("devusage.cpuoutput_value.usage = %d \n", devusage.cpuoutput_value.usage);

    dev_sta_rep_item.mem_used.phy = devusage.memoutput_value.usage;               //物理内存
    printf("devusage.memoutput_value.usage = %d \n", devusage.memoutput_value.usage);

    dev_sta_rep_item.mem_used.virt = sg_memvirt_used();					 //虚拟内存    系统信息自实现获取
    printf("sg_memvirt_used = %d \n", sg_memvirt_used);

    //磁盘占用率获取
    storageoutput_value = (storageusage_s*)VOS_Malloc(MID_SGDEV, sizeof(storageusage_s) * STORAGE_PARTITION_MAX_NUM);
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    ret = storageusage_status_call_get_threshold(&infoNum, storageoutput_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("storageusage_status_call_get_threshold error! \n");
        return VOS_ERR;
    }
    if (vm_rpc_get_install_dir_info(&container_install_dir, errmsg, SYSMAN_RPC_ERRMSG_MAX) != true) {  //获取当前磁盘路径
        printf("vm_rpc_get_install_dir_info error! \n");
        return VOS_ERR;
    }
    printf("container_install_dir.current_dir.dir_path = %s\n", container_install_dir.current_dir.dir_path);

    for (num = 0; num < infoNum; num++) {   //不确定用哪个  先都打印一下  找翟工确认  确认后 根据接口  如果匹配成功 返回正确结果 如果失败找不到 就是100%  加syslog
        printf("storageoutput_value[%d].path = %s\n", num, storageoutput_value[num].path);
        if (0 == strncmp(storageoutput_value[num].path, container_install_dir.current_dir.dir_path,
            strlen(container_install_dir.current_dir.dir_path))) {
            printf("usage %d = %d \n", num, storageoutput_value[num].usage);
            dev_sta_rep_item.diskUsed = storageoutput_value[num].usage;
            printf("storageoutput_value[%d].usage = %d \n", num, storageoutput_value[num].usage);
            break;
        } else {
            dev_sta_rep_item.diskUsed = 100;
            ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "get_storageoutput_usage_value_fail");
        }
    }
    VOS_Free(storageoutput_value);

    //温度获取 主板（CPU）温度需要判断获取********************************************************************************
    if (sg_get_device_temperature_threshold(&dev_sta_rep_item) != VOS_OK) {
        return VOS_ERR;
    }
    printf("****dev_sta_rep_item.tempValue = %d\n", dev_sta_rep_item.tempValue);

    sg_mqtttimestr(now_time, timestamp, sizeof(timestamp), TIME_UTC);           //使用UTC时间
    if (timestamp != NULL) {
        memcpy_s(dev_sta_rep_item.devDateTime, DATA_BUF_F64_SIZE, timestamp, strlen(timestamp) + 1); //设备当前时间
        printf("timestamp = %s \n", timestamp);
    }
    sg_get_devstdatetime(dev_sta_rep_item.devStDateTime, &dev_num); 	            //获取设备最近一次启动时间
    printf("devStDateTime = %s \n", dev_sta_rep_item.devStDateTime);

    dev_sta_rep_item.devRunTime = (uint32_t)dev_num;                            //获取设备运行时长
    printf("devRunTime = %u \n", dev_sta_rep_item.devRunTime);

    sgcc_get_links_info2(&dev_sta_rep_item.linkState, &dev_sta_rep_item.link_len);//获取links信息

    //地理位置信息经纬度获取
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    ret = gnss_status_call_get_gnss_location(&gnss_location, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("gnss_status_call_get_gnss_location error! \n");
        return VOS_ERR;
    }
    sprintf_s(dev_sta_rep_item.longitude, DATA_BUF_F64_SIZE, "%.5f", gnss_location.longitude);
    sprintf_s(dev_sta_rep_item.latitude, DATA_BUF_F64_SIZE, "%.5f", gnss_location.latitude);
    printf("gnss_location.longitude = %.5f\n", gnss_location.longitude);
    printf("dev_sta_rep_item.longitude = %s\n", dev_sta_rep_item.longitude);

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_run_status(&dev_sta_rep_item, NULL, item->msg_send);  //NULL 不传msg
    sg_push_pack_item(item);        //入列
    return VOS_OK;
}

//设备状态查询命令
int sg_handle_dev_inquire_cmd(int32_t mid)
{
    int i = 0;
    int ret = VOS_OK;
    uint16_t code = REQUEST_SUCCESS;
    uint32_t infoNum = 0;
    uint32_t num = 0;
    long dev_num = 0;
    time_t now_time;
    now_time = time(NULL);
    char   timestamp[DATA_BUF_F32_SIZE] = { 0 };
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    char errormsg[DATA_BUF_F256_SIZE] = { 0 };
    cpuusage_s cpuoutput_value = { 0 };
    memoryusage_s memoutput_value = { 0 };
    GNSS_CMD_LOCATION_S gnss_location = { 0 };
    dev_sta_reply_s dev_sta_rep_item = { 0 };
    storageusage_s *storageoutput_value = NULL;
    mqtt_data_info_s *item = NULL;

    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    ret = cpuusage_status_call_get_threshold(&cpuoutput_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("cpuusage_status_call_get_threshold error! \n");
        return VOS_ERR;
    }
    dev_sta_rep_item.cpuRate = cpuoutput_value.usage;
    printf("cpuoutput_value.usage = %d \n", cpuoutput_value.usage);


    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    ret = memoryusage_status_call_get_threshold(&memoutput_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("memoryusage_status_call_get_threshold error! \n");
        return VOS_ERR;
    }
    dev_sta_rep_item.mem_used.phy = memoutput_value.usage;               //物理内存
    printf("memoutput_value.usage = %d \n", memoutput_value.usage);

    dev_sta_rep_item.mem_used.virt = sg_memvirt_used();					 //虚拟内存  	系统信息自实现获取
    printf("sg_memvirt_used = %d \n", sg_memvirt_used);

    //磁盘占用率获取
    storageoutput_value = (storageusage_s*)VOS_Malloc(MID_SGDEV, sizeof(storageusage_s) * STORAGE_PARTITION_MAX_NUM);
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    ret = storageusage_status_call_get_threshold(&infoNum, storageoutput_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("storageusage_status_call_get_threshold error! \n");
        return VOS_ERR;
    }
    for (num = 0; num < infoNum; num++) {   //不确定用哪个  先都打印一下
        printf("path %d = %s \n", num, storageoutput_value[num].path);
        printf("name %d = %s \n", num, storageoutput_value[num].name);
        printf("alert %d = %d \n", num, storageoutput_value[num].alert);
        printf("warning %d = %d \n", num, storageoutput_value[num].warning);
        printf("usage %d = %d \n", num, storageoutput_value[num].usage);
    }
    dev_sta_rep_item.diskUsed = storageoutput_value[0].usage;
    printf("storageoutput_value[0].usage = %d \n", storageoutput_value[0].usage);
    VOS_Free(storageoutput_value);

    //温度获取 主板（CPU）温度需要判断获取********************************************************************************
    if (sg_get_device_temperature_threshold(&dev_sta_rep_item) != VOS_OK) {
        return VOS_ERR;
    }

    sg_mqtttimestr(now_time, timestamp, sizeof(timestamp), TIME_UTC);           //使用UTC时间
    if (timestamp != NULL) {
        memcpy_s(dev_sta_rep_item.devDateTime, DATA_BUF_F64_SIZE, timestamp, strlen(timestamp) + 1); //设备当前时间
        printf("timestamp = %s \n", timestamp);
    }
    sg_get_devstdatetime(dev_sta_rep_item.devStDateTime, &dev_num); 	            //获取设备最近一次启动时间
    printf("devStDateTime = %s \n", dev_sta_rep_item.devStDateTime);

    dev_sta_rep_item.devRunTime = (uint32_t)dev_num;                            //获取设备运行时长
    printf("devStDateTime = %u \n", dev_sta_rep_item.devRunTime);

    sgcc_get_links_info2(&dev_sta_rep_item.linkState, &dev_sta_rep_item.link_len);//获取links信息

    //地理位置信息经纬度获取
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    ret = gnss_status_call_get_gnss_location(&gnss_location, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("gnss_status_call_get_gnss_location error! \n");
        return VOS_ERR;
    }
    sprintf_s(dev_sta_rep_item.longitude, DATA_BUF_F64_SIZE, "%.5f", gnss_location.longitude);
    sprintf_s(dev_sta_rep_item.latitude, DATA_BUF_F64_SIZE, "%.5f", gnss_location.latitude);
    printf("gnss_location.longitude = %.5f\n", gnss_location.longitude);
    printf("dev_sta_rep_item.longitude = %s\n", dev_sta_rep_item.longitude);

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_run_status_reply(code, mid, errormsg, &dev_sta_rep_item, item->msg_send);
    sg_push_pack_item(item);        //入列
    return VOS_OK;
}

//设备信息查询命令
int sg_handle_dev_info_cmd(int32_t mid)
{
    int ret = VOS_OK;
    uint8_t i = 0;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE] = { 0 };
    mqtt_data_info_s *item = NULL;
    dev_info_inq_reply_s dev_info_inq_reply_item = { 0 };
    temperature_threshold_status_s temp_threshold_get = { 0 };

    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    if (sg_get_dev_devinfo(&dev_info_inq_reply_item.dev) != VOS_OK) {
        printf("sg_get_dev_devinfo error! \n");
        ret = VOS_ERR;
    }
    if (sg_get_cpu_devinfo(&dev_info_inq_reply_item.cpu) != VOS_OK) {
        printf("sg_get_cpu_devinfo error! \n");
        ret = VOS_ERR;
    }
    if (sg_get_mem_devinfo(&dev_info_inq_reply_item.mem) != VOS_OK) {
        printf("sg_get_mem_devinfo error! \n");
        ret = VOS_ERR;
    }
    if (sg_get_disk_devinfo(&dev_info_inq_reply_item.disk) != VOS_OK) {
        printf("sg_get_disk_devinfo error! \n");
        ret = VOS_ERR;
    }
    if (sg_get_os_devinfo(&dev_info_inq_reply_item.os) != VOS_OK) {
        printf("sg_get_os_devinfo error! \n");
        ret = VOS_ERR;
    }
    if (sg_get_monitor_temperature_threshold(&dev_info_inq_reply_item.temperature) != VOS_OK) {         //温度获取
        printf("sg_get_monitor_temperature_threshold error! \n");
        ret = VOS_ERR;
    }
    if (sgcc_get_links_info(&dev_info_inq_reply_item.links, &dev_info_inq_reply_item.link_len) != VOS_OK) {//获取links信息
        printf("sgcc_get_links_info error! \n");
        ret = VOS_ERR;
    }
    if (sg_read_period_file(&dev_info_inq_reply_item.rep_period) != VOS_OK) {                   //周期参数获取
        printf("sg_read_period_file error! \n");
        ret = VOS_ERR;
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

    sg_pack_dev_info_reply(code, mid, errormsg, &dev_info_inq_reply_item, item->msg_send);
    sg_push_pack_item(item);                                                    //入列
    return ret;
}


//设备管理参数修改命令
int sg_handle_dev_set_para_cmd(int32_t mid, dev_man_conf_command_s paraobj)    //paraobj为传入参数
{
    uint8_t num = 0;
    uint32_t infoNum = 0;
    int cpu_alert = 0;
    int mem_alert = 0;
    int disk_alert = 0;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    mqtt_data_info_s *item = NULL;
    temperature_threshold_config_s *temp_threshold_set = NULL;
    temperature_threshold_status_s *temp_threshold_get = NULL;
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    if (sg_get_devusage_threshold(&cpu_alert, CPU_USAGE) != VOS_OK) {
        printf("sg_get_CPU_USAGE_threshold error!\n");
        return VOS_ERR;
    }
    if (paraobj.cpuLmt >= 50 && paraobj.cpuLmt <= (cpu_alert - 1)) {
        if (sysman_rpc_transport_open() != VOS_OK) {
            printf("sysman_rpc_transport_open error!\n");
            return VOS_ERR;
        }
        cpuusage_config_call_set_threshold(paraobj.cpuLmt, errormsg, DATA_BUF_F256_SIZE);
        sysman_rpc_transport_close();
    }

    if (sg_get_devusage_threshold(&mem_alert, MEM_USAGE) != VOS_OK) {
        printf("sg_get_MEM_USAGE_threshold error!\n");
        return VOS_ERR;
    }
    if (paraobj.memLmt >= 50 && paraobj.memLmt <= (mem_alert - 1)) {
        if (sysman_rpc_transport_open() != VOS_OK) {
            printf("sysman_rpc_transport_open error!\n");
            return VOS_ERR;
        }
        memoryusage_config_call_set_threshold(paraobj.memLmt, errormsg, DATA_BUF_F256_SIZE);
        sysman_rpc_transport_close();
    }

    if (sg_get_devusage_threshold(&disk_alert, STORAGE_USAGE) != VOS_OK) {
        printf("sg_get_STORAGE_USAGE_threshold error!\n");
        return VOS_ERR;
    }
    if (paraobj.diskLmt >= 50 && paraobj.diskLmt <= (disk_alert - 1)) {
        if (sysman_rpc_transport_open() != VOS_OK) {
            printf("sysman_rpc_transport_open error!\n");
            return VOS_ERR;
        }
        storageusage_config_call_set_threshold(paraobj.diskLmt, errormsg, DATA_BUF_F256_SIZE);
        sysman_rpc_transport_close();
    }
    //获取高低温阈值(仅供调试 后面删掉)
    temp_threshold_get = (temperature_threshold_status_s*)VOS_Malloc(MID_SGDEV, sizeof(temperature_threshold_status_s) *
        STORAGE_PARTITION_MAX_NUM);
    (void)memset_s(temp_threshold_get, MONITOR_TEMP_NUM * sizeof(temperature_threshold_status_s), 0, MONITOR_TEMP_NUM *
        sizeof(temperature_threshold_status_s));
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    temperature_status_call_get_monitor_threshold(&infoNum, temp_threshold_get, errmsg, SYSMAN_RPC_ERRMSG_MAX);  //有返回值

    for (num = 0; num < infoNum; num++) {   //这里需要进行"Main_board"对比获取
        if (0 == strncmp(temp_threshold_get[num].moduleName, TEMPERATURE_MODULE_NAME_MAINBOARD,
            strlen(TEMPERATURE_MODULE_NAME_MAINBOARD))) {
            printf("temLow = %d\n", temp_threshold_get[num].temLow);
            printf("temHigh = %d\n", temp_threshold_get[num].temHigh);
            printf("temVal = %d\n", temp_threshold_get[num].temVal);
            break;
        }
    }
    sysman_rpc_transport_close();
    (void)VOS_Free(temp_threshold_get);

    //设置高低温阈值
    temp_threshold_set = (temperature_threshold_config_s*)VOS_Malloc(MID_SGDEV, sizeof(temperature_threshold_config_s) *
        STORAGE_PARTITION_MAX_NUM);
    if (temp_threshold_set == NULL) {
        printf("temp_threshold_set = NULL\n");
        return VOS_ERR;
    }
    memset_s(temp_threshold_set, sizeof(temperature_threshold_config_s) *
        STORAGE_PARTITION_MAX_NUM, 0, sizeof(temperature_threshold_config_s) * STORAGE_PARTITION_MAX_NUM);
    memcpy_s(temp_threshold_set->moduleName, TEMP_NAME_LEN, TEMPERATURE_MODULE_NAME_MAINBOARD,
        strlen(TEMPERATURE_MODULE_NAME_MAINBOARD) + 1);
    temp_threshold_set->temType = TEMP_MONITOR_TEMP_ALL;
    temp_threshold_set->temLow = paraobj.temperature.temLow;
    temp_threshold_set->temHigh = paraobj.temperature.temHigh;

    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    temperature_config_call_set_monitor_threshold(temp_threshold_set, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();
    (void)VOS_Free(temp_threshold_set);

    //时间间隔设置
    sg_set_period(&paraobj.rep_period);

    //中间件执行
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_set_para_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);        //入列
    return VOS_OK;
}

//设备时间同步命令
int sg_handle_dev_set_time_cmd(int32_t mid, dev_time_command_s *timeobj)
{
    sys_time_s systime = { 0 };
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    if (sg_get_time(timeobj->dateTime, &systime) != VOS_OK) {
        printf("sg_get_time error!\n");
        return VOS_ERR;
    }
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    datetime_action_call_set_sys_datetime_systohc(&systime, errmsg, DATA_BUF_F256_SIZE);   //设置时间
    sysman_rpc_transport_close();

    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    datetime_config_call_set_timezone(timeobj->timeZone, errmsg, DATA_BUF_F256_SIZE);       //设置时区
    sysman_rpc_transport_close();

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

    sg_pack_dev_set_time_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}

//设备日志召回  先略过
void sg_handle_dev_log_cmd(int32_t mid, dev_log_recall_s logobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    file_info_s fileinfo = { 0 };
    //中间件执行
    // logobj.url=			//没有对应接口 需要自己打包
    // logobj.logType=

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

    sg_pack_dev_log_reply(code, mid, errormsg, &fileinfo, item->msg_send);
    sg_push_pack_item(item);
    //设备日志召回应答
}

//设备控制命令
int sg_handle_dev_ctrl_cmd(int32_t mid, char *action)
{
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    //先应答
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

    sg_pack_dev_ctrl_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    // 中间件执行
    // 延迟5秒服务，复位原因：用户手动命令复位

    if (strncmp(action, "os-reboot", strlen("os-reboot")) == 0) {
        ret = reboot_action_call_reboot_reason(5, BOARD_MAIN, REBOOT_REASON_USER_CMD, errormsg, DATA_BUF_F256_SIZE);   //重启终端系统
    } else if (strncmp(action, "edge-reboot", strlen("edge-reboot")) == 0) {
        sg_set_edge_reboot(REBOOT_EDGE_SET);            //重启终端组件
    } else {
        return VOS_ERR;
    }
    return ret;
}

//容器安装控制  
void sg_handle_container_install_cmd(int32_t mid, container_install_cmd_s cmdobj)
{
    bool app_flag = false;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_WAIT;
    char errormsg[DATA_BUF_F256_SIZE];
    dev_upgrede_res_reply_s statusobj = { 0 };
    dev_status_reply_s status = { 0 };
    code = REQUEST_SUCCESS;
    status.state = STATUS_PRE_DOWNLOAD;
    set_container_install_status(status);
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    if (strlen(cmdobj.withAPP.version) != 0) {
        app_flag = true;

        item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
        (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
        sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

        sg_pack_container_install_cmd(code, mid, errormsg, item->msg_send);
        sg_push_pack_item(item);
        status.state = STATUS_EXE_DOWNLOAD;
        set_container_install_status(status);
        if (sg_file_download(cmdobj.withAPP.file, statusobj.msg) != VOS_OK) {
            code = REQUEST_NOTEXITS;
            statusobj.code = REQUEST_NOTEXITS;
        }
    }
    status.state = STATUS_EXE_DOWNLOAD;
    set_container_install_status(status);

    if (sg_file_download(cmdobj.image, statusobj.msg) != VOS_OK) {
        code = REQUEST_NOTEXITS;
        statusobj.code = REQUEST_NOTEXITS;
    }
    if (cmdobj.policy > 0) {
        //添加至任务表
        return;
    }
    if (sg_container_install(&cmdobj, statusobj.msg) != VOS_OK) {
        code = REQUEST_FAILED;
        statusobj.code = REQUEST_FAILED;
    }
    if (app_flag) {
        if (sg_container_with_app_install(&cmdobj, statusobj.msg)) {            //安装指定APP
            code = REQUEST_FAILED;
            statusobj.code = REQUEST_FAILED;
        }
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    mqtt_data_info_s *sitem = NULL;
    sitem = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(sitem, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(sitem->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_data_pub());
    sg_pack_dev_install_result(statusobj, errormsg, sitem->msg_send);    //发送结果
    sg_push_pack_item(sitem);
}

//容器启动
int sg_handle_container_start_cmd(int32_t mid, char *container_name)
{
    bool result = true;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char errorcode[DATA_BUF_F256_SIZE];//ac用，本模块不用
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    //中间件执行
    result = vm_rpc_container_start(container_name, errorcode, DATA_BUF_F256_SIZE, errormsg, DATA_BUF_F256_SIZE);
    if (result != true) {
        printf("vm_rpc_container_start error!\n");
        return VOS_ERR;
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_start_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}

//容器停止
int sg_handle_container_stop_cmd(int32_t mid, char *container_name)
{
    bool result = true;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char errorcode[DATA_BUF_F256_SIZE];//ac用，本模块不用
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    result = vm_rpc_container_stop(container_name, errorcode, DATA_BUF_F256_SIZE, errormsg, DATA_BUF_F256_SIZE);
    if (result != true) {
        printf("vm_rpc_container_stop error!\n");
        return VOS_ERR;
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_stop_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}

//容器卸载
int sg_handle_container_delete_cmd(int32_t mid, char *container_name)
{
    bool result = true;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char errorcode[DATA_BUF_F256_SIZE];//ac用，本模块不用
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    result = vm_rpc_container_uninstall(container_name, errorcode, DATA_BUF_F256_SIZE, errormsg, DATA_BUF_F256_SIZE);
    if (result != true) {
        printf("vm_rpc_container_stop error!\n");
        return VOS_ERR;
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_remove_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}

//容器配置修改
void sg_handle_container_param_set_cmd(int32_t mid, container_conf_cmd_s *cmdobj)   //cmdobj为传入
{
    gboolean ret = 0;
    INSTALL_OVA_PARA_NORTH_S para = { 0 };
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char container_name[1024];
    char errormsg[DATA_BUF_F256_SIZE];
    container_alarm_type alarm_type = { 0 };
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    //资源设置接口，不需要重启容器：


    // para.north_type = MODE_MQTT;
    // para.ova_name =  
    // // para.uuid = 
    // memcpy_s(para.container_name, MAX_CONTAINER_NAME_LEN, cmdobj->container, strlen(cmdobj->container));

    // para.index = 
    // para.disk_size = 
    // para.mem_size = 
    // para.cpu_mask = 
    // para.dev_list = 
    // para.container_type = 

    ret = vm_rpc_container_modify_north(&para, errormsg, DATA_BUF_F256_SIZE);

    // 阈值设置接口，不需要重启容器：
    // vm_rpc_set_container_alarm_threshold(container_name, alarm_type,  unsigned int threshold_size, char *errRet, size_t errLen);   //2020.11.18编译  有报错先屏蔽  原因：可能没加相关头文件

    // cmdobj.cfgCpu.cpus = 
    // cmdobj.cfgCpu.cpuLmt = 
    // cmdobj.cfgMem.memory = 
    // cmdobj.cfgMem.memLmt = 
    // cmdobj.cfgDisk.disk = 
    // cmdobj.cfgDisk.diskLmt = 
    // cmdobj.port = 
    // cmdobj.mount[i] = 
    // cmdobj.dev[i] = 

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());
    sg_pack_container_param_set_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
}

//容器配置查询命令 
int sg_handle_container_param_get(int32_t mid)
{
    int ret = VOS_OK;
    int num = 0;
    bool result = true;
    int container_cnt = 0;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;

    CONTAINER_INFO_S *container_info = NULL;
    container_config_reply_s contiainer_config_reply = { 0 };
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    result = vm_rpc_container_status(&container_info, &container_cnt, NULL, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    if (result != true) {
        printf("vm_rpc_container_status error!\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    printf("container_cnt = %d\n", container_cnt);
    if (container_cnt == 0) {    //是否容器个数为零
        code = REQUEST_FAILED;
        if (container_info != NULL) {			//将内存释放掉
            VOS_Free(container_info);
            container_info = NULL;
        }
    } else {                     //如果个数不等于零
        contiainer_config_reply.contPara_len = container_cnt;
        contiainer_config_reply.contPara = (container_conf_cmd_s *)VOS_Malloc(MID_SGDEV,
            sizeof(container_conf_cmd_s) * container_cnt);
        if (contiainer_config_reply.contPara == NULL) {
            printf("malloc use failure\n");
            code = REQUEST_FAILED;
            ret = VOS_ERR;
        }
        (void)memset_s(contiainer_config_reply.contPara, sizeof(container_conf_cmd_s)  * container_cnt,
            0, sizeof(container_conf_cmd_s) * container_cnt);
        for (num = 0; num < container_cnt; num++) {
            memcpy_s(contiainer_config_reply.contPara[num].container, DATA_BUF_F64_SIZE, container_info[num].container_name,
                strlen(container_info[num].container_name));  //容器名称赋值
            contiainer_config_reply.contPara[num].cfgCpu.cpus = sg_hamming_weight(container_info[num].cpu_mask);              //CPU核数赋值
            printf("container_info[%d].cpu_mask = %lld\n", num, container_info[num].cpu_mask);

            contiainer_config_reply.contPara[num].cfgCpu.cpuLmt = container_info[num].cpu_threshold;                          //CPU监控阈值
            contiainer_config_reply.contPara[num].cfgMem.memory = container_info[num].container_mem_size;                     //内存限值赋值
            contiainer_config_reply.contPara[num].cfgMem.memLmt = container_info[num].memory_threshold;                       //内存监控阈值赋值
            contiainer_config_reply.contPara[num].cfgDisk.disk = container_info[num].container_storage_size;                  //存储限值赋值
            contiainer_config_reply.contPara[num].cfgDisk.diskLmt = container_info[num].storage_threshold;                    //磁盘存储监控阈值，百分数

            printf("contiainer_config_reply.contPara[%d].cfgCpu.cpus = %d\n", num, contiainer_config_reply.contPara[num].cfgCpu.cpus);
            printf("contiainer_config_reply.contPara[%d].cfgCpu.cpuLmt = %d\n", num, contiainer_config_reply.contPara[num].cfgCpu.cpuLmt);
            printf("contiainer_config_reply.contPara[%d].cfgMem.memory = %d\n", num, contiainer_config_reply.contPara[num].cfgMem.memory);
            printf("contiainer_config_reply.contPara[%d].cfgMem.memLmt = %d\n", num, contiainer_config_reply.contPara[num].cfgMem.memLmt);
            printf("contiainer_config_reply.contPara[%d].cfgDisk.disk = %d\n", num, contiainer_config_reply.contPara[num].cfgDisk.disk);
            printf("contiainer_config_reply.contPara[%d].cfgDisk.diskLmt = %d\n", num, contiainer_config_reply.contPara[num].cfgDisk.diskLmt);
            // contiainer_config_reply.contPara[num].port =                                //端口资源配置参数

        }
        if (container_info != NULL) {			//将内存释放掉
            VOS_Free(container_info);
            container_info = NULL;
        }
    }

    // sysman_rpc_transport_close();
    //中间件执行
    // gboolean vm_status_call_get_container_device_list(char *container_name, container_dev_node **node_list, int *node_cnt, char *errRet, size_t errLen);

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_param_get_reply(code, mid, errormsg, &contiainer_config_reply, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}

// 容器状态查询应答， 容器状态上报
int sg_handle_container_status_get(char *type, int32_t mid)
{
    int ret = VOS_OK;
    char *ptr = NULL;
    char errormsg[DATA_BUF_F256_SIZE] = { 0 };
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    char up_timestr[DATA_BUF_F256_SIZE] = { 0 };
    int num = 0;
    int container_cnt = 0;                          //这个为container_info的个数
    bool result = true;
    uint16_t code = REQUEST_SUCCESS;
    time_t uptime_num = 0;
    time_t up_time = 0;
    struct tm tmp_ptr = { 0 };
    mqtt_data_info_s *item = NULL;
    CONTAINER_INFO_S *container_info = NULL;
    container_status_reply_s *status = NULL;
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    result = vm_rpc_container_status(&container_info, &container_cnt, NULL, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    if (result != true) {                        //获取信息成功 接收获取到的信息
        printf("vm_rpc_container_status error!\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    if (container_cnt == 0) {
        code = REQUEST_FAILED;
        if (container_info != NULL) {			//将内存释放掉
            VOS_Free(container_info);
            container_info = NULL;
        }
    } else {
        status = (container_status_reply_s *)VOS_Malloc(MID_SGDEV, sizeof(container_status_reply_s) * container_cnt);
        if (status == NULL) {
            printf("malloc use failure\n");
            code = REQUEST_FAILED;
            ret = VOS_ERR;
        }
        (void)memset_s(status, sizeof(container_status_reply_s)  * container_cnt,
            0, sizeof(container_status_reply_s) * container_cnt);
        printf("container_cnt = %d\n", container_cnt);
        status->container_len = container_cnt;
        for (num = 0; num < container_cnt; num++) {
            memcpy_s(status[num].container, DATA_BUF_F64_SIZE, container_info[num].container_name,
                strlen(container_info[num].container_name));  //容器名称赋值
            memcpy_s(status[num].version, DATA_BUF_F64_SIZE, container_info[num].container_version,
                strlen(container_info[num].container_version));  //容器版本号
            memcpy_s(status[num].state, DATA_BUF_F64_SIZE, container_info[num].container_status,
                strlen(container_info[num].container_status));  	//容器运行状态， running 或 stopped
            memcpy_s(status[num].ip, DATA_BUF_F64_SIZE, container_info[num].container_ipaddr,
                strlen(container_info[num].container_ipaddr));  	//IP 地址及端口
            status[num].cpuRate = container_info[num].cpuusage;                     //CPU占用率，百分比
            status[num].memUsed = container_info[num].memoryusage;                  //内存占用率，百分比
            status[num].diskUsed = container_info[num].stroageusage;                //磁盘占用率，百分比

            printf("status[%d].container = %s\n", num, status[num].container);
            printf("status[%d].version = %s\n", num, status[num].version);
            printf("status[%d].state = %s\n", num, status[num].state);
            printf("status[%d].ip = %s\n", num, status[num].ip);
            printf("status[%d].cpuRate = %d\n", num, status[num].cpuRate);
            printf("status[%d].memUsed = %d\n", num, status[num].memUsed);
            printf("status[%d].diskUsed = %d\n", num, status[num].diskUsed);
            printf("container_info[%d].container_up_time = %s\n", num, container_info[num].container_up_time);
            ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "status[%d].container = %s\n", num, status[num].container);
            ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "status[%d].version = %s\n", num, status[num].version);
            ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "status[%d].state = %s\n", num, status[num].state);

            //获取最近一次启动时间 先判断uptime_num是不是等于0
            uptime_num = strtoul(container_info[num].container_up_time, &ptr, 10);
            printf("uptime_num = %lu\n", uptime_num);
            if (uptime_num == 0) {
                printf("uptime_num get error!\n");
                code = REQUEST_FAILED;
                ret = VOS_ERR;
            }
            (void)localtime_r(&uptime_num, &tmp_ptr);
            (void)sprintf_s(status[num].started, DATA_BUF_F64_SIZE, "%d-%d-%d %d:%d:%d", 1900 + tmp_ptr.tm_year,  //获取最近一次启动时间
                1 + tmp_ptr.tm_mon, tmp_ptr.tm_mday, tmp_ptr.tm_hour, tmp_ptr.tm_min, tmp_ptr.tm_sec);
            printf("status[%d].started = %s\n", num, status[num].started);
            //获取运行时间 先判断一下status 是不是出于running状态  然后用 当前时间减去最近一次启动时间就是运行时间
            if (strncmp(status[num].state, "running", strlen("running")) != 0) {
                status[num].lifeTime = 0;
            } else {
                up_time = time(NULL) - uptime_num;
                printf("up_time = %lu\n", up_time);
                status[num].lifeTime = up_time;                 //运行时间，单位：秒
                printf("status[%d].lifeTime = %lu\n", num, status[num].lifeTime);
            }
            //status.created =  		//创建时间      暂时没有
            //status.image =    		//容器镜像信息  暂时没有
        }
        if (container_info != NULL) {			//将内存释放掉
            VOS_Free(container_info);
            container_info = NULL;
        }
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    if (sg_pack_container_status_get_reply(type, code, mid, errormsg, status, item->msg_send) != VOS_OK) {
        ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "ssp_syslog sg_pack_container_status_get_reply error !\n");
        ret = VOS_ERR;
    }
    if (status != NULL) {
        VOS_Free(status);
        status = NULL;
    }
    sg_push_pack_item(item);
    return ret;
}

//容器升级  先忽略
void sg_handle_container_upgrade_cmd(int32_t mid, container_upgrade_cmd_s cmdobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //先应答

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_upgrade_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);

    //中间件执行
    // cmdobj.jobId = 
    // cmdobj.policy = 
    // cmdobj.version = 
    // cmdobj.file.name = 
    // cmdobj.file.fileType = 
    // cmdobj.file.url = 
    // cmdobj.file.size = 
    // cmdobj.file.md5 = 
    // cmdobj.file.sign.name = 
    // cmdobj.file.sign.url = 
    // cmdobj.file.sign.size = 
    // cmdobj.file.sign.md5 = 
}

//容器日志召回   先略过
void sg_handle_container_log_get_cmd(int32_t mid, container_log_recall_cmd_s cmdobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    file_info_s fileinfo;
    //中间件执行

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());
    sg_pack_container_log_get_reply(code, mid, errormsg, &fileinfo, item->msg_send);
    sg_push_pack_item(item);
}

//应用安装控制
void sg_handle_app_install_cmd(int32_t mid, app_install_cmd_s cmdobj)
{
    printf("sgdevagent**** : sg_handle_app_install_cmd  mid= %d.\n", mid);
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_WAIT;
    char errormsg[DATA_BUF_F256_SIZE];
    APPM_OPERATION_PARA para = { 0 };
    dev_status_reply_s status = { 0 };
    dev_upgrede_res_reply_s statusobj = { 0 };

    statusobj.jobId = cmdobj.jobId;
    status.jobId = cmdobj.jobId;
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    status.state = STATUS_PRE_DOWNLOAD;
    set_app_install_status(status);		//设置开始状态

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_install_cmd(code, mid, errormsg, item->msg_send);    //收到命令先返回
    sg_push_pack_item(item);

    code = REQUEST_SUCCESS;
    status.state = STATUS_EXE_DOWNLOAD;
    set_app_install_status(status);		//设置正在下载
    printf("sgdevagent**** : sg_file_download  cmdobj.file= %s.\n", cmdobj.file);
    if (sg_file_download(cmdobj.file, statusobj.msg) != VOS_OK) {
        code = REQUEST_NOTEXITS;
        statusobj.code = REQUEST_NOTEXITS;
    }
    status.state = STATUS_PRE_INSTALL;
    set_app_install_status(status);		//开始安装
    if (cmdobj.policy > 0) { //需要开启另外的线程吗？ 重开定时器 		//向线程发送任务
        return;
        VOS_T_Delay(cmdobj.policy * 1000);
    }
    if (sg_app_install(cmdobj, statusobj.msg) != VOS_OK) {
        code = REQUEST_REFUSE;
        statusobj.code = REQUEST_REFUSE;
    }
    printf("sgdevagent**** : sg_app_install  statusobj.msg= %s.\n", statusobj.msg);
    status.state = STATUS_FINISH_INSTALL;
    set_app_install_status(status);		//设置安装完成
    mqtt_data_info_s *sitem = NULL;
    sitem = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(sitem, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(sitem->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_data_pub());
    sg_pack_dev_install_result(statusobj, errormsg, sitem->msg_send);    //发送结果
    sg_push_pack_item(sitem);
}
//应用启动
int sg_handle_app_start_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];

    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmdobj->container, strlen(cmdobj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmdobj->app, strlen(cmdobj->app));
    //中间件执行
    if (appm_rpc_transport_open() != VOS_OK) {
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    ret = app_management_action_call_app_start(&para, &errmsg);
    appm_rpc_transport_close();

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_start_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}
//应用停止
void sg_handle_app_stop_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmdobj->container, strlen(cmdobj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmdobj->app, strlen(cmdobj->app));
    if (appm_rpc_transport_open() != VOS_OK) {
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    ret = app_management_action_call_app_stop(&para, &errmsg);
    appm_rpc_transport_close();

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_stop_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
}
//应用卸载
void sg_handle_app_uninstall_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmdobj->container, strlen(cmdobj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmdobj->app, strlen(cmdobj->app));
    if (appm_rpc_transport_open() != VOS_OK) {
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    ret = app_management_action_call_app_uninstall(&para, &errmsg);
    appm_rpc_transport_close();

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_uninstall_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
}
//应用使能
void sg_handle_app_enble_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmdobj->container, strlen(cmdobj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmdobj->app, strlen(cmdobj->app));
    if (appm_rpc_transport_open() != VOS_OK) {
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    ret = app_management_action_call_app_enable(&para, &errmsg);
    appm_rpc_transport_close();

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_enable_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
}
//应用去使能
void sg_handle_app_unenble_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmdobj->container, strlen(cmdobj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmdobj->app, strlen(cmdobj->app));
    if (appm_rpc_transport_open() != VOS_OK) {
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    ret = app_management_action_call_app_disable(&para, &errmsg);
    appm_rpc_transport_close();

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_unenble_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
}

//应用配置修改     不支持 先略过
void sg_handle_app_param_set_cmd(int32_t mid, app_conf_cmd_s cmdobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_param_set_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
}
//应用配置状态查询命令
void sg_handle_app_param_get(int32_t mid, char *container_name)
{
    int ret = 0;
    uint16_t num = 0;
    mqtt_data_info_s *item = NULL;
    app_info_t *info = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char lxc_name[1024];
    int app_cnt = 0;
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    app_conf_reply_s statusobj = { 0 };
    //中间件执行
    if (appm_rpc_transport_open() != VOS_OK) {
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    ret = app_management_status_call_get_app_info(lxc_name, &info, &app_cnt, errormsg);
    appm_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("app_management_status_call_get_app_info error! \n");
        ret = VOS_ERR;
    }
    statusobj.app_num = app_cnt;
    if (NULL != lxc_name) {
        memcpy_s(statusobj.container, DATA_BUF_F64_SIZE, lxc_name, strlen(lxc_name) + 1);  //容器名称赋值
    }
    for (num = 0; num < statusobj.app_num; num++) {
        statusobj.appCfgs[num].cfgCpu.cpus = info->services[num].cpu_usage_current;						//CPU核数（例如值为2,3,4）
        statusobj.appCfgs[num].cfgCpu.cpuLmt = info->services[num].cpu_usage_threshold; 					//CPU监控阈值
        statusobj.appCfgs[num].cfgMem.memory = info->services[num].memory_usage_current;					//内存限值,单位： M Byte
        statusobj.appCfgs[num].cfgMem.memLmt = info->services[num].memory_usage_threshold; 					//内存监控阈值，百分数
        if (NULL != info->services[num].name) {
            memcpy_s(statusobj.appCfgs[num].app, DATA_BUF_F64_SIZE, info->services[num].name, strlen(info->services[num].name) + 1); 	//应用文件名字
        }
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_param_get_reply(code, mid, errormsg, &statusobj, item->msg_send);
    sg_push_pack_item(item);
}

//应用状态查询命令
void sg_handle_app_status_get(int32_t mid, char *container_name)
{
    int ret = VOS_OK;
    int i = 0;
    int app_cnt = 0;
    uint16_t num = 0;
    uint16_t process_count = 0;
    uint16_t test_count = 0;
    uint16_t num_process = 0;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE] = { 0 };
    mqtt_data_info_s *item = NULL;
    app_info_t *app_info = NULL;
    app_inq_reply_s statusobj = { 0 };

    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //中间件执行
    if (appm_rpc_transport_open() != VOS_OK) {
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    ret = app_management_status_call_get_app_info(container_name, &app_info, &app_cnt, errormsg);
    appm_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("app_management_status_call_get_app_info error! \n");
        ret = VOS_ERR;
    }
    statusobj.apps_num = app_cnt;
    printf("app_cnt = %d\n", app_cnt);
    memcpy_s(statusobj.container, DATA_BUF_F64_SIZE, container_name, strlen(container_name));
    printf("statusobj.container = %s\n", statusobj.container);
    statusobj.apps = (apps_info_s*)VOS_Malloc(MID_SGDEV, sizeof(apps_info_s) * app_cnt);            //记得释放
    if (statusobj.apps == NULL) {
        printf("malloc use failure\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    (void)memset_s(statusobj.apps, sizeof(apps_info_s) * app_cnt, 0, sizeof(apps_info_s) * app_cnt);
    for (num = 0; num < app_cnt; num++) {  //容器下面有多少个APP，每个APP下面对应的就是多少个进程
        process_count = 0;
        memcpy_s(statusobj.apps[num].app, DATA_BUF_F64_SIZE, app_info[num].name, strlen(app_info[num].name));          //APP 名称
        memcpy_s(statusobj.apps[num].version, DATA_BUF_F32_SIZE, app_info[num].version, strlen(app_info[num].version));//APP 版本号
        memcpy_s(statusobj.apps[num].appHash, DATA_BUF_F64_SIZE, app_info[num].hash, strlen(app_info[num].hash)); 		//APP 的哈希值

        printf("statusobj.apps[%d].app = %s\n", num, statusobj.apps[num].app);
        printf("statusobj.apps[%d].version = %s\n", num, statusobj.apps[num].version);
        printf("statusobj.apps[%d].appHash = %s\n", num, statusobj.apps[num].appHash);

        //需要根据实际进程名称是不是有数据来判断获取进程数
        for (i = 0; i < APP_MANAGEMENT_APP_SERVICE_MAX; i++) {
            if (strlen(app_info[num].services[i].name) != 0) {
                process_count++;
            } else {
                break;
            }
        }
        statusobj.apps[num].srvNumber = process_count;                                              //当前 APP 的进程数量
        printf("statusobj.apps[%d].srvNumber = %d\n", num, process_count);

        statusobj.apps[num].process = (process_info_s*)VOS_Malloc(MID_SGDEV, sizeof(process_info_s) * process_count);            //记得释放
        if (statusobj.apps[num].process == NULL) {
            printf("malloc use failure\n");
            code = REQUEST_FAILED;
            ret = VOS_ERR;
        }
        (void)memset_s(statusobj.apps[num].process, sizeof(process_info_s) * process_count, 0, sizeof(process_info_s) * process_count);

        for (num_process = 0; num_process < process_count; num_process++) {                      //进程信息获取
            statusobj.apps[num].process[num_process].srvIndex = num_process;                   //进程索引
            memcpy_s(statusobj.apps[num].process[num_process].srvName, DATA_BUF_F64_SIZE, app_info[num].services[num_process].name, strlen(app_info[num].services[num_process].name)); //进程名称
            if(app_info[num].services[num_process].enable == 1) {   // 服务使能状态， yes 或 no
                memcpy_s(statusobj.apps[num].process[num_process].srvEnable, DATA_BUF_F64_SIZE, "yes", strlen("yes"));
            } else {
                memcpy_s(statusobj.apps[num].process[num_process].srvEnable, DATA_BUF_F64_SIZE, "no", strlen("no"));
            }
            if(app_info[num].services[num_process].status == 0) {   // 服务状态， running 或 stopped
                memcpy_s(statusobj.apps[num].process[num_process].srvStatus, DATA_BUF_F64_SIZE, "running", strlen("running"));
            } else {
                memcpy_s(statusobj.apps[num].process[num_process].srvStatus, DATA_BUF_F64_SIZE, "stopped", strlen("stopped"));
            }
            statusobj.apps[num].process[num_process].cpuLmt = app_info[num].services[num_process].cpu_usage_threshold; 		//CPU 检测阈值，百分比数据
            statusobj.apps[num].process[num_process].cpuRate = app_info[num].services[num_process].cpu_usage_current;		//当前 CPU 使用率，百分比数据
            statusobj.apps[num].process[num_process].memLmt = app_info[num].services[num_process].memory_usage_threshold;	//内存检测阈值，百分比数据
            statusobj.apps[num].process[num_process].memUsed = app_info[num].services[num_process].memory_usage_current;	//当前内存使用空间的大小，百分比数据
            sprintf_s(statusobj.apps[num].process[num_process].startTime, DATA_BUF_F64_SIZE, "%d-%d-%d %d:%d:%d",
                app_info[num].services[num_process].start_time.year, app_info[num].services[num_process].start_time.month,
                app_info[num].services[num_process].start_time.day, app_info[num].services[num_process].start_time.hour,
                app_info[num].services[num_process].start_time.minute, app_info[num].services[num_process].start_time.second);  //表示服务启动时间

            printf("statusobj.apps[%d].process[%d].srvIndex = %d\n", num, num_process, statusobj.apps[num].process[num_process].srvIndex);
            printf("statusobj.apps[%d].process[%d].srvName = %s\n", num, num_process, statusobj.apps[num].process[num_process].srvName);
            printf("statusobj.apps[%d].process[%d].srvEnable = %s\n", num, num_process, statusobj.apps[num].process[num_process].srvEnable);
            printf("statusobj.apps[%d].process[%d].srvStatus = %s\n", num, num_process, statusobj.apps[num].process[num_process].srvStatus);
            printf("statusobj.apps[%d].process[%d].cpuLmt = %d\n", num, num_process, statusobj.apps[num].process[num_process].cpuLmt);
            printf("statusobj.apps[%d].process[%d].cpuRate = %d\n", num, num_process, statusobj.apps[num].process[num_process].memLmt);
            printf("statusobj.apps[%d].process[%d].memLmt = %d\n", num, num_process, statusobj.apps[num].process[num_process].memLmt);
            printf("statusobj.apps[%d].process[%d].memUsed = %d\n", num, num_process, statusobj.apps[num].process[num_process].memUsed);
            printf("statusobj.apps[%d].process[%d].startTime = %s\n", num, num_process, statusobj.apps[num].process[num_process].startTime);
        }
    
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_status_get_reply(code, mid, errormsg, &statusobj, item->msg_send);
    VOS_Free(statusobj.apps[num].process);
    statusobj.apps[num].process = NULL;
    VOS_Free(statusobj.apps); //释放
    statusobj.apps = NULL;
    sg_push_pack_item(item);

}
//应用升级  先略过
void sg_handle_app_upgrade_cmd(int32_t mid, app_upgrade_cmd_s cmdobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    //先应答
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_upgrade_cmd(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    //中间件执行
    // cmdobj.jobId = 
    // cmdobj.policy = 
    // cmdobj.version = 
    // cmdobj.container = 
    // cmdobj.file.name = 
    // cmdobj.file.fileType = 
    // cmdobj.file.url = 
    // cmdobj.file.size = 
    // cmdobj.file.md5 = 
    // cmdobj.file.sign.name = 
    // cmdobj.file.sign.url = 
    // cmdobj.file.sign.size = 
    // cmdobj.file.sign.md5 = 
}

//应用日志查询
void sg_handle_app_log_get_cmd(int32_t mid, app_log_recall_cmd_s cmdobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    file_info_s fielinfo;

    //中间件执行
    // cmdobj.container = 
    // cmdobj.url = 
    // cmdobj.app = 

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_log_get_reply(code, mid, errormsg, &fielinfo, item->msg_send);
    sg_push_pack_item(item);
}
