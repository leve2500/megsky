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

#include "sgdev_struct.h"
#include "sgdev_curl.h"
#include "sgdev_queue.h"

#include "sgdev_debug.h"
#include "task_app.h"
#include "task_container.h"
#include "task_deal.h"
#include "task_link.h"
#include "timer_pack.h"
#include "mqtt_json.h"
#include "mqtt_pub.h"

#include "thread_interact.h"

typedef struct dev_usage_status_reply {
    cpuusage_s      cpuoutput_value;                      //CPU ���أ����� 50 ��ʾ 50%��
    memoryusage_s   memoutput_value;                      //�ڴ�ʹ�����
}dev_usage_status_s;

//�豸��������  �Թ� δ�ӷ���ֵ
int sg_handle_dev_install_cmd(int32_t mid, device_upgrade_s *cmd_obj)
{
    dev_status_reply_s status;
    status.jobId = cmd_obj->jobId;
    status.progress = 0;
    status.state = STATUS_PRE_DOWNLOAD;
    set_device_upgrade_status(status);		//���ÿ�ʼ״̬
    patch_status_s patch_status = { 0 };
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    //����У�� ��У��ʲô���ظ�
    if (sysman_rpc_transport_open() != VOS_OK) {
        return VOS_ERR;
    }
    software_management_action_call_install_patch(DEFAULT_FILE_PATH, NULL, true, errmsg, SYSMAN_RPC_ERRMSG_MAX);  //������
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

    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_install_cmd(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);        //�ش�����

    //����ǩ���ļ�

    //���������ļ�

    //�жϲ�������
    if (cmd_obj.upgradeType == UPGRADEPACH) {
        sg_down_update_patch(cmd_obj, errormsg);
    } else if (cmd_obj.upgradeType == UPGRADEHOST) {
        sg_down_update_host(cmd_obj, errormsg);
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    //��������ļ�
    return VOS_OK;
}
//�豸�������� ����  �Թ�
void sg_handle_dev_install_section(sg_dev_section_info_s pcfg)
{
    dev_status_reply_s status;
    status.jobId = pcfg.jobid;
    status.progress = 100;
    status.state = STATUS_FINISH_INSTALL;
    set_device_upgrade_status(status);		//����״̬
    //�汾�ж�
    //����ϱ�
}

static int sg_devusage_status_call_get_threshold(dev_usage_status_s *dev_status)  //�����Ż��ο� sg_get_devusage_threshold(int *threshold, uint8_t select)
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

//�豸״̬�ϱ�
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

    dev_sta_rep_item.mem_used.phy = devusage.memoutput_value.usage;               //�����ڴ�
    printf("devusage.memoutput_value.usage = %d \n", devusage.memoutput_value.usage);

    dev_sta_rep_item.mem_used.virt = sg_memvirt_used();					 //�����ڴ�    ϵͳ��Ϣ��ʵ�ֻ�ȡ
    printf("sg_memvirt_used = %d \n", sg_memvirt_used);

    //����ռ���ʻ�ȡ
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
    if (vm_rpc_get_install_dir_info(&container_install_dir, errmsg, SYSMAN_RPC_ERRMSG_MAX) != true) {  //��ȡ��ǰ����·��
        printf("vm_rpc_get_install_dir_info error! \n");
        return VOS_ERR;
    }
    printf("container_install_dir.current_dir.dir_path = %s\n", container_install_dir.current_dir.dir_path);

    for (num = 0; num < infoNum; num++) {   //��ȷ�����ĸ�  �ȶ���ӡһ��  �ҵԹ�ȷ��  ȷ�Ϻ� ���ݽӿ�  ���ƥ��ɹ� ������ȷ��� ���ʧ���Ҳ��� ����100%  ��syslog
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

    //�¶Ȼ�ȡ ���壨CPU���¶���Ҫ�жϻ�ȡ********************************************************************************
    if (sg_get_device_temperature_threshold(&dev_sta_rep_item) != VOS_OK) {
        return VOS_ERR;
    }
    printf("****dev_sta_rep_item.tempValue = %d\n", dev_sta_rep_item.tempValue);

    sg_mqtttimestr(now_time, timestamp, sizeof(timestamp), TIME_UTC);           //ʹ��UTCʱ��
    if (timestamp != NULL) {
        memcpy_s(dev_sta_rep_item.devDateTime, DATA_BUF_F64_SIZE, timestamp, strlen(timestamp) + 1); //�豸��ǰʱ��
        printf("timestamp = %s \n", timestamp);
    }
    sg_get_devstdatetime(dev_sta_rep_item.devStDateTime, &dev_num); 	            //��ȡ�豸���һ������ʱ��
    printf("devStDateTime = %s \n", dev_sta_rep_item.devStDateTime);

    dev_sta_rep_item.devRunTime = (uint32_t)dev_num;                            //��ȡ�豸����ʱ��
    printf("devRunTime = %u \n", dev_sta_rep_item.devRunTime);

    sgcc_get_links_info2(&dev_sta_rep_item.linkState, &dev_sta_rep_item.link_len);//��ȡlinks��Ϣ

    //����λ����Ϣ��γ�Ȼ�ȡ
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

    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_run_status(&dev_sta_rep_item, NULL, item->msg_send);  //NULL ����msg
    sg_push_pack_item(item);        //����
    return VOS_OK;
}

//�豸״̬��ѯ����
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
    dev_sta_rep_item.mem_used.phy = memoutput_value.usage;               //�����ڴ�
    printf("memoutput_value.usage = %d \n", memoutput_value.usage);

    dev_sta_rep_item.mem_used.virt = sg_memvirt_used();					 //�����ڴ�  	ϵͳ��Ϣ��ʵ�ֻ�ȡ
    printf("sg_memvirt_used = %d \n", sg_memvirt_used);

    //����ռ���ʻ�ȡ
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
    for (num = 0; num < infoNum; num++) {   //��ȷ�����ĸ�  �ȶ���ӡһ��
        printf("path %d = %s \n", num, storageoutput_value[num].path);
        printf("name %d = %s \n", num, storageoutput_value[num].name);
        printf("alert %d = %d \n", num, storageoutput_value[num].alert);
        printf("warning %d = %d \n", num, storageoutput_value[num].warning);
        printf("usage %d = %d \n", num, storageoutput_value[num].usage);
    }
    dev_sta_rep_item.diskUsed = storageoutput_value[0].usage;
    printf("storageoutput_value[0].usage = %d \n", storageoutput_value[0].usage);
    VOS_Free(storageoutput_value);

    //�¶Ȼ�ȡ ���壨CPU���¶���Ҫ�жϻ�ȡ********************************************************************************
    if (sg_get_device_temperature_threshold(&dev_sta_rep_item) != VOS_OK) {
        return VOS_ERR;
    }

    sg_mqtttimestr(now_time, timestamp, sizeof(timestamp), TIME_UTC);           //ʹ��UTCʱ��
    if (timestamp != NULL) {
        memcpy_s(dev_sta_rep_item.devDateTime, DATA_BUF_F64_SIZE, timestamp, strlen(timestamp) + 1); //�豸��ǰʱ��
        printf("timestamp = %s \n", timestamp);
    }
    sg_get_devstdatetime(dev_sta_rep_item.devStDateTime, &dev_num); 	            //��ȡ�豸���һ������ʱ��
    printf("devStDateTime = %s \n", dev_sta_rep_item.devStDateTime);

    dev_sta_rep_item.devRunTime = (uint32_t)dev_num;                            //��ȡ�豸����ʱ��
    printf("devStDateTime = %u \n", dev_sta_rep_item.devRunTime);

    sgcc_get_links_info2(&dev_sta_rep_item.linkState, &dev_sta_rep_item.link_len);//��ȡlinks��Ϣ

    //����λ����Ϣ��γ�Ȼ�ȡ
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

    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_run_status_reply(code, mid, errormsg, &dev_sta_rep_item, item->msg_send);
    sg_push_pack_item(item);        //����
    return VOS_OK;
}

//�豸��Ϣ��ѯ����
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
    if (sg_get_monitor_temperature_threshold(&dev_info_inq_reply_item.temperature) != VOS_OK) {         //�¶Ȼ�ȡ
        printf("sg_get_monitor_temperature_threshold error! \n");
        ret = VOS_ERR;
    }
    if (sgcc_get_links_info(&dev_info_inq_reply_item.links, &dev_info_inq_reply_item.link_len) != VOS_OK) {//��ȡlinks��Ϣ
        printf("sgcc_get_links_info error! \n");
        ret = VOS_ERR;
    }
    if (sg_read_period_file(&dev_info_inq_reply_item.rep_period) != VOS_OK) {                   //���ڲ�����ȡ
        printf("sg_read_period_file error! \n");
        ret = VOS_ERR;
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

    sg_pack_dev_info_reply(code, mid, errormsg, &dev_info_inq_reply_item, item->msg_send);
    sg_push_pack_item(item);                                                    //����
    return ret;
}


//�豸��������޸�����
int sg_handle_dev_set_para_cmd(int32_t mid, dev_man_conf_command_s paraobj)    //paraobjΪ�������
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
    //��ȡ�ߵ�����ֵ(�������� ����ɾ��)
    temp_threshold_get = (temperature_threshold_status_s*)VOS_Malloc(MID_SGDEV, sizeof(temperature_threshold_status_s) *
        STORAGE_PARTITION_MAX_NUM);
    (void)memset_s(temp_threshold_get, MONITOR_TEMP_NUM * sizeof(temperature_threshold_status_s), 0, MONITOR_TEMP_NUM *
        sizeof(temperature_threshold_status_s));
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    temperature_status_call_get_monitor_threshold(&infoNum, temp_threshold_get, errmsg, SYSMAN_RPC_ERRMSG_MAX);  //�з���ֵ

    for (num = 0; num < infoNum; num++) {   //������Ҫ����"Main_board"�ԱȻ�ȡ
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

    //���øߵ�����ֵ
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

    //ʱ��������
    sg_set_period(&paraobj.rep_period);

    //�м��ִ��
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_set_para_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);        //����
    return VOS_OK;
}

//�豸ʱ��ͬ������
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
    datetime_action_call_set_sys_datetime_systohc(&systime, errmsg, DATA_BUF_F256_SIZE);   //����ʱ��
    sysman_rpc_transport_close();

    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        return VOS_ERR;
    }
    datetime_config_call_set_timezone(timeobj->timeZone, errmsg, DATA_BUF_F256_SIZE);       //����ʱ��
    sysman_rpc_transport_close();

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

    sg_pack_dev_set_time_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}

//�豸��־�ٻ�  ���Թ�
void sg_handle_dev_log_cmd(int32_t mid, dev_log_recall_s logobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    file_info_s fileinfo = { 0 };
    //�м��ִ��
    // logobj.url=			//û�ж�Ӧ�ӿ� ��Ҫ�Լ����
    // logobj.logType=

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

    sg_pack_dev_log_reply(code, mid, errormsg, &fileinfo, item->msg_send);
    sg_push_pack_item(item);
    //�豸��־�ٻ�Ӧ��
}

//�豸��������
int sg_handle_dev_ctrl_cmd(int32_t mid, char *action)
{
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    //��Ӧ��
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

    sg_pack_dev_ctrl_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    // �м��ִ��
    // �ӳ�5����񣬸�λԭ���û��ֶ����λ

    if (strncmp(action, "os-reboot", strlen("os-reboot")) == 0) {
        ret = reboot_action_call_reboot_reason(5, BOARD_MAIN, REBOOT_REASON_USER_CMD, errormsg, DATA_BUF_F256_SIZE);   //�����ն�ϵͳ
    } else if (strncmp(action, "edge-reboot", strlen("edge-reboot")) == 0) {
        sg_set_edge_reboot(REBOOT_EDGE_SET);            //�����ն����
    } else {
        return VOS_ERR;
    }
    return ret;
}

//������װ����  
void sg_handle_container_install_cmd(int32_t mid, container_install_cmd_s cmd_obj)
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

    if (strlen(cmd_obj.withAPP.version) != 0) {
        app_flag = true;

        item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
        (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
        sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

        sg_pack_container_install_cmd(code, mid, errormsg, item->msg_send);
        sg_push_pack_item(item);
        status.state = STATUS_EXE_DOWNLOAD;
        set_container_install_status(status);
        if (sg_file_download(cmd_obj.withAPP.file, statusobj.msg) != VOS_OK) {
            code = REQUEST_NOTEXITS;
            statusobj.code = REQUEST_NOTEXITS;
        }
    }
    status.state = STATUS_EXE_DOWNLOAD;
    set_container_install_status(status);

    if (sg_file_download(cmd_obj.image, statusobj.msg) != VOS_OK) {
        code = REQUEST_NOTEXITS;
        statusobj.code = REQUEST_NOTEXITS;
    }
    if (cmd_obj.policy > 0) {
        //����������
        return;
    }
    if (sg_container_install(&cmd_obj, statusobj.msg) != VOS_OK) {
        code = REQUEST_FAILED;
        statusobj.code = REQUEST_FAILED;
    }
    if (app_flag) {
        if (sg_container_with_app_install(&cmd_obj, statusobj.msg)) {            //��װָ��APP
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
    sprintf_s(sitem->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_data_pub());
    sg_pack_dev_install_result(statusobj, errormsg, sitem->msg_send);    //���ͽ��
    sg_push_pack_item(sitem);
}

//��������
int sg_handle_container_start_cmd(int32_t mid, char *container_name)
{
    bool result = true;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char errorcode[DATA_BUF_F256_SIZE];//ac�ã���ģ�鲻��
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    //�м��ִ��
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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_start_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}

//����ֹͣ
int sg_handle_container_stop_cmd(int32_t mid, char *container_name)
{
    bool result = true;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char errorcode[DATA_BUF_F256_SIZE];//ac�ã���ģ�鲻��
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_stop_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}

//����ж��
int sg_handle_container_delete_cmd(int32_t mid, char *container_name)
{
    bool result = true;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    char errorcode[DATA_BUF_F256_SIZE];//ac�ã���ģ�鲻��
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_remove_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}
/**
 * sg_get_container_modify_mount
 * ���ܣ���ȡmount����
 * ���룺container_conf_cmd_s *cmd_obj
 * ������container_mount_dir_add_s *mount_info
 * ����ֵ��int
 */
static int sg_get_container_modify_mount(container_conf_cmd_s *cmd_obj, container_mount_dir_add_s *mount_info)
{
    int ret                    = VOS_OK;
    int i                      = 0;
    char left_obj[DATA_BUF_F64_SIZE]  = { 0 };
    char right_obj[DATA_BUF_F64_SIZE] = { 0 };
    if (cmd_obj->mount_len > 0) {                                                                                        // mount��ֵ
        mount_info->num = cmd_obj->mount_len;
        mount_info->add_type = MOUNT_DIR_ADD_OVERWRITE;
        mount_info->mount_dir = (mount_dir_cfg_s*)VOS_Malloc(MID_SGDEV, sizeof(mount_dir_cfg_s) * cmd_obj->mount_len);   // �ǵ��ͷ�
        if (mount_info->mount_dir == NULL) {
            (void)printf("mount_info->mount_dir VOS_Malloc error!\n");
            return VOS_ERR;
        }
        (void)memset_s(mount_info->mount_dir, sizeof(mount_dir_cfg_s) * cmd_obj->mount_len, 0,
                sizeof(mount_dir_cfg_s) * cmd_obj->mount_len);
        for (i = 0; i < cmd_obj->mount_len; i++) {
            if (sg_str_colon(cmd_obj->mount[i], strlen(cmd_obj->mount[i]), left_obj, right_obj) != VOS_OK) {
                (void)printf("sg_str_colon error\n");
                return VOS_ERR;
            }
            sprintf_s(mount_info->mount_dir[i].host_dir, VM_PATH_MAX_128, "%s", left_obj);
            sprintf_s(mount_info->mount_dir[i].container_dir, VM_PATH_MAX_128, "%s", right_obj);
            printf("mount_info->mount_dir[%d].host_dir                  = %s\n", i, mount_info->mount_dir[i].host_dir);
            printf("mount_info->mount_dir[%d].container_dir             = %s\n", i, mount_info->mount_dir[i].container_dir);
        }
    }
    return ret;
}
/**
 * sg_get_container_modify_dev
 * ���ܣ���ȡdev����
 * ���룺container_conf_cmd_s *cmd_obj
 * ������install_map_dev *dev_info
 * ����ֵ��int
 */
static int sg_get_container_modify_dev(container_conf_cmd_s *cmd_obj, install_map_dev *dev_info)
{
    int ret                    = VOS_OK;
    int i                      = 0;
    char left_obj[DATA_BUF_F64_SIZE]  = { 0 };
    char right_obj[DATA_BUF_F64_SIZE] = { 0 };
    if (cmd_obj->dev_len > 0) {                                                                                          // dev��ֵ
        dev_info->dev_num = cmd_obj->dev_len;
        dev_info->dev_list = (install_dev_node*)VOS_Malloc(MID_SGDEV, 
                sizeof(install_dev_node) * cmd_obj->dev_len);                                                            // �ǵ��ͷ�
        if (dev_info->dev_list == NULL) {
            (void)printf("dev_info->dev_list VOS_Malloc error!\n");
            return VOS_ERR;
        }
        (void)memset_s(dev_info->dev_list, sizeof(install_dev_node) * cmd_obj->dev_len, 0,
                 sizeof(install_dev_node) * cmd_obj->dev_len);
        for (i = 0; i < cmd_obj->dev_len; i++) {
            if (sg_str_colon(cmd_obj->dev[i], strlen(cmd_obj->dev[i]), left_obj, right_obj) != VOS_OK) {
                (void)printf("sg_str_colon error\n");
                return VOS_ERR;
            }
            sprintf_s(dev_info->dev_list[i].host_dev, CONTAINER_DEV_NAME_MAX_LEN, "%s", left_obj);
            sprintf_s(dev_info->dev_list[i].map_dev, CONTAINER_DEV_NAME_MAX_LEN, "%s", right_obj);
            printf("dev_info->dev_list[%d].host_dev                     = %s\n", i, dev_info->dev_list[i].host_dev);
            printf("dev_info->dev_list[%d].map_dev                      = %s\n", i, dev_info->dev_list[i].map_dev);
        }
    }
    return ret;
}

// ���������޸Ĵ�����
static int sg_container_modify_north(container_conf_cmd_s *cmd_obj) 
{
    int ret                                  = VOS_OK;
    int cpus                                 = 0;
    int i                                    = 0;
    int container_len                        = 0;
    char left_obj[DATA_BUF_F64_SIZE]                = { 0 };
    char right_obj[DATA_BUF_F64_SIZE]               = { 0 };
    char errRet[VM_RET_STRING_SIZE]          = { 0 };
    INSTALL_OVA_PARA_NORTH_SG_S install_info = { 0 };

    container_len = strlen(cmd_obj->container) + 1;
    (void)printf("container_len = %d\n", container_len);
    install_info.container_name = (char*)VOS_Malloc(MID_SGDEV, sizeof(char) * container_len);
    if (install_info.container_name == NULL) {
        (void)printf("install_info.container_name = NULL\n");
        return VOS_ERR;
    }
    (void)memset_s(install_info.container_name, sizeof(char) * container_len, 0, sizeof(char) * container_len);
    memcpy_s(install_info.container_name, sizeof(char) * container_len, cmd_obj->container, strlen(cmd_obj->container));  // �������Ƹ�ֵ
    cpus = cmd_obj->cfgCpu.cpus;
    install_info.cpu_mask = 0;
    for (i = 0; i < cpus; i++) {
        install_info.cpu_mask |= i;                                                                                     // cpu������ֵ
    }
    install_info.cpu_threshold = cmd_obj->cfgCpu.cpuLmt;                                                                 // cpu�����ֵ��ֵ
    install_info.mem_size = cmd_obj->cfgMem.memory;                                                                      // �ڴ���ֵ��ֵ
    install_info.mem_threshold = cmd_obj->cfgMem.memLmt;                                                                 // �ڴ�����ֵ��ֵ
    install_info.disk_size = cmd_obj->cfgDisk.disk;                                                                      // �洢��ֵ��ֵ
    install_info.disk_threshold = cmd_obj->cfgDisk.diskLmt;                                                              // ���̴洢�����ֵ��ֵ

    if (strlen(cmd_obj->port) != 0) {                                                                                    // port��ֵ
        if (sg_str_colon(cmd_obj->port, strlen(cmd_obj->port), left_obj, right_obj) != VOS_OK) {
            (void)printf("sg_str_colon error\n");
            return VOS_ERR;
        }
        install_info.port_map.port_node[0].host_port = atoi(left_obj);
        install_info.port_map.port_node[0].container_port = atoi(right_obj);
    }
    printf("install_info.container_name                             = %s\n", install_info.container_name);
    printf("install_info.cpu_mask                                   = %lld\n", install_info.cpu_mask);
    printf("install_info.cpu_threshold                              = %d\n", install_info.cpu_threshold);
    printf("install_info.mem_size                                   = %d\n", install_info.mem_size);
    printf("install_info.mem_threshold                              = %d\n", install_info.mem_threshold);
    printf("install_info.disk_size                                  = %d\n", install_info.mem_size);
    printf("install_info.disk_threshold                             = %d\n", install_info.mem_size);
    printf("install_info.port_map.port_node[0].host_port            = %d\n", install_info.port_map.port_node[0].host_port);
    printf("install_info.port_map.port_node[0].container_port       = %d\n", install_info.port_map.port_node[0].container_port);

    if (sg_get_container_modify_mount(cmd_obj, &install_info.mount_dir_add) != VOS_OK) {                                 //��ȡmount
        (void)printf("sg_get_container_modify_mount error!\n");
        ret = VOS_ERR;
    }
    if (sg_get_container_modify_dev(cmd_obj, &install_info.dev_map) != VOS_OK) {                                         //��ȡdev
        (void)printf("sg_get_container_modify_dev error!\n");
        ret = VOS_ERR;
    }
    if (!vm_rpc_container_modify_north_sg(&install_info, errRet, VM_RET_STRING_SIZE)) {
        printf("vm_rpc_container_modify_north_sg error!\n");
        ret = VOS_ERR;
    }
    (void)VOS_Free(install_info.mount_dir_add.mount_dir);
    install_info.mount_dir_add.mount_dir = NULL;
    (void)VOS_Free(install_info.dev_map.dev_list);
    install_info.dev_map.dev_list = NULL;
    (void)VOS_Free(install_info.container_name);
    install_info.container_name = NULL;
    return ret;
}

//���������޸�
int sg_handle_container_param_set_cmd(int32_t mid, container_conf_cmd_s *cmd_obj)   //cmd_objΪ����
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char container_name[1024];
    char errormsg[DATA_BUF_F256_SIZE];
    container_alarm_type alarm_type = { 0 };
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
    //��Դ���ýӿڣ�����Ҫ����������

    if (sg_container_modify_north(cmd_obj) != VOS_OK) {
        printf("vm_rpc_container_status error!\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());
    sg_pack_container_param_set_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}

// ���޷��ų�����תΪcpu����  ��Ҫȷ��
static void sg_container_cpumask_to_cpucore(char *cpucore, unsigned len, long long cpu_mask)
{
    int cpu_core_len = 0;
    int i;

    for(i = 0; i < 64; i++) {
        if((unsigned long long)cpu_mask & 1) {
            cpu_core_len += sprintf_s(&cpucore[cpu_core_len], len - (unsigned int)cpu_core_len, "%d", i);
        }
        cpu_mask = (unsigned long long)cpu_mask >> 1;
    }
    if(cpu_core_len > 0) {
        cpucore[cpu_core_len -1] = 0;
    }
}
// ��ȡport
static int sg_get_port_map_info(container_conf_cmd_s *container_conf_cmd)
{
    int ret = VOS_OK;
    int port_num = 0;
    char errRet[VM_RET_STRING_SIZE]         = { 0 };
    port_map_node *port_info                = NULL;

    if(!vm_rpc_get_port_map_info(container_conf_cmd->container, &port_num, &port_info, errRet, VM_RET_STRING_SIZE)) {
        printf("vm_rpc_get_port_map_info error!\n");
        ret = VOS_ERR;
    }
    printf("port_num                                                       = %d\n", port_num);
    if (port_num <= 0) {
        printf("vm_rpc_get_port_map_info error!\n");
    } else {
        if(port_info == NULL) {
            printf("port_info NULL\n");
        } else {
            sprintf_s(container_conf_cmd->port, DATA_BUF_F64_SIZE, "%d:%d", 
                port_info[0].host_port, port_info[0].container_port);
            printf("container_conf_cmd->port = %s\n", container_conf_cmd->port);
        }
    }
    if (port_info) {
        (void)VOS_Free(port_info);
        port_info = NULL;
    }
    return ret;
}

// ��ȡĿ¼ӳ����Ϣ
static int sg_get_mount_dir(container_conf_cmd_s *container_conf_cmd)
{
    int i   = 0;
    int ret = VOS_OK;
    int mount_num = 0;
    char errRet[VM_RET_STRING_SIZE]       = { 0 };
    container_mount_dir_get_s *mount_info = NULL;

    if(!vm_rpc_get_mount_dir(container_conf_cmd->container, &mount_num, &mount_info, errRet, VM_RET_STRING_SIZE)) {
        printf("vm_rpc_get_mount_dir error!\n");
        ret = VOS_ERR;
    }
    printf("mount_num                                                       = %d\n", mount_num);
    container_conf_cmd->mount_len = mount_num;
    if (mount_num <= 0) {
        printf("vm_rpc_get_mount_dir error!\n");
    } else {
        if(mount_info == NULL) {
            printf("mount_info NULL\n");
        } else {
            for (i = 0; i < mount_num; i++) {
                sprintf_s(container_conf_cmd->mount[i], DATA_BUF_F64_SIZE, "%s:%s", mount_info[i].host_dir, mount_info[i].container_dir);
                printf("container_conf_cmd->mount[%d]     = %s\n", i, container_conf_cmd->mount[i]);
            }
        }
    }
    if (mount_info) {
        (void)VOS_Free(mount_info);
        mount_info = NULL;
    }
    return ret;
}

//��ȡ�豸�ڵ���Ϣ
static int sg_get_container_device_list(container_conf_cmd_s *container_conf_cmd)
{
    int i   = 0;
    int ret = VOS_OK;
    int dev_num = 0;
    char errRet[VM_RET_STRING_SIZE]   = { 0 };
    container_dev_node *dev_info      = NULL;

    if(!vm_status_call_get_container_device_list(container_conf_cmd->container, &dev_info, &dev_num, errRet, VM_RET_STRING_SIZE)) {
        printf("vm_status_call_get_container_device_list error!\n");
        ret = VOS_ERR;
    }
    printf("dev_num                                                       = %d\n", dev_num);
    container_conf_cmd->dev_len = dev_num;
    if (dev_num <= 0) {
        printf("vm_status_call_get_container_device_list error!\n");
    } else {
        if(dev_info == NULL) {
            printf("dev_info NULL\n");
        } else {
            for (i = 0; i < dev_num; i++) {
                sprintf_s(container_conf_cmd->dev[i], DATA_BUF_F64_SIZE, "%s:%s", dev_info[i].host_dev, dev_info[i].map_dev);
                printf("container_conf_cmd->dev[%d]     = %s\n", i, container_conf_cmd->dev[i]);
            }
        }
    }
    if (dev_info) {
        (void)VOS_Free(dev_info);
        dev_info = NULL;
    }
    return ret;
}

static int sg_container_status_to_reply(container_config_reply_s *contiainer_config_reply, int container_num, 
        CONTAINER_INFO_S *container_list)
{
    int num           = 0;
    int ret           = VOS_OK;
    char cpucore[128] = { 0 };

    contiainer_config_reply->contPara_len = container_num;
    contiainer_config_reply->contPara = (container_conf_cmd_s *)VOS_Malloc(MID_SGDEV,
            sizeof(container_conf_cmd_s) * container_num);
    if (contiainer_config_reply->contPara == NULL) {
        printf("malloc use failure\n");
        ret = VOS_ERR;
    }
    (void)memset_s(contiainer_config_reply->contPara, sizeof(container_conf_cmd_s)  * container_num, 0,
                                                         sizeof(container_conf_cmd_s) * container_num);
    for (num = 0; num < container_num; num++) {
        memcpy_s(contiainer_config_reply->contPara[num].container, DATA_BUF_F64_SIZE, container_list[num].container_name,
                strlen(container_list[num].container_name));                                                            // �������Ƹ�ֵ
        sg_container_cpumask_to_cpucore(cpucore, 128, (long long)container_list[num].cpu_mask);                         // 2021.1.23 �¼Ӳ���
        contiainer_config_reply->contPara[num].cfgCpu.cpus = sg_hamming_weight(container_list[num].cpu_mask);           // CPU������ֵ
        contiainer_config_reply->contPara[num].cfgCpu.cpuLmt = container_list[num].cpu_threshold;                       // CPU�����ֵ
        contiainer_config_reply->contPara[num].cfgMem.memory = container_list[num].container_mem_size;                  // �ڴ���ֵ��ֵ
        contiainer_config_reply->contPara[num].cfgMem.memLmt = container_list[num].memory_threshold;                    // �ڴ�����ֵ��ֵ
        contiainer_config_reply->contPara[num].cfgDisk.disk = container_list[num].container_storage_size;               // �洢��ֵ��ֵ
        contiainer_config_reply->contPara[num].cfgDisk.diskLmt = container_list[num].storage_threshold;                 // ���̴洢�����ֵ���ٷ���

        printf("contiainer_config_reply->contPara[%d].container        = %s\n", num, contiainer_config_reply->contPara[num].container);
        printf("cpucore                                                = %s\n", cpucore);
        printf("container_list[%d].cpu_mask                            = %lld\n", num, container_list[num].cpu_mask);
        printf("contiainer_config_reply->contPara[%d].cfgCpu.cpus      = %d\n", num, contiainer_config_reply->contPara[num].cfgCpu.cpus);
        printf("contiainer_config_reply->contPara[%d].cfgCpu.cpuLmt    = %d\n", num, contiainer_config_reply->contPara[num].cfgCpu.cpuLmt);
        printf("contiainer_config_reply->contPara[%d].cfgMem.memory    = %d\n", num, contiainer_config_reply->contPara[num].cfgMem.memory);
        printf("contiainer_config_reply->contPara[%d].cfgMem.memLmt    = %d\n", num, contiainer_config_reply->contPara[num].cfgMem.memLmt);
        printf("contiainer_config_reply->contPara[%d].cfgDisk.disk     = %d\n", num, contiainer_config_reply->contPara[num].cfgDisk.disk);
        printf("contiainer_config_reply->contPara[%d].cfgDisk.diskLmt  = %d\n", num, contiainer_config_reply->contPara[num].cfgDisk.diskLmt);

        if (sg_get_port_map_info(&contiainer_config_reply->contPara[num]) != VOS_OK) {
            printf("sg_get_port_map_info error!\n");
            ret = VOS_ERR;
        }

        if (sg_get_mount_dir(&contiainer_config_reply->contPara[num]) != VOS_OK) {
            printf("sg_get_mount_dir error!\n");
            ret = VOS_ERR;
        }

        if (sg_get_container_device_list(&contiainer_config_reply->contPara[num]) != VOS_OK) {
            printf("sg_get_container_device_list error!\n");
            ret = VOS_ERR;
        }
    }
    return ret;
}

//�������ò�ѯ���� 
int sg_handle_container_param_get(int32_t mid)
{
    int ret = VOS_OK;
    int container_num = 0;
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    CONTAINER_INFO_S *container_list = NULL;
    container_config_reply_s contiainer_config_reply = { 0 };
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    char errormsg[DATA_BUF_F256_SIZE];
    

    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    if (vm_rpc_container_status(&container_list, &container_num, NULL, errmsg, SYSMAN_RPC_ERRMSG_MAX) == false) {
        printf("vm_rpc_container_status error!\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    printf("container_num = %d\n", container_num);
    if (container_num == 0) {    //�Ƿ���������Ϊ��
        code = REQUEST_FAILED;
        if (container_list != NULL) {			//���ڴ��ͷŵ�
            VOS_Free(container_list);
            container_list = NULL;
            ret = VOS_ERR;
        }
    } else {                     //���������������
        if (sg_container_status_to_reply(&contiainer_config_reply, container_num, container_list) != VOS_OK) {
            code = REQUEST_FAILED;
            ret = VOS_ERR;
        }
        if (container_list != NULL) {			//���ڴ��ͷŵ�
            VOS_Free(container_list);
            container_list = NULL;
        }
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_param_get_reply(code, mid, errormsg, &contiainer_config_reply, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}

// ����״̬��ѯӦ�� ����״̬�ϱ�
int sg_handle_container_status_get(char *type, int32_t mid)
{
    int ret = VOS_OK;
    char *ptr = NULL;
    char errormsg[DATA_BUF_F256_SIZE] = { 0 };
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    char up_timestr[DATA_BUF_F256_SIZE] = { 0 };
    int num = 0;
    int container_cnt = 0;                          //���Ϊcontainer_info�ĸ���
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
    if (result != true) {                        //��ȡ��Ϣ�ɹ� ���ջ�ȡ������Ϣ
        printf("vm_rpc_container_status error!\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    if (container_cnt == 0) {
        code = REQUEST_FAILED;
        if (container_info != NULL) {			//���ڴ��ͷŵ�
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
                strlen(container_info[num].container_name));  //�������Ƹ�ֵ
            memcpy_s(status[num].version, DATA_BUF_F64_SIZE, container_info[num].container_version,
                strlen(container_info[num].container_version));  //�����汾��
            memcpy_s(status[num].state, DATA_BUF_F64_SIZE, container_info[num].container_status,
                strlen(container_info[num].container_status));  	//��������״̬�� running �� stopped
            memcpy_s(status[num].ip, DATA_BUF_F64_SIZE, container_info[num].container_ipaddr,
                strlen(container_info[num].container_ipaddr));  	//IP ��ַ���˿�
            status[num].cpuRate = container_info[num].cpuusage;                     //CPUռ���ʣ��ٷֱ�
            status[num].memUsed = container_info[num].memoryusage;                  //�ڴ�ռ���ʣ��ٷֱ�
            status[num].diskUsed = container_info[num].stroageusage;                //����ռ���ʣ��ٷֱ�

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

            //��ȡ���һ������ʱ�� ���ж�uptime_num�ǲ��ǵ���0
            uptime_num = strtoul(container_info[num].container_up_time, &ptr, 10);
            printf("uptime_num = %lu\n", uptime_num);
            if (uptime_num == 0) {
                printf("uptime_num get error!\n");
                code = REQUEST_FAILED;
                ret = VOS_ERR;
            }
            (void)localtime_r(&uptime_num, &tmp_ptr);
            (void)sprintf_s(status[num].started, DATA_BUF_F64_SIZE, "%d-%d-%d %d:%d:%d", 1900 + tmp_ptr.tm_year,  //��ȡ���һ������ʱ��
                1 + tmp_ptr.tm_mon, tmp_ptr.tm_mday, tmp_ptr.tm_hour, tmp_ptr.tm_min, tmp_ptr.tm_sec);
            printf("status[%d].started = %s\n", num, status[num].started);
            //��ȡ����ʱ�� ���ж�һ��status �ǲ��ǳ���running״̬  Ȼ���� ��ǰʱ���ȥ���һ������ʱ���������ʱ��
            if (strncmp(status[num].state, "running", strlen("running")) != 0) {
                status[num].lifeTime = 0;
            } else {
                up_time = time(NULL) - uptime_num;
                printf("up_time = %lu\n", up_time);
                status[num].lifeTime = up_time;                 //����ʱ�䣬��λ����
                printf("status[%d].lifeTime = %lu\n", num, status[num].lifeTime);
            }
            //status.created =  		//����ʱ��      ��ʱû��
            //status.image =    		//����������Ϣ  ��ʱû��
        }
        if (container_info != NULL) {			//���ڴ��ͷŵ�
            VOS_Free(container_info);
            container_info = NULL;
        }
    }
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

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

//��������  �Ⱥ���
void sg_handle_container_upgrade_cmd(int32_t mid, container_upgrade_cmd_s cmd_obj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //��Ӧ��

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_upgrade_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);

    //�м��ִ��
    // cmd_obj.jobId = 
    // cmd_obj.policy = 
    // cmd_obj.version = 
    // cmd_obj.file.name = 
    // cmd_obj.file.fileType = 
    // cmd_obj.file.url = 
    // cmd_obj.file.size = 
    // cmd_obj.file.md5 = 
    // cmd_obj.file.sign.name = 
    // cmd_obj.file.sign.url = 
    // cmd_obj.file.sign.size = 
    // cmd_obj.file.sign.md5 = 
}

//������־�ٻ�
int sg_handle_container_log_get_cmd(int32_t mid, container_log_recall_cmd_s *cmd_obj)
{
    int ret = VOS_OK;
    uint16_t code = REQUEST_SUCCESS;
    mqtt_data_info_s *item             = NULL;
    file_info_s fileinfo               = { 0 };
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    char errormsg[DATA_BUF_F256_SIZE]         = { 0 };
    char file_road[DATA_BUF_F64_SIZE]         = { 0 };

    (void)sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

/*   //�м��ִ��
    if(appm_rpc_transport_open() != VOS_OK) {
        ssp_syslog(LOG_ERR, SYSLOG_LOG, SGDEV_MODULE, "open appm failed!\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    ret = app_management_status_call_get_app_log(cmd_obj->container, "all", unsigned long start_time, unsigned long end_time, char *errmsg);  //ʱ�����õԹ����� ��ʼʱ��Ϊ������װʱ��   ����ʱ��Ϊ��ǰʱ��
    appm_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("app_management_status_call_get_app_info error! \n");
        ret = VOS_ERR;
    }
    (void)sprintf_s(fileinfo.name, DATA_BUF_F64_SIZE, "{%s}.log", cmd_obj->container);
//    fileinfo.fileType = 
    memcpy_s(fileinfo.url, DATA_BUF_F64_SIZE, cmd_obj->url, strlen(cmd_obj->url));
    (void)sprintf_s(file_road, DATA_BUF_F256_SIZE, "/run/shm/appm/{%s}/app_log_dir", cmd_obj->container);
    fileinfo.size = getfilesize(file_road);
    //MD5
        printf("sgdevagent**** : fileinfo.md5 = %s.\n", fileinfo.md5);
        sg_tolower(file.md5, strlen(file.md5));
    }
    //   fileinfo.sign = 
*/
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());
    sg_pack_container_log_get_reply(code, mid, errormsg, &fileinfo, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}

//Ӧ�ð�װ����
void sg_handle_app_install_cmd(int32_t mid, app_install_cmd_s cmd_obj)
{
    printf("sgdevagent**** : sg_handle_app_install_cmd  mid= %d.\n", mid);
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_WAIT;
    char errormsg[DATA_BUF_F256_SIZE];
    APPM_OPERATION_PARA para = { 0 };
    dev_status_reply_s status = { 0 };
    dev_upgrede_res_reply_s statusobj = { 0 };

    statusobj.jobId = cmd_obj.jobId;
    status.jobId = cmd_obj.jobId;
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    status.state = STATUS_PRE_DOWNLOAD;
    set_app_install_status(status);		//���ÿ�ʼ״̬

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_install_cmd(code, mid, errormsg, item->msg_send);    //�յ������ȷ���
    sg_push_pack_item(item);

    code = REQUEST_SUCCESS;
    status.state = STATUS_EXE_DOWNLOAD;
    set_app_install_status(status);		//������������
    printf("sgdevagent**** : sg_file_download  cmd_obj.file= %s.\n", cmd_obj.file);
    if (sg_file_download(cmd_obj.file, statusobj.msg) != VOS_OK) {
        code = REQUEST_NOTEXITS;
        statusobj.code = REQUEST_NOTEXITS;
    }
    status.state = STATUS_PRE_INSTALL;
    set_app_install_status(status);		//��ʼ��װ
    if (cmd_obj.policy > 0) { //��Ҫ����������߳��� �ؿ���ʱ�� 		//���̷߳�������
        return;
        VOS_T_Delay(cmd_obj.policy * 1000);
    }
    if (sg_app_install(cmd_obj, statusobj.msg) != VOS_OK) {
        code = REQUEST_REFUSE;
        statusobj.code = REQUEST_REFUSE;
    }
    printf("sgdevagent**** : sg_app_install  statusobj.msg= %s.\n", statusobj.msg);
    status.state = STATUS_FINISH_INSTALL;
    set_app_install_status(status);		//���ð�װ���
    mqtt_data_info_s *sitem = NULL;
    sitem = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(sitem, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(sitem->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_data_pub());
    sg_pack_dev_install_result(statusobj, errormsg, sitem->msg_send);    //���ͽ��
    sg_push_pack_item(sitem);
}
//Ӧ������
int sg_handle_app_start_cmd(int32_t mid, app_control_cmd_s *cmd_obj)
{
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];

    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmd_obj->container, strlen(cmd_obj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmd_obj->app, strlen(cmd_obj->app));
    //�м��ִ��
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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_start_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}
//Ӧ��ֹͣ
void sg_handle_app_stop_cmd(int32_t mid, app_control_cmd_s *cmd_obj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmd_obj->container, strlen(cmd_obj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmd_obj->app, strlen(cmd_obj->app));
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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_stop_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}
//Ӧ��ж��
void sg_handle_app_uninstall_cmd(int32_t mid, app_control_cmd_s *cmd_obj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmd_obj->container, strlen(cmd_obj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmd_obj->app, strlen(cmd_obj->app));
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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_uninstall_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}
//Ӧ��ʹ��
void sg_handle_app_enble_cmd(int32_t mid, app_control_cmd_s *cmd_obj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmd_obj->container, strlen(cmd_obj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmd_obj->app, strlen(cmd_obj->app));
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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_enable_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
}
//Ӧ��ȥʹ��
void sg_handle_app_unenble_cmd(int32_t mid, app_control_cmd_s *cmd_obj)
{
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
    memcpy_s(para.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmd_obj->container, strlen(cmd_obj->container));
    memcpy_s(para.app_name, APP_MANAGEMENT_APP_NAME_MAX_LEN + 1, cmd_obj->app, strlen(cmd_obj->app));
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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_unenble_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}


static int sg_appm_modify_app_info_cmd(APPM_OPERATION_PARA *para)
{
    appm_error_message err_msg = { 0 };
    (void)memset_s(&err_msg, sizeof(appm_error_message), 0, sizeof(appm_error_message));
    if(para == NULL || para->lxc_name == NULL || para->app_name == NULL) {
        return VOS_ERR;
    }

    if(appm_rpc_transport_open()) {
        (void)printf("Connect to rpc server error.\r\n");
        return VOS_ERR;
    }

    if (app_management_action_call_app_set_config(para, &err_msg)) {
        printf("app_management_action_call_app_set_config error!\n");
        return VOS_ERR;
    }
    appm_rpc_transport_close();
    return VOS_OK;
}


//Ӧ�������޸�
int sg_handle_app_param_set_cmd(int32_t mid, app_conf_cmd_s *cmd_obj)
{
    int ret = VOS_OK;
    int cpus = 0, i;
    uint16_t code = REQUEST_SUCCESS;

    char errormsg[DATA_BUF_F256_SIZE];
    APPM_OPERATION_PARA para_obj    = { 0 };
    mqtt_data_info_s *item          = NULL;
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    memcpy_s(para_obj.lxc_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmd_obj->container, strlen(cmd_obj->container));
    memcpy_s(para_obj.app_name, APP_MANAGEMENT_LXC_NAME_MAX_LEN + 1, cmd_obj->app, strlen(cmd_obj->app));
    cpus = cmd_obj->cfgCpu.cpus;
    para_obj.cpu_mask = 0;
    for (i = 0; i < cpus; i++) {
        para_obj.cpu_mask |= i;
    }
    para_obj.cpu_threshold = cmd_obj->cfgCpu.cpuLmt;
    para_obj.memory_limit = cmd_obj->cfgMem.memory;
    para_obj.memory_threshold = cmd_obj->cfgMem.memLmt;
    if (sg_appm_modify_app_info_cmd(&para_obj) != VOS_OK) {
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    printf("para_obj.lxc_name           = %s\n", para_obj.lxc_name);
    printf("para_obj.app_name           = %s\n", para_obj.app_name);
    printf("cmd_obj->cfgCpu.cpus         = %d\n", cmd_obj->cfgCpu.cpus);
    printf("para_obj.cpu_mask           = %lu\n", para_obj.cpu_mask);
    printf("para_obj.cpu_threshold      = %d\n", para_obj.cpu_threshold);
    printf("para_obj.memory_limit       = %d\n", para_obj.memory_limit);
    printf("para_obj.memory_threshold   = %d\n", para_obj.memory_threshold);

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    //�м��ִ��
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    sg_pack_app_param_set_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}
//Ӧ������״̬��ѯ����
void sg_handle_app_param_get(int32_t mid, char *container_name)
{
    int ret         = VOS_OK;
    int app_cnt     = 0;
    uint16_t num    = 0;
    uint16_t code = REQUEST_SUCCESS;
    char cpu_core[128]          = { 0 };
    char errormsg[DATA_BUF_F256_SIZE]  = { 0 };
    app_conf_reply_s statusobj  = { 0 };
    mqtt_data_info_s *item      = NULL;
    app_info_t *app_info        = NULL;

    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
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
    statusobj.app_num = app_cnt;
    memcpy_s(statusobj.container, DATA_BUF_F64_SIZE, container_name, strlen(container_name));
    printf("app_cnt = %d\n", app_cnt);
    printf("statusobj.container = %s\n", statusobj.container);

    statusobj.appCfgs = (app_cfgs_info_s*)VOS_Malloc(MID_SGDEV, sizeof(app_cfgs_info_s) * app_cnt);            //�ǵ��ͷ�
    if (statusobj.appCfgs == NULL) {
        printf("malloc use failure\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    (void)memset_s(statusobj.appCfgs, sizeof(app_cfgs_info_s) * app_cnt, 0, sizeof(app_cfgs_info_s) * app_cnt);
    for (num = 0; num < app_cnt; num++) { 
        memcpy_s(statusobj.appCfgs[num].app, DATA_BUF_F64_SIZE, app_info[num].name, strlen(app_info[num].name));
        printf("statusobj.appCfgs[%d].app = %s\n", num, statusobj.appCfgs[num].app);
        

        statusobj.appCfgs[num].cfgCpu.cpus = sg_hamming_weight(app_info[num].services->cpu_mask);
        printf("******cpu_core*****  = %s\n", cpu_core);
        printf("******statusobj.appCfgs[%d].cfgCpu.cpus*****  = %d\n",num, statusobj.appCfgs[num].cfgCpu.cpus);
        statusobj.appCfgs[num].cfgCpu.cpuLmt = app_info[num].services->cpu_usage_threshold;
        statusobj.appCfgs[num].cfgMem.memory = app_info[num].services->memory_usage_current;
        statusobj.appCfgs[num].cfgMem.memLmt = app_info[num].services->memory_usage_threshold;
        printf("******statusobj.appCfgs[%d].cfgCpu.cpuLmt*****  = %d\n",num, statusobj.appCfgs[num].cfgCpu.cpuLmt);
        printf("******statusobj.appCfgs[%d].cfgMem.memory*****  = %d\n",num, statusobj.appCfgs[num].cfgMem.memory);
        printf("******statusobj.appCfgs[%d].cfgMem.memLmt*****  = %d\n",num, statusobj.appCfgs[num].cfgMem.memLmt);
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_param_get_reply(code, mid, errormsg, &statusobj, item->msg_send);
    VOS_Free(statusobj.appCfgs); //�ͷ�
    statusobj.appCfgs = NULL;
    sg_push_pack_item(item);
}

//Ӧ��״̬��ѯ����
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
    //�м��ִ��
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
    statusobj.apps = (apps_info_s*)VOS_Malloc(MID_SGDEV, sizeof(apps_info_s) * app_cnt);            //�ǵ��ͷ�
    if (statusobj.apps == NULL) {
        printf("malloc use failure\n");
        code = REQUEST_FAILED;
        ret = VOS_ERR;
    }
    (void)memset_s(statusobj.apps, sizeof(apps_info_s) * app_cnt, 0, sizeof(apps_info_s) * app_cnt);
    for (num = 0; num < app_cnt; num++) {  //���������ж��ٸ�APP��ÿ��APP�����Ӧ�ľ��Ƕ��ٸ�����
        process_count = 0;
        memcpy_s(statusobj.apps[num].app, DATA_BUF_F64_SIZE, app_info[num].name, strlen(app_info[num].name));          //APP ����
        memcpy_s(statusobj.apps[num].version, DATA_BUF_F32_SIZE, app_info[num].version, strlen(app_info[num].version));//APP �汾��
        memcpy_s(statusobj.apps[num].appHash, DATA_BUF_F64_SIZE, app_info[num].hash, strlen(app_info[num].hash)); 		//APP �Ĺ�ϣֵ

        printf("statusobj.apps[%d].app = %s\n", num, statusobj.apps[num].app);
        printf("statusobj.apps[%d].version = %s\n", num, statusobj.apps[num].version);
        printf("statusobj.apps[%d].appHash = %s\n", num, statusobj.apps[num].appHash);

        //��Ҫ����ʵ�ʽ��������ǲ������������жϻ�ȡ������
        for (i = 0; i < APP_MANAGEMENT_APP_SERVICE_MAX; i++) {
            if (strlen(app_info[num].services[i].name) != 0) {
                process_count++;
            } else {
                break;
            }
        }
        statusobj.apps[num].srvNumber = process_count;                                              //��ǰ APP �Ľ�������
        printf("statusobj.apps[%d].srvNumber = %d\n", num, process_count);

        statusobj.apps[num].process = (process_info_s*)VOS_Malloc(MID_SGDEV, sizeof(process_info_s) * process_count);            //�ǵ��ͷ�
        if (statusobj.apps[num].process == NULL) {
            printf("malloc use failure\n");
            code = REQUEST_FAILED;
            ret = VOS_ERR;
        }
        (void)memset_s(statusobj.apps[num].process, sizeof(process_info_s) * process_count, 0, sizeof(process_info_s) * process_count);

        for (num_process = 0; num_process < process_count; num_process++) {                      //������Ϣ��ȡ
            statusobj.apps[num].process[num_process].srvIndex = num_process;                   //��������
            memcpy_s(statusobj.apps[num].process[num_process].srvName, DATA_BUF_F64_SIZE, app_info[num].services[num_process].name, strlen(app_info[num].services[num_process].name)); //��������
            if(app_info[num].services[num_process].enable == 1) {   // ����ʹ��״̬�� yes �� no
                memcpy_s(statusobj.apps[num].process[num_process].srvEnable, DATA_BUF_F64_SIZE, "yes", strlen("yes"));
            } else {
                memcpy_s(statusobj.apps[num].process[num_process].srvEnable, DATA_BUF_F64_SIZE, "no", strlen("no"));
            }
            if(app_info[num].services[num_process].status == 0) {   // ����״̬�� running �� stopped
                memcpy_s(statusobj.apps[num].process[num_process].srvStatus, DATA_BUF_F64_SIZE, "running", strlen("running"));
            } else {
                memcpy_s(statusobj.apps[num].process[num_process].srvStatus, DATA_BUF_F64_SIZE, "stopped", strlen("stopped"));
            }
            statusobj.apps[num].process[num_process].cpuLmt = app_info[num].services[num_process].cpu_usage_threshold; 		//CPU �����ֵ���ٷֱ�����
            statusobj.apps[num].process[num_process].cpuRate = app_info[num].services[num_process].cpu_usage_current;		//��ǰ CPU ʹ���ʣ��ٷֱ�����
            statusobj.apps[num].process[num_process].memLmt = app_info[num].services[num_process].memory_usage_threshold;	//�ڴ�����ֵ���ٷֱ�����
            statusobj.apps[num].process[num_process].memUsed = app_info[num].services[num_process].memory_usage_current;	//��ǰ�ڴ�ʹ�ÿռ�Ĵ�С���ٷֱ�����
            sprintf_s(statusobj.apps[num].process[num_process].startTime, DATA_BUF_F64_SIZE, "%04d-%02d-%02d %02d:%02d:%02d",
                app_info[num].services[num_process].start_time.year, app_info[num].services[num_process].start_time.month,
                app_info[num].services[num_process].start_time.day, app_info[num].services[num_process].start_time.hour,
                app_info[num].services[num_process].start_time.minute, app_info[num].services[num_process].start_time.second);  //��ʾ��������ʱ��

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
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_status_get_reply(code, mid, errormsg, &statusobj, item->msg_send);
    VOS_Free(statusobj.apps[num].process);
    statusobj.apps[num].process = NULL;
    VOS_Free(statusobj.apps); //�ͷ�
    statusobj.apps = NULL;
    sg_push_pack_item(item);

}
//Ӧ������  
void sg_handle_app_upgrade_cmd(int32_t mid, app_upgrade_cmd_s cmd_obj)
{
    printf("sgdevagent**** : sg_handle_app_upgrade_cmd  mid= %d.\n", mid);
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_WAIT;
    char errormsg[DATA_BUF_F256_SIZE];
    APPM_OPERATION_PARA para = { 0 };
    dev_status_reply_s status = { 0 };
    dev_upgrede_res_reply_s statusobj = { 0 };

    statusobj.jobId = cmd_obj.jobId;
    status.jobId = cmd_obj.jobId;
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");

    status.state = STATUS_PRE_DOWNLOAD;
    set_app_upgrade_status(status);		//���ÿ�ʼ״̬

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_upgrade_cmd(code, mid, errormsg, item->msg_send);    //�յ������ȷ���
    sg_push_pack_item(item);

    code = REQUEST_SUCCESS;
    status.state = STATUS_EXE_DOWNLOAD;
    set_app_upgrade_status(status);		//������������
    printf("sgdevagent**** : sg_file_download  cmd_obj.file= %s.\n", cmd_obj.file);
    if (sg_file_download(cmd_obj.file, statusobj.msg) != VOS_OK) {
        code = REQUEST_NOTEXITS;
        statusobj.code = REQUEST_NOTEXITS;
    }
    status.state = STATUS_PRE_INSTALL;
    set_app_upgrade_status(status);		//��ʼ��װ
    if (cmd_obj.policy > 0) { //��Ҫ����������߳��� �ؿ���ʱ�� 		//���̷߳�������
        return;
        VOS_T_Delay(cmd_obj.policy * 1000);
    }
    if (sg_app_update(cmd_obj, statusobj.msg) != VOS_OK) {
        code = REQUEST_REFUSE;
        statusobj.code = REQUEST_REFUSE;
    }
    printf("sgdevagent**** : sg_app_install  statusobj.msg= %s.\n", statusobj.msg);
    status.state = STATUS_FINISH_INSTALL;
    set_app_install_status(status);		//���ð�װ���
    mqtt_data_info_s *sitem = NULL;
    sitem = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(sitem, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(sitem->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_data_pub());
    sg_pack_dev_install_result(statusobj, errormsg, sitem->msg_send);    //���ͽ��
    sg_push_pack_item(sitem);
 
}

//Ӧ����־��ѯ
void sg_handle_app_log_get_cmd(int32_t mid, app_log_recall_cmd_s cmd_obj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    file_info_s fielinfo;

    //�м��ִ��
    // cmd_obj.container = 
    // cmd_obj.url = 
    // cmd_obj.app = 

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pub_topic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_log_get_reply(code, mid, errormsg, &fielinfo, item->msg_send);
    sg_push_pack_item(item);
}
