/*=====================================================================
 * �ļ���param_cfg.h
 *
 * �����������ļ��Ͷ����ļ���ȡ
 *
 * ���ߣ�Ѧ����			2020��9��27��17:10:15
 *
 * �޸ļ�¼��
=====================================================================*/


#ifndef __SGDEV_PARAM_H__
#define __SGDEV_PARAM_H__

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

int getfilesize(char *strFileName);               // ��ȡ�ļ���С
int read_section_file(void);                      // �������ļ�
void write_section_file(void);                    // д�����ļ�
int read_period_file(void);                       // �������ļ�
void sg_write_period_file(rep_period_s *paraobj); // д�����ļ�
int read_param_file(void);                        // �������ļ�
void write_param_file(void);                      // д�����ļ�
sg_dev_param_info_s sg_get_param(void);           // ��ȡ����
sg_period_info_s sg_get_period(void);             // �������ڲ���
sg_dev_section_info_s sg_get_section(void);       // ��ȡ����

#ifdef __cplusplus
}
#endif 

#endif
