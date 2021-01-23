/*=====================================================================
 * �ļ���param_cfg.h
 *
 * �����������ļ��Ͷ����ļ���ȡ
 *
 * ���ߣ�Ѧ����			2020��9��27��17:10:15
 *
 * �޸ļ�¼��
=====================================================================*/


#ifndef _SGDEV_PARAM_H_
#define _SGDEV_PARAM_H_

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

int getfilesize(char *strFileName);

//�������ļ�
int read_section_file(void);

//д�����ļ�
void write_section_file(void);

//�������ļ�
int read_period_file(void);

//д�����ļ�
void sg_write_period_file(rep_period_s *paraobj);

//�������ļ�
int read_param_file(void);

//д�����ļ�
void write_param_file(void);

//��ȡ����
sg_dev_param_info_s sg_get_param(void);

//�������ڲ���
sg_period_info_s sg_get_period(void);

//��ȡ����
sg_dev_section_info_s sg_get_section(void);


#ifdef __cplusplus
}
#endif 

#endif
