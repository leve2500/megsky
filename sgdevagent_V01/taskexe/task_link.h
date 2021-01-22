/*=====================================================================
  *�ļ���downlink.h
 *
  *���������нӿ�
 *
  *���ߣ�����			2020��10��13��20:14:07
  *
  *�޸ļ�¼��
=====================================================================*/

#ifndef _TASK_LINK_H_
#define _TASK_LINK_H_

#ifdef __cplusplus
extern "C" {
#endif

//�豸��Ϣ�ֶ�
#define HOSTNAME_FILE             "/proc/sys/kernel/hostname"                   //�ն�����
#define DEVICE_TYPE_FILE          "/mnt/internal_storage/security_proxy_config" //�ն����� �ն�IDǰ�����ֽ�
#define VENDOR_FILE               "/etc/devinfo/vendor"                         //�ն˳�����Ϣ
#define DEVICE_HARDWARE_VERSION   "/mnt/custom/hardware_version"                //�ն�Ӳ���汾�� ������ʽ"version:HV01.01",���û�У�Ĭ��"HV01.01"

int sg_get_dev_insert_info(dev_acc_req_s * devinfo);
int sg_get_dev_devinfo(dev_info_s *info);                                 //��ȡ�豸��Ϣ�ֶ�
int sg_get_cpu_devinfo(cpu_info_s *info);                                 //��ȡcpu��Ϣ�ֶ�
int sg_get_mem_devinfo(mem_info_s *info);                                 //��ȡmem��Ϣ�ֶ�
int sg_get_disk_devinfo(disk_info_s *info);                               //��ȡdisk��Ϣ�ֶ� 
int sg_get_os_devinfo(os_info_s *info);                                   //��ȡos��Ϣ�ֶ�
int sgcc_get_links_info(link_info_s **links_info_out, int *links_num);    //��ȡlinks��Ϣ�ֶ�
int sgcc_get_links_info2(link_dev_info_s **links_info_out, int *links_num);

int sg_down_update_host(device_upgrade_s cmdobj, char *errormsg);
int sg_down_update_patch(device_upgrade_s cmdobj, char *errormsg);
int sg_get_devstdatetime(char *timeBuf, long *timenum);                //��ȡ�豸���һ������ʱ�� ��ȡ�豸����ʱ��
uint32_t sg_memvirt_total(void);                                    //��ȡ�����ڴ��ʹ������

uint8_t sg_memvirt_used(void);        //��ȡ�����ڴ�ĵ�ǰʹ����
void sg_getdevinf(void);              //��ȡ�����豸��Ϣ
void sg_getdevname(char *devname);       //��ȡ�豸����	

//�豸��������޸������װ����
int sg_get_devusage_threshold(int *threshold, uint8_t select);      //��ȡ���豸�е�alertֵ
int sg_creat_timer(rep_period_s *paraobj);                           //���´�����ʱ��
void sg_set_period(rep_period_s *paraobj);                           //ʱ��������

int sg_read_period_file(sg_period_info_s *m_devperiod);             //�����ڲ����ļ�
int sg_get_monitor_temperature_threshold(temp_info_s *info);        //��ȡ�¶ȼ����ֵ�ķ���
int sg_get_device_temperature_threshold(dev_sta_reply_s *dev_sta_reply);

int sg_get_time(char *dateTime, sys_time_s *sys_time);              //��ȡʱ����Ϣ ���ַ�ʱ���ʽתΪint
int sg_get_dev_edge_reboot(void);                                   //��ȡ���������־
void sg_set_edge_reboot(int flag);                                  //�������������־


#ifdef __cplusplus
}
#endif 

#endif


