/*
 * Copyright（c）Huawei Technologies Co.,Ltd.2019-2020.All rights reserved
 * Description : sgdevagent common header file
*/

#ifndef _SGDEV_COMMON_H_
#define _SGDEV_COMMON_H_

#ifdef __cplusplus
extern "C" {  // only need to export C interface if used by C++ source code
#endif


int sg_find(char *m_pBuf, int m_nLen, const char *i_pStr, int i_nNum, int i_nIndex);
int sg_str_left(char *pData, int m_nLen, char *dData, int i_nLen);
int sg_str_mid(char *pData, int m_nLen, int i_nIndex, char *dData, int i_nLen);
int sg_str_right(char *pData, int m_nLen, char *dData, int i_nLen);
int sg_str_colon(char *pData, int m_nLen, char *ldDatal, char *rdData);// ：分割左右字符
int sg_cmd_common_get(const char* p, char* desp);
int sg_read_file_get(const char *p, char *desp);
int sg_file_common_get(const char *p, char *desp);

int sg_get_dev_insert_info(dev_acc_req_s * devinfo);
int sg_get_dev_devinfo(dev_info_s *info);                                 //获取设备信息字段
int sg_get_cpu_devinfo(cpu_info_s *info);                                 //获取cpu信息字段
int sg_get_mem_devinfo(mem_info_s *info);                                 //获取mem信息字段
int sg_get_disk_devinfo(disk_info_s *info);                               //获取disk信息字段 
int sg_get_os_devinfo(os_info_s *info);                                   //获取os信息字段
int sgcc_get_links_info(link_info_s **links_info_out, int *links_num);    //获取links信息字段
int sgcc_get_links_info2(link_dev_info_s **links_info_out, int *links_num);

int sg_down_update_host(device_upgrade_s cmdobj, char *errormsg);
int sg_down_update_patch(device_upgrade_s cmdobj, char *errormsg);
int sg_get_devstdatetime(char *timeBuf, long *timenum);                //获取设备最近一次启动时间 获取设备运行时长
uint32_t sg_memvirt_total(void);                                    //获取虚拟内存的使用总量

uint8_t sg_memvirt_used(void);        //获取虚拟内存的当前使用率
void sg_getdevinf(void);              //获取其他设备信息
void sg_getdevname(char *devname);       //获取设备名称	

//设备管理参数修改命令封装方法
int sg_get_devusage_threshold(int *threshold, uint8_t select);      //获取到设备中的alert值
int sg_creat_timer(rep_period_s *paraobj);                           //重新创建定时器
void sg_set_period(rep_period_s *paraobj);                           //时间间隔设置

int sg_read_period_file(sg_period_info_s *m_devperiod);             //读周期参数文件
int sg_get_monitor_temperature_threshold(temp_info_s *info);        //获取温度监控阈值的方法
int sg_get_device_temperature_threshold(dev_sta_reply_s *dev_sta_reply);

int sg_get_time(char *dateTime, sys_time_s *sys_time);              //获取时间信息 将字符时间格式转为int
int sg_get_dev_edge_reboot(void);                                   //获取重启组件标志
void sg_set_edge_reboot(int flag);                                  //设置重启组件标志
unsigned long long sg_hamming_weight(unsigned long long number);    //统计无符号长整数二进制表示中1的个数

#ifdef __cplusplus
}
#endif 

#endif


