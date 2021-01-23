#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <sys/sysinfo.h>
#include <sys/time.h>               //ϵͳ��

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"
#include "ssp_mid.h"
#include "ssp_syslog.h"
#include "net_common.h"
#include "ifm_status.h"
#include "vm_public.h"
#include "sysman_devinfo_def.h"
#include "sysman_rpc_api.h"

#include "sgdev_struct.h"
#include "sgdev_debug.h"
#include "sgdev_param.h"
#include "task_link.h"               //�Լ���
#include "mqtt_json.h"
#include "mqtt_dev.h"

int edge_reboot_flag = REBOOT_EDGE_RESET;


//��ȡ�ն�����
static int sg_get_devname(char *devName)
{
    int pos = 0;
    char Temp_str[DATA_BUF_F64_SIZE];
    if (sg_file_common_get(HOSTNAME_FILE, Temp_str) == VOS_OK) {
        //��������
        pos = sg_find(Temp_str, strlen(Temp_str), "\n", 1, 0);
        if (pos > 0) {
            if (sg_str_left(Temp_str, strlen(Temp_str), devName, pos) > 0) {
                printf("devName in = %s\n", devName);
            } else {
                printf("sg_str_left error!\n");
                return VOS_ERR;
            }
        } else {
            printf("common_get pos error!\n");
            return VOS_ERR;
        }
    } else {
        printf("common_get devName error!\n");
        return VOS_ERR;
    }
    return VOS_OK;
}

//��ȡ�ն�����
static int sg_get_devtype(char *devType)
{
    int pos = 0;
    char Temp_str[1500];
    if (sg_read_file_get(DEVICE_TYPE_FILE, Temp_str) == VOS_OK) {
        pos = sg_find(Temp_str, strlen(Temp_str), "XXX", 1, 0);
        if (pos > 0) {
            if (sg_str_mid(Temp_str, strlen(Temp_str), pos - 3, devType, 3)) {
                printf("devType in = %s\n", devType);
            } else {
                printf("sg_str_mid error!\n");
                return VOS_ERR;
            }
        } else {
            printf("common_get pos error!\n");
            return VOS_ERR;
        }
    } else {
        printf("common_get devType error!\n");
        return VOS_ERR;
    }
    return VOS_OK;
}
//��ȡ�ն�Ӳ���汾��
static int sg_get_hardware_version(char *Version)
{
    int pos = 0;
    char Temp_str[DATA_BUF_F64_SIZE];
    if (sg_file_common_get(DEVICE_HARDWARE_VERSION, Temp_str) == VOS_OK) {
        pos = sg_find(Temp_str, strlen(Temp_str), "H", 1, 0);
        if (pos > 0) {
            if (sg_str_mid(Temp_str, strlen(Temp_str), pos, Version, 7)) {
                printf("Version in = %s\n", Version);
            } else {
                printf("sg_str_mid error!\n");
                return VOS_ERR;
            }
        } else {
            printf("common_get pos error!\n");
            return VOS_ERR;
        }
    } else {
        printf("common_get Version error!\n");
        return VOS_ERR;
    }
    return VOS_OK;
}

//��ȡ�豸��Ϣ�ֶ�
int sg_get_dev_devinfo(dev_info_s *info)
{
    int pos = 0;
    char Temp_str[DATA_BUF_F64_SIZE];
    if (info == NULL) {
        printf("\nsg_get_dev_devinfo :info is NULL.\n");
        return VOS_ERR;
    }
    if (sg_get_devname(info->devName) == VOS_OK) {                              //��ȡ�ն�����
        printf("devName out = %s\n", info->devName);
        ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "syslog_info->devName = %s\n", info->devName);
    } else {
        printf("get_devName error!\n");
        return VOS_ERR;
    }
    if (sg_get_devtype(info->devType) == VOS_OK) {                              //��ȡ�ն����� �ն�IDǰ�����ֽ�
        printf("devType out = %s\n", info->devType);
        ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "syslog_info->devType = %s\n", info->devType);
    } else {
        printf("get_devType error!\n");
        return VOS_ERR;
    }
    if (sg_file_common_get(VENDOR_FILE, info->mfgInfo) == VOS_OK) {              //��ȡ�ն˳�����Ϣ
        printf("info->mfgInfo = %s \n", info->mfgInfo);
        ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "syslog_info->mfgInfo = %s\n", info->mfgInfo);
    } else {
        printf("common_get mfgInfo error!\n");
        return VOS_ERR;
    }
    sprintf_s(info->devStatus, DATA_BUF_F64_SIZE, "%s", "online");                     //��ȡ�ն�״̬ ??
    ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "syslog_info->devStatus = %s\n", info->devStatus);

    if (sg_get_hardware_version(info->hardVersion) == VOS_OK) {                 //��ȡ�ն�Ӳ���汾��
        printf("Version out = %s\n", info->hardVersion);
        ssp_syslog(LOG_INFO, SYSLOG_LOG, SGDEV_MODULE, "syslog_info->hardVersion = %s\n", info->hardVersion);
    } else {
        printf("get_Version error!\n");
        memcpy(info->hardVersion, "HV01.01", strlen("HV01.01"));                //���û�о�Ĭ��Ϊ"HV01.01"
        printf("Version out = %s\n", info->hardVersion);
        return VOS_ERR;
    }
    return VOS_OK;
}


int sg_get_cpu_devinfo(cpu_info_s *info)
{
    FILE *fp;
    int ret = VOS_OK;
    char buf[DEV_INFO_MSG_BUFFER_LEN] = { 0 };
    char dstbuf[DEV_INFO_MSG_BUFFER_LEN] = { 0 };
    int frequency = 1000;
    int dstbuflen = 0;
    struct sysinfo si;
    cpuusage_s output_value = { 0 };
    char errmsg[DATA_BUF_F256_SIZE] = { 0 };
    //cpu����
    info->cpus = sysconf(_SC_NPROCESSORS_ONLN);
    //cpu��Ƶ
    if ((fp = fopen(CPU_FREQUENCY_PATH, "r")) == NULL) {
        printf("\nfile open cpu devinfo failed.\n");
        return VOS_ERR;
    }
    while (!feof(fp))
    {
        fgets(buf, DEV_INFO_MSG_BUFFER_LEN, fp);
    }
    fclose(fp);
    dstbuflen = sg_str_right(buf, strlen(buf), dstbuf, strlen(buf) - strlen("cpu MHz :"));
    if (dstbuflen > 0) {
        frequency = atoi(dstbuf);
    }
    info->frequency = frequency / 1024.0;
    //cpu����
    sysinfo(&si);
    info->cache = si.bufferram / (1024 * 1024);
    //cpu�澯��ֵ
    if (sysman_rpc_transport_open() == VOS_OK) {
        ret = cpuusage_status_call_get_threshold(&output_value, errmsg, DATA_BUF_F256_SIZE);
    }
    sysman_rpc_transport_close();
    if (ret == VOS_OK) {
        info->cpuLmt = output_value.warning;    // �澯��ֵȡ output_value.warning���澯��ֵ�ָ�ȡ output_value.warning - 10
    }
    //cpu�ܹ�
    if (sg_cmd_common_get("uname -m", info->arch) == VOS_OK) {
        return VOS_OK;
    }
    return ret;
}
int sg_get_mem_devinfo(mem_info_s *info)
{
    int ret = VOS_OK;
    struct sysinfo si;
    memoryusage_s output_value = { 0 };
    char errmsg[DATA_BUF_F256_SIZE] = { 0 };
    //�����ڴ棬��MΪ��λ 
    info->virt = sg_memvirt_total();
    //�����ڴ棬��MΪ��λ
    sysinfo(&si);
    info->phy = si.totalram / (1024 * 1024);

    //�ڴ�����ֵ������50��ʾ50% 
    if (sysman_rpc_transport_open() == VOS_OK) {
        ret = memoryusage_status_call_get_threshold(&output_value, errmsg, DATA_BUF_F256_SIZE);
    }
    sysman_rpc_transport_close();

    info->memLmt = output_value.warning;
    return ret;
}
int sg_get_disk_devinfo(disk_info_s *info)
{
    int ret = VOS_OK;
    int pos = 0;
    char buf[DATA_BUF_F256_SIZE] = { 0 };
    char dstbuf[DATA_BUF_F256_SIZE] = { 0 };
    storageusage_s* output_value = NULL;
    char errmsg[DATA_BUF_F256_SIZE] = { 0 };
    uint32_t infoNum = 0;
    int freeRet = 0;

    if (sg_file_common_get(STORAGE_SIZE_FILE, buf) == VOS_OK) {    // #define STORAGE_SIZE_FILE "/etc/devinfo/storage-size"
        pos = sg_find(buf, strlen(buf), " M", 1, 0);
        if (pos > 0) {
            if (sg_str_left(buf, strlen(buf), dstbuf, pos) > 0) {
                info->disk = atoi(dstbuf);
            } else {
                return VOS_ERR;
            }
        } else {
            return VOS_ERR;
        }
    } else {
        return VOS_ERR;
    }
    output_value = (storageusage_s*)VOS_Malloc(MID_SGDEV, sizeof(storageusage_s) * STORAGE_PARTITION_MAX_NUM);
    if (sysman_rpc_transport_open() == VOS_OK) {
        ret = storageusage_status_call_get_threshold(&infoNum, output_value, errmsg, DATA_BUF_F256_SIZE);
    }
    sysman_rpc_transport_close();
    if (output_value != NULL) {
        freeRet = VOS_Free(output_value);
        if (freeRet != VOS_OK) {
            printf("\n output_value fail");
        }
    }
    info->diskLmt = output_value[0].warning;
    return ret;
}
int sg_get_os_devinfo(os_info_s *info)    //2020.12.15 ��飺��Ҫ����
{
    // char            distro[DATA_BUF_F64_SIZE];        //����ϵͳ�����ƣ���"Ubuntu""Redhat"
    // char            version[DATA_BUF_F64_SIZE];       //����ϵͳ�汾����"18.10"
    // char            kernel[DATA_BUF_F64_SIZE];        //����ϵͳ�ںˣ���"3.10-17"
    // char            softVersion[DATA_BUF_F64_SIZE];   //ƽ̨�������汾����"V01.024"
    // char            patchVersion[DATA_BUF_F64_SIZE];  //ƽ̨�����������汾����"PV01.0001"

    char buf[DATA_BUF_F256_SIZE] = { 0 };

    if (sg_cmd_common_get("uname -s", info->distro) != VOS_OK) {
        printf("megsky test uname -s fail \n ");
    }
    if (sg_cmd_common_get("uname -v", info->version) != VOS_OK) {
        printf("megsky test uname -v fail \n ");
    }
    if (sg_cmd_common_get("uname -r", info->kernel) != VOS_OK) {
        printf("megsky test uname -r fail \n ");
    }
    if (sg_file_common_get(CUSTOM_SOFTWARE_VERSION_FILE, buf) == VOS_OK) {
        sprintf_s(info->softVersion, DATA_BUF_F256_SIZE, "%s", buf);
    } else if (sg_file_common_get(SOFTWARE_VERSION_FILE, buf) == VOS_OK) {
        sprintf_s(info->softVersion, DATA_BUF_F256_SIZE, "%s", buf);
    } else {
        return VOS_ERR;
    }
    if (sg_file_common_get(CUSTOM_PATCH_VERSION_FILE, buf) == VOS_OK) {
        sprintf_s(info->patchVersion, DATA_BUF_F256_SIZE, "%s", buf);
    } else if (sg_file_common_get(PATCH_VERSION_FILE, buf) == VOS_OK) {
        sprintf_s(info->patchVersion, DATA_BUF_F256_SIZE, "%s", buf);
    } else {
        return VOS_ERR;
    }
    return VOS_OK;
}

static gint links_comp(gconstpointer a, gconstpointer b)
{
    ifm_link_info_t *ifm_link_info1 = *(ifm_link_info_t **)a;
    ifm_link_info_t *ifm_link_info2 = *(ifm_link_info_t **)b;
    return (gint)(ifm_link_info1->if_index - ifm_link_info2->if_index);
}
//��ȡ����������Ϣ  ��ȡtype  id  name  mac
int sgcc_get_links_info(link_info_s **links_info_out, int *links_num)
{
    int i = 0;
    int num = 0;
    int ret_spr = 0;
    GError *error = NULL;
    GPtrArray *ret_entry = NULL;
    ifm_link_info_t *ifm_link_info = NULL;
    ifm_statusIf *ifm_status_client = NULL;
    link_info_s *links = NULL;

    ifm_status_client = (ifm_statusIf *)create_rpc_client((GType)TYPE_IFM_STATUS_CLIENT, (GType)SERVICE_TAG_IFM_STATUS);
    if (ifm_status_client == NULL) {
        return -1;
    }
    ret_entry = g_ptr_array_new();
    if (ret_entry == NULL) {
        free_rpc_client(ifm_status_client);
        return -1;
    }
    g_ptr_array_set_free_func(ret_entry, g_object_unref);

    //��ȡ
    bool rpc_call_return = ifm_status_if_get_link_info(ifm_status_client, &ret_entry, "",
        IF_SHOW_INFO_E_IFM_NO_DEV_AND_UP, &error);
    free_rpc_client(ifm_status_client);
    if (rpc_call_return == false) {
        g_ptr_array_unref(ret_entry);
        return -1;
    }
    //����
    g_ptr_array_sort(ret_entry, (GCompareFunc)links_comp);

    links = (link_info_s *)VOS_Malloc(MID_SGDEV, sizeof(link_info_s) * ret_entry->len);
    if (links == NULL) {
        fprintf(stderr, "Error - unable to allocate required memory\n");
        return VOS_ERR;
    }
    (void)memset_s(links, sizeof(link_info_s) * ret_entry->len, 0, sizeof(link_info_s) * ret_entry->len);

    //����
    for (i = 0; i < ret_entry->len; i++) {
        ifm_link_info = (ifm_link_info_t *)g_ptr_array_index(ret_entry, i);
        if (ifm_link_info->if_index < 0 || ifm_link_info->mtu == 0) {
            continue;
        }
        if (strcmp(ifm_link_info->type, "Eth") != 0 &&
            strcmp(ifm_link_info->type, "LTE") != 0 &&
            strcmp(ifm_link_info->type, "plc") != 0 &&
            strcmp(ifm_link_info->type, "rf") != 0) {
            continue;
        }
        if (strcmp(ifm_link_info->name, "br0") == 0 ||
            strcmp(ifm_link_info->name, "br_lsw") == 0 ||
            strcmp(ifm_link_info->name, "br_docker0") == 0) {
            continue;
        }
        //type
        if (sprintf_s(links[num].type, DATA_BUF_F32_SIZE, "%s", ifm_link_info->type) < 0) {
            goto error_end;
        }
        //id
        if (sprintf_s(links[num].id, DATA_BUF_F32_SIZE, "%d", ifm_link_info->if_index) < 0) {
            goto error_end;
        }
        //name
        if (strncmp(ifm_link_info->vlan_master, "NO", strlen(ifm_link_info->vlan_master)) != 0) {
            ret_spr = sprintf_s(links[num].name, DATA_BUF_F32_SIZE, "%s@%s", ifm_link_info->name, ifm_link_info->vlan_master);
        } else {
            ret_spr = sprintf_s(links[num].name, DATA_BUF_F32_SIZE, "%s", ifm_link_info->name);
        }
        if (ret_spr < 0) {
            goto error_end;
        }
        //mac
        if (sprintf_s(links[num].mac, DATA_BUF_F32_SIZE, "%s", ifm_link_info->mac_addr) < 0) {
            goto error_end;
        }
        printf("\n****links_in[%d].type = %s\n", num, links[num].type);
        printf("****links_in[%d].name = %s\n", num, links[num].name);
        printf("****links_in[%d].id = %s\n", num, links[num].id);
        printf("****links_in[%d].mac = %s\n", num, links[num].mac);
        num++;
    }
    *links_num = num;
    *links_info_out = links;
error_end:
    g_ptr_array_unref(ret_entry);
    return VOS_OK;
}

//��ȡ����������Ϣ2  ��ȡname  status
int sgcc_get_links_info2(link_dev_info_s **links_info_out, int *links_num)
{
    int i = 0;
    int num = 0;
    int ret_spr = 0;
    GError *error = NULL;
    GPtrArray *ret_entry = NULL;
    ifm_link_info_t *ifm_link_info = NULL;
    ifm_statusIf *ifm_status_client = NULL;
    link_dev_info_s *links = NULL;

    ifm_status_client = (ifm_statusIf *)create_rpc_client((GType)TYPE_IFM_STATUS_CLIENT, (GType)SERVICE_TAG_IFM_STATUS);
    if (ifm_status_client == NULL) {
        return -1;
    }
    ret_entry = g_ptr_array_new();
    if (ret_entry == NULL) {
        free_rpc_client(ifm_status_client);
        return -1;
    }
    g_ptr_array_set_free_func(ret_entry, g_object_unref);

    //��ȡ
    bool rpc_call_return = ifm_status_if_get_link_info(ifm_status_client, &ret_entry, "",
        IF_SHOW_INFO_E_IFM_NO_DEV_AND_UP, &error);
    free_rpc_client(ifm_status_client);
    if (rpc_call_return == false) {
        g_ptr_array_unref(ret_entry);
        return -1;
    }
    //����
    g_ptr_array_sort(ret_entry, (GCompareFunc)links_comp);

    links = (link_dev_info_s *)VOS_Malloc(MID_SGDEV, sizeof(link_dev_info_s) * ret_entry->len);
    if (links == NULL) {
        fprintf(stderr, "Error - unable to allocate required memory\n");
        return VOS_ERR;
    }
    (void)memset_s(links, sizeof(link_dev_info_s) * ret_entry->len, 0, sizeof(link_dev_info_s) * ret_entry->len);

    //����
    for (i = 0; i < ret_entry->len; i++) {
        ifm_link_info = (ifm_link_info_t *)g_ptr_array_index(ret_entry, i);
        if (ifm_link_info->if_index < 0 || ifm_link_info->mtu == 0) {
            continue;
        }
        if (strcmp(ifm_link_info->type, "Eth") != 0 &&
            strcmp(ifm_link_info->type, "LTE") != 0 &&
            strcmp(ifm_link_info->type, "plc") != 0 &&
            strcmp(ifm_link_info->type, "rf") != 0) {
            continue;
        }
        if (strcmp(ifm_link_info->name, "br0") == 0 ||
            strcmp(ifm_link_info->name, "br_lsw") == 0 ||
            strcmp(ifm_link_info->name, "br_docker0") == 0) {
            continue;
        }
        //name
        if (strncmp(ifm_link_info->vlan_master, "NO", strlen(ifm_link_info->vlan_master)) != 0) {
            ret_spr = sprintf_s(links[num].name, DATA_BUF_F32_SIZE, "%s@%s", ifm_link_info->name, ifm_link_info->vlan_master);
        } else {
            ret_spr = sprintf_s(links[num].name, DATA_BUF_F32_SIZE, "%s", ifm_link_info->name);
        }
        if (ret_spr < 0) {
            goto error_end;
        }
        //status
        if ((strncmp(ifm_link_info->name, "plc", strlen("plc")) == 0 ||
            strncmp(ifm_link_info->name, "rf", strlen("rf")) == 0) &&
            strncmp(ifm_link_info->oper_status, "unknown", strlen(ifm_link_info->oper_status)) == 0) {

            ret_spr = sprintf_s(links[num].status, DATA_BUF_F64_SIZE, "%s", "up");
        } else {
            ret_spr = sprintf_s(links[num].status, DATA_BUF_F64_SIZE, "%s", ifm_link_info->oper_status);
        }
        if (ret_spr < 0) {
            goto error_end;
        }
        num++;
    }
    *links_num = num;
    *links_info_out = links;
error_end:
    g_ptr_array_unref(ret_entry);
    return VOS_OK;
}

int sg_get_dev_insert_info(dev_acc_req_s *devinfo)  //��ȡ�豸������Ϣ
{
    int ret = VOS_OK;
    if (sg_get_dev_devinfo(&devinfo->dev) == VOS_ERR) {
        ret = VOS_ERR;
        printf("get device infomation fail. \n");
    }
    if (sg_get_cpu_devinfo(&devinfo->cpu) == VOS_ERR) {
        ret = VOS_ERR;
        printf("get cpu infomation fail. \n");
    }
    if (sg_get_mem_devinfo(&devinfo->mem) == VOS_ERR) {
        ret = VOS_ERR;
        printf("get memory infomation fail. \n");
    }
    if (sg_get_disk_devinfo(&devinfo->disk) == VOS_ERR) {
        ret = VOS_ERR;
        printf("get disk infomation fail. \n");
    }
    if (sg_get_os_devinfo(&devinfo->os) == VOS_ERR) {
        ret = VOS_ERR;
        printf("get os infomation fail. \n");
    }
    if (sgcc_get_links_info(&devinfo->links, &devinfo->link_len) == VOS_ERR) {
        ret = VOS_ERR;
        printf("get links infomation fail. \n");
    }
    return ret;
}

int sg_down_update_host(device_upgrade_s cmdobj, char *errormsg)
{
    int ret = VOS_OK;
    char errmsg[DATA_BUF_F256_SIZE];
    char filepath[DATA_BUF_F256_SIZE];
    char signature[DATA_BUF_F256_SIZE];
    sprintf(filepath, "/mnt/internal/internal_storage/%s", cmdobj.file.name);
    sprintf(signature, "/mnt/internal/internal_storage/%s", cmdobj.file.sign.name);
    if (sysman_rpc_transport_open() == VOS_OK) {
        ret = software_management_action_call_load_software(filepath, signature, errmsg, DATA_BUF_F256_SIZE);
        sysman_rpc_transport_close();
        if (ret != VOS_OK) {
            sprintf(errormsg, "%s", errmsg);
        }
    }
    return ret;
}
int sg_down_update_patch(device_upgrade_s cmdobj, char *errormsg)
{
    int ret = VOS_OK;
    char errmsg[DATA_BUF_F256_SIZE];

    //ret = software_management_action_call_install_patch(const char* filepath, const char* signature, bool force_flag, char* errmsg, size_t msglen);

    return ret;
}

//��ȡ�豸���һ������ʱ��
int sg_get_devstdatetime(char *timeBuf, long *timenum)
{
    struct sysinfo Information;
    //Information.uptime		            //��ʾ�����豸����ʱ��
    time_t cur_time = 0;
    time_t boot_time = 0;
    struct tm format_time = { 0 };
    if (sysinfo(&Information)) {
        return VOS_ERR;
    }
    *timenum = Information.uptime;
    cur_time = time(0);
    if (cur_time > Information.uptime) {
        boot_time = cur_time - Information.uptime;
    } else {
        boot_time = Information.uptime - cur_time;
    }
    if (gmtime_r(&boot_time, &format_time) == NULL) {   //UTCʱ�� 
        return VOS_ERR;
    }
    (void)strftime(timeBuf, 32, "%Y-%m-%d,%T,%Z", &format_time);
}

//��ȡ�����ڴ��ʹ������
uint32_t sg_memvirt_total(void)
{
    char temp_str[DEV_INFO_MSG_BUFFER_LEN];
    uint32_t total = 0;
    char buf[DEV_INFO_MSG_BUFFER_LEN];
    FILE * p_file = NULL;
    p_file = popen("free |grep Swap|awk '{print $2}'", "r");
    if (!p_file) {
        printf("Erro to popen \n");
    }
    while (fgets(buf, DEV_INFO_MSG_BUFFER_LEN, p_file) != NULL) {
        sprintf(temp_str, "%s", buf);
    }
    sscanf(temp_str, "%d", &total);	//���ַ���תΪ����
    pclose(p_file);
    return total;
}
//��ȡ�����ڴ�ĵ�ǰʹ����
uint8_t sg_memvirt_used(void)
{
    float usagerate = 0.0;
    uint8_t ret_value = 0;
    char temp_str[DEV_INFO_MSG_BUFFER_LEN];
    uint32_t used = 0;
    char buf[DEV_INFO_MSG_BUFFER_LEN];
    FILE * p_file = NULL;
    p_file = popen("free |grep Swap|awk '{print $3}'", "r");  //��ȡ�����ڴ��ʹ����
    if (!p_file) {
        printf("Erro to popen \n");
    }
    while (fgets(buf, DEV_INFO_MSG_BUFFER_LEN, p_file) != NULL) {
        sprintf(temp_str, "%s", buf);
    }
    sscanf(temp_str, "%d", &used);	//���ַ���תΪ����
    pclose(p_file);
    printf("used = %d\n", used);
    printf("sg_memvirt_total = %d\n", sg_memvirt_total());
    if (sg_memvirt_total() == 0) {
        ret_value = 0;
    } else {
        usagerate = (float)used / sg_memvirt_total();   			//��ȡ�ٷ��� 
        ret_value = (uint8_t)(usagerate * 100);
    }
    return ret_value;
}

void sg_getdevinf(void)			//��ȡ�����豸��Ϣ��������		
{
    float usagerate;
    char temp_str[DEV_INFO_MSG_BUFFER_LEN];
    uint32_t used = 0;
    char buf[1024];
    FILE * p_file = NULL;
    p_file = popen("ip link", "r");  //��ȡ�����ڴ��ʹ����
    if (!p_file) {
        printf("Erro to popen \n");
    }
    while (fgets(buf, DEV_INFO_MSG_BUFFER_LEN, p_file) != NULL) {
        sprintf(temp_str, "%s", buf);
        printf("aaaa=%s\n", buf);
    }
    printf("sss=%s\n", temp_str);
    // sscanf(temp_str,"%d",&used);	//���ַ���תΪ����
    pclose(p_file);
}

//��ȡ�豸����
void sg_getdevname(char *devname)
{
    char buf[DEV_INFO_MSG_BUFFER_LEN];
    FILE * p_file = NULL;
    p_file = popen("hostname", "r");  //��ȡ�����ڴ��ʹ����
    if (!p_file) {
        printf("Erro to popen \n");
    }
    while (fgets(buf, DEV_INFO_MSG_BUFFER_LEN, p_file) != NULL) {
        sprintf(devname, "%s", buf);
    }
    pclose(p_file);
}

/******************************************************************
 * ��ȡ���豸�е�alertֵ
 * ������threshold
 * ���룺select
 * selectȡ: CPU_USAGE
 * 			 MEM_USAGE
 * 			 STORAGE_USAGE
 *****************************************************************/
int sg_get_devusage_threshold(int *threshold, uint8_t select)
{
    int ret = VOS_OK;
    uint32_t infoNum = 0;
    cpuusage_s cpu_value = { 0 };
    memoryusage_s mem_value = { 0 };
    storageusage_s *storage_value = NULL;
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    ret = sysman_rpc_transport_open();
    if (ret == VOS_OK) {
        if (select == CPU_USAGE) {
            ret = cpuusage_status_call_get_threshold(&cpu_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
        } else if (select == MEM_USAGE) {
            ret = memoryusage_status_call_get_threshold(&mem_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
        } else if (select == STORAGE_USAGE) {
            storage_value = (storageusage_s*)VOS_Malloc(MID_SGDEV, sizeof(storageusage_s) * STORAGE_PARTITION_MAX_NUM);
            ret = storageusage_status_call_get_threshold(&infoNum, storage_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
        }
    } else {
        printf("rpc open error! \n");
        return VOS_ERR;
    }
    sysman_rpc_transport_close();
    // �澯��ֵȡ output_value.warning���澯��ֵ�ָ�ȡ output_value.warning - 10
    if (select == CPU_USAGE) {
        *threshold = cpu_value.alert;
    } else if (select == MEM_USAGE) {
        *threshold = mem_value.alert;
    } else if (select == STORAGE_USAGE) {
        *threshold = storage_value[0].alert;
        VOS_Free(storage_value);
    }
    return ret;
}


//���´�����ʱ��
int sg_creat_timer(rep_period_s *paraobj)
{
    int nRet = VOS_OK;
    //ע�ᶨʱ��
    nRet = sg_timer_heart_create(paraobj->heartPeriod);
    if (nRet != VOS_OK) {
        return nRet;
    }
    nRet = sg_timer_dev_create(paraobj->devPeriod);
    if (nRet != VOS_OK) {
        return nRet;
    }
    nRet = sg_timer_container_create(paraobj->conPeriod);
    if (nRet != VOS_OK) {
        return nRet;
    }
    nRet = sg_timer_app_create(paraobj->appPeriod);
    if (nRet != VOS_OK) {
        return nRet;
    }
    return nRet;
}
//ʱ��������
void sg_set_period(rep_period_s *paraobj)
{
    sg_write_period_file(paraobj);					//�����ļ��洢
    if (sg_creat_timer(paraobj) == VOS_OK) {	//���´�����ʱ��
        printf("********Creat timer success !************** \n");
    } else {
        printf("creat timer error! \n");
    }
}

//�����ڲ����ļ�
int sg_read_period_file(sg_period_info_s *m_devperiod)
{
    int filesize = 0;
    FILE *fp = NULL;
    char *buf = NULL;
    char real_path[PATH_MAX] = { 0 };
    json_t *piload = NULL;

    filesize = getfilesize("/mnt/internal_storage/sg_period_params");
    if (filesize <= 0) {
        return VOS_ERR;
    }

    buf = (char *)VOS_Malloc(MID_SGDEV, filesize + 1);
    if (buf == NULL) {
        fprintf(stderr, "Error - unable to allocate required memory\n");
        return VOS_ERR;
    }
    (void)memset_s(buf, filesize + 1, 0, filesize + 1);

    if (realpath("/mnt/internal_storage/sg_period_params", real_path) == NULL) {
        return -1;
    }

    fp = fopen(real_path, "r");
    if (fp == NULL) {
        (void)printf("Failed to open the file!\n");
        (void)VOS_Free(buf);
        return VOS_ERR;
    }

    if (!fread(buf, filesize, 1, fp)) {
        (void)printf("read file error!\n");
        (void)fclose(fp);
        (void)VOS_Free(buf);
        return VOS_ERR;
    }

    piload = load_json(buf);		//���ַ���תΪjson
    printf("period_buf = %s\n", buf);
    (void)VOS_Free(buf);

    if (!json_is_object(piload)) {
        printf("%s,%d\n", __FILE__, __LINE__);
    }
    if (json_into_uint32_t(&m_devperiod->appperiod, piload, "appperiod") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_uint32_t(&m_devperiod->containerperiod, piload, "containerperiod") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_uint32_t(&m_devperiod->devperiod, piload, "devperiod") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    if (json_into_uint32_t(&m_devperiod->devheartbeatperiod, piload, "devheartbeatperiod") != VOS_OK) {
        (void)json_decref(piload);
        return VOS_ERR;
    }
    (void)json_decref(piload);

    return VOS_OK;
}

//��ȡ�¶ȼ����ֵ�ķ���
int sg_get_monitor_temperature_threshold(temp_info_s *temp_info_out)
{
    int ret = VOS_OK;
    uint32_t infoNum = 0;
    uint32_t num = 0;
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    temperature_threshold_status_s *temp_threshold_get = NULL;
    //��ȡ�ߵ�����ֵ
    temp_threshold_get = (temperature_threshold_status_s*)VOS_Malloc(MID_SGDEV, sizeof(temperature_threshold_status_s) *
        MONITOR_TEMP_NUM);
    (void)memset_s(temp_threshold_get, MONITOR_TEMP_NUM * sizeof(temperature_threshold_status_s), 0, MONITOR_TEMP_NUM *
        sizeof(temperature_threshold_status_s));

    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman rpc open error!\n");
        return VOS_ERR;
    }
    ret = temperature_status_call_get_monitor_threshold(&infoNum, temp_threshold_get, errmsg, SYSMAN_RPC_ERRMSG_MAX);  //�з���ֵ
    sysman_rpc_transport_close();
    for (num = 0; num < infoNum; num++) {   //������Ҫ����"Main_board"�ԱȻ�ȡ
        if (0 == strncmp(temp_threshold_get[num].moduleName, TEMPERATURE_MODULE_NAME_MAINBOARD,
            strlen(TEMPERATURE_MODULE_NAME_MAINBOARD))) {
            printf("gettemLow = %d\n", temp_threshold_get[num].temLow);
            printf("gettemHigh = %d\n", temp_threshold_get[num].temHigh);
            printf("gettemVal = %d\n", temp_threshold_get[num].temVal);
            temp_info_out->temLow = temp_threshold_get[num].temLow;
            temp_info_out->temHigh = temp_threshold_get[num].temHigh;

            printf("temp_info_out->temLow = %d\n", temp_info_out->temLow);
            printf("temp_info_out->temHigh = %d\n", temp_info_out->temHigh);
            //           break;  �ж�һ��Ӧ�÷����Ķ�
        }
    }
    (void)VOS_Free(temp_threshold_get);
    return ret;
}

int sg_get_device_temperature_threshold(dev_sta_reply_s *dev_sta_reply)
{
    int ret = VOS_OK;
    uint32_t infoNum = 0;
    uint32_t num = 0;
    char errmsg[SYSMAN_RPC_ERRMSG_MAX] = { 0 };
    temperature_s *tempoutput_value = NULL;

    tempoutput_value = (temperature_s*)VOS_Malloc(MID_SGDEV, sizeof(temperature_s) * MONITOR_TEMP_NUM);

    (void)memset_s(tempoutput_value, MONITOR_TEMP_NUM * sizeof(temperature_s), 0, sizeof(temperature_s) *
        MONITOR_TEMP_NUM);
    if (sysman_rpc_transport_open() != VOS_OK) {
        printf("sysman_rpc_transport_open error!\n");
        ret = VOS_ERR;
    }
    ret = temperature_status_call_get_device_temperature(&infoNum, tempoutput_value, errmsg, SYSMAN_RPC_ERRMSG_MAX);
    sysman_rpc_transport_close();
    if (ret != VOS_OK) {
        printf("temperature_status_call_get_device_temperature error! \n");
        ret = VOS_ERR;
    }
    for (num = 0; num < infoNum; num++) {
        if (0 == strncmp(tempoutput_value[num].name, TEMPERATURE_MODULE_NAME_MAINBOARD,
            strlen(TEMPERATURE_MODULE_NAME_MAINBOARD))) {
            dev_sta_reply->tempValue = tempoutput_value[num].temperature;
            printf("tempoutput_value[%d].temperature = %d \n", num, tempoutput_value[num].temperature);
            break;
        }
    }
    VOS_Free(tempoutput_value);
    return ret;
}


//��ȡʱ����Ϣ ��XXXX-XX-XX XX:XX:XXʱ���ʽ����Ϊint��
int sg_get_time(char *dateTime, sys_time_s *sys_time)
{
    int ret = VOS_OK;
    int pos = 0;
    char Time_str[16];
    pos = sg_find(dateTime, strlen(dateTime), "-", 1, 0);
    if (pos > 0) {
        if (sg_str_mid(dateTime, strlen(dateTime), pos - 4, Time_str, 4)) {  		//��  
            sys_time->tm_year = atoi(Time_str);
        } else {
            printf("sg_str_mid_get_year_error!\n");
            ret = VOS_ERR;
        }
        if (sg_str_mid(dateTime, strlen(dateTime), pos + 1, Time_str, 2)) {		//�� 
            sys_time->tm_mon = atoi(Time_str);
        } else {
            printf("sg_str_mid_get_mon_error!\n");
            ret = VOS_ERR;
        }
        if (sg_str_mid(dateTime, strlen(dateTime), pos + 4, Time_str, 2)) {		//�� 
            sys_time->tm_mday = atoi(Time_str);
        } else {
            printf("sg_str_mid_get_day_error!\n");
            ret = VOS_ERR;
        }
        if (sg_str_mid(dateTime, strlen(dateTime), pos + 7, Time_str, 2)) {		//ʱ 
            sys_time->tm_hour = atoi(Time_str);
        } else {
            printf("sg_str_mid_get_hour_error!\n");
            ret = VOS_ERR;
        }
        if (sg_str_mid(dateTime, strlen(dateTime), pos + 10, Time_str, 2)) {		//�� 
            sys_time->tm_min = atoi(Time_str);
        } else {
            printf("sg_str_mid_get_min_error!\n");
            ret = VOS_ERR;
        }
        if (sg_str_mid(dateTime, strlen(dateTime), pos + 13, Time_str, 2)) {		//�� 
            sys_time->tm_sec = atoi(Time_str);
            printf("sys_time->tm_sec = %d\n", sys_time->tm_sec);
        } else {
            printf("sg_str_mid_get_sec_error!\n");
            ret = VOS_ERR;
        }
    } else {
        printf("common_get pos error!\n");
        ret = VOS_ERR;
    }
    return ret;
}

int sg_get_dev_edge_reboot(void)
{
    return edge_reboot_flag;
}

void sg_set_edge_reboot(int flag)
{
    edge_reboot_flag = flag;
    printf("Set edge_reboot_flag = %d . \n", edge_reboot_flag);
}
