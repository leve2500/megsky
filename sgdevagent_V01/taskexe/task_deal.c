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
    cpuusage_s      cpuoutput_value;                      //CPU ���أ����� 50 ��ʾ 50%��
    memoryusage_s   memoutput_value;                      //�ڴ�ʹ�����
}dev_usage_status_s;

//�豸��������  �Թ� δ�ӷ���ֵ
int sg_handle_dev_install_cmd(int32_t mid, device_upgrade_s cmdobj)
{
    dev_status_reply_s status;
    status.jobId = cmdobj.jobId;
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

    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
    sg_pack_dev_install_cmd(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);        //�ش�����

    //����ǩ���ļ�

    //���������ļ�

    //�жϲ�������
    if (cmdobj.upgradeType == UPGRADEPACH) {
        sg_down_update_patch(cmdobj, errormsg);
    } else if (cmdobj.upgradeType == UPGRADEHOST) {
        sg_down_update_host(cmdobj, errormsg);
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

    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
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

    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());
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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_device_reply_pub());

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
        //����������
        return;
    }
    if (sg_container_install(&cmdobj, statusobj.msg) != VOS_OK) {
        code = REQUEST_FAILED;
        statusobj.code = REQUEST_FAILED;
    }
    if (app_flag) {
        if (sg_container_with_app_install(&cmdobj, statusobj.msg)) {            //��װָ��APP
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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_remove_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return VOS_OK;
}

//���������޸�
void sg_handle_container_param_set_cmd(int32_t mid, container_conf_cmd_s *cmdobj)   //cmdobjΪ����
{
    gboolean ret = 0;
    INSTALL_OVA_PARA_NORTH_S para = { 0 };
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char container_name[1024];
    char errormsg[DATA_BUF_F256_SIZE];
    container_alarm_type alarm_type = { 0 };
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
    //��Դ���ýӿڣ�����Ҫ����������


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

    // ��ֵ���ýӿڣ�����Ҫ����������
    // vm_rpc_set_container_alarm_threshold(container_name, alarm_type,  unsigned int threshold_size, char *errRet, size_t errLen);   //2020.11.18����  �б���������  ԭ�򣺿���û�����ͷ�ļ�

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

//�������ò�ѯ���� 
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
    if (container_cnt == 0) {    //�Ƿ���������Ϊ��
        code = REQUEST_FAILED;
        if (container_info != NULL) {			//���ڴ��ͷŵ�
            VOS_Free(container_info);
            container_info = NULL;
        }
    } else {                     //���������������
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
                strlen(container_info[num].container_name));  //�������Ƹ�ֵ
            contiainer_config_reply.contPara[num].cfgCpu.cpus = sg_hamming_weight(container_info[num].cpu_mask);              //CPU������ֵ
            printf("container_info[%d].cpu_mask = %lld\n", num, container_info[num].cpu_mask);

            contiainer_config_reply.contPara[num].cfgCpu.cpuLmt = container_info[num].cpu_threshold;                          //CPU�����ֵ
            contiainer_config_reply.contPara[num].cfgMem.memory = container_info[num].container_mem_size;                     //�ڴ���ֵ��ֵ
            contiainer_config_reply.contPara[num].cfgMem.memLmt = container_info[num].memory_threshold;                       //�ڴ�����ֵ��ֵ
            contiainer_config_reply.contPara[num].cfgDisk.disk = container_info[num].container_storage_size;                  //�洢��ֵ��ֵ
            contiainer_config_reply.contPara[num].cfgDisk.diskLmt = container_info[num].storage_threshold;                    //���̴洢�����ֵ���ٷ���

            printf("contiainer_config_reply.contPara[%d].cfgCpu.cpus = %d\n", num, contiainer_config_reply.contPara[num].cfgCpu.cpus);
            printf("contiainer_config_reply.contPara[%d].cfgCpu.cpuLmt = %d\n", num, contiainer_config_reply.contPara[num].cfgCpu.cpuLmt);
            printf("contiainer_config_reply.contPara[%d].cfgMem.memory = %d\n", num, contiainer_config_reply.contPara[num].cfgMem.memory);
            printf("contiainer_config_reply.contPara[%d].cfgMem.memLmt = %d\n", num, contiainer_config_reply.contPara[num].cfgMem.memLmt);
            printf("contiainer_config_reply.contPara[%d].cfgDisk.disk = %d\n", num, contiainer_config_reply.contPara[num].cfgDisk.disk);
            printf("contiainer_config_reply.contPara[%d].cfgDisk.diskLmt = %d\n", num, contiainer_config_reply.contPara[num].cfgDisk.diskLmt);
            // contiainer_config_reply.contPara[num].port =                                //�˿���Դ���ò���

        }
        if (container_info != NULL) {			//���ڴ��ͷŵ�
            VOS_Free(container_info);
            container_info = NULL;
        }
    }

    // sysman_rpc_transport_close();
    //�м��ִ��
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

//��������  �Ⱥ���
void sg_handle_container_upgrade_cmd(int32_t mid, container_upgrade_cmd_s cmdobj)
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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());

    sg_pack_container_upgrade_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);

    //�м��ִ��
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

//������־�ٻ�   ���Թ�
void sg_handle_container_log_get_cmd(int32_t mid, container_log_recall_cmd_s cmdobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    file_info_s fileinfo;
    //�м��ִ��

    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_container_reply_pub());
    sg_pack_container_log_get_reply(code, mid, errormsg, &fileinfo, item->msg_send);
    sg_push_pack_item(item);
}

//Ӧ�ð�װ����
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
    set_app_install_status(status);		//���ÿ�ʼ״̬

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_install_cmd(code, mid, errormsg, item->msg_send);    //�յ������ȷ���
    sg_push_pack_item(item);

    code = REQUEST_SUCCESS;
    status.state = STATUS_EXE_DOWNLOAD;
    set_app_install_status(status);		//������������
    printf("sgdevagent**** : sg_file_download  cmdobj.file= %s.\n", cmdobj.file);
    if (sg_file_download(cmdobj.file, statusobj.msg) != VOS_OK) {
        code = REQUEST_NOTEXITS;
        statusobj.code = REQUEST_NOTEXITS;
    }
    status.state = STATUS_PRE_INSTALL;
    set_app_install_status(status);		//��ʼ��װ
    if (cmdobj.policy > 0) { //��Ҫ����������߳��� �ؿ���ʱ�� 		//���̷߳�������
        return;
        VOS_T_Delay(cmdobj.policy * 1000);
    }
    if (sg_app_install(cmdobj, statusobj.msg) != VOS_OK) {
        code = REQUEST_REFUSE;
        statusobj.code = REQUEST_REFUSE;
    }
    printf("sgdevagent**** : sg_app_install  statusobj.msg= %s.\n", statusobj.msg);
    status.state = STATUS_FINISH_INSTALL;
    set_app_install_status(status);		//���ð�װ���
    mqtt_data_info_s *sitem = NULL;
    sitem = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(sitem, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(sitem->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_data_pub());
    sg_pack_dev_install_result(statusobj, errormsg, sitem->msg_send);    //���ͽ��
    sg_push_pack_item(sitem);
}
//Ӧ������
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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_start_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    return ret;
}
//Ӧ��ֹͣ
void sg_handle_app_stop_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
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
//Ӧ��ж��
void sg_handle_app_uninstall_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
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
//Ӧ��ʹ��
void sg_handle_app_enble_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
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
//Ӧ��ȥʹ��
void sg_handle_app_unenble_cmd(int32_t mid, app_control_cmd_s *cmdobj)
{
    int ret = 0;
    mqtt_data_info_s *item = NULL;
    APPM_OPERATION_PARA  para = { 0 };
    appm_error_message errmsg = { 0 };
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
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

//Ӧ�������޸�     ��֧�� ���Թ�
void sg_handle_app_param_set_cmd(int32_t mid, app_conf_cmd_s cmdobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    //�м��ִ��
    if (code != REQUEST_SUCCESS) {
        sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "error");
    }
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_param_set_reply(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
}
//Ӧ������״̬��ѯ����
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
    //�м��ִ��
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
        memcpy_s(statusobj.container, DATA_BUF_F64_SIZE, lxc_name, strlen(lxc_name) + 1);  //�������Ƹ�ֵ
    }
    for (num = 0; num < statusobj.app_num; num++) {
        statusobj.appCfgs[num].cfgCpu.cpus = info->services[num].cpu_usage_current;						//CPU����������ֵΪ2,3,4��
        statusobj.appCfgs[num].cfgCpu.cpuLmt = info->services[num].cpu_usage_threshold; 					//CPU�����ֵ
        statusobj.appCfgs[num].cfgMem.memory = info->services[num].memory_usage_current;					//�ڴ���ֵ,��λ�� M Byte
        statusobj.appCfgs[num].cfgMem.memLmt = info->services[num].memory_usage_threshold; 					//�ڴ�����ֵ���ٷ���
        if (NULL != info->services[num].name) {
            memcpy_s(statusobj.appCfgs[num].app, DATA_BUF_F64_SIZE, info->services[num].name, strlen(info->services[num].name) + 1); 	//Ӧ���ļ�����
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
            sprintf_s(statusobj.apps[num].process[num_process].startTime, DATA_BUF_F64_SIZE, "%d-%d-%d %d:%d:%d",
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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());
    sg_pack_app_status_get_reply(code, mid, errormsg, &statusobj, item->msg_send);
    VOS_Free(statusobj.apps[num].process);
    statusobj.apps[num].process = NULL;
    VOS_Free(statusobj.apps); //�ͷ�
    statusobj.apps = NULL;
    sg_push_pack_item(item);

}
//Ӧ������  ���Թ�
void sg_handle_app_upgrade_cmd(int32_t mid, app_upgrade_cmd_s cmdobj)
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
    sprintf_s(item->pubtopic, DATA_BUF_F256_SIZE, "%s", get_topic_app_reply_pub());

    sg_pack_app_upgrade_cmd(code, mid, errormsg, item->msg_send);
    sg_push_pack_item(item);
    //�м��ִ��
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

//Ӧ����־��ѯ
void sg_handle_app_log_get_cmd(int32_t mid, app_log_recall_cmd_s cmdobj)
{
    mqtt_data_info_s *item = NULL;
    uint16_t code = REQUEST_SUCCESS;
    char errormsg[DATA_BUF_F256_SIZE];
    sprintf_s(errormsg, DATA_BUF_F256_SIZE, "%s", "command success");
    file_info_s fielinfo;

    //�м��ִ��
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
