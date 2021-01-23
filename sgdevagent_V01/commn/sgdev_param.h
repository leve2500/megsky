/*=====================================================================
 * 文件：param_cfg.h
 *
 * 描述：参数文件和断面文件读取
 *
 * 作者：薛庆朋			2020年9月27日17:10:15
 *
 * 修改记录：
=====================================================================*/


#ifndef _SGDEV_PARAM_H_
#define _SGDEV_PARAM_H_

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

int getfilesize(char *strFileName);

//读断面文件
int read_section_file(void);

//写断面文件
void write_section_file(void);

//读周期文件
int read_period_file(void);

//写周期文件
void sg_write_period_file(rep_period_s *paraobj);

//读参数文件
int read_param_file(void);

//写参数文件
void write_param_file(void);

//获取参数
sg_dev_param_info_s sg_get_param(void);

//返回周期参数
sg_period_info_s sg_get_period(void);

//获取断面
sg_dev_section_info_s sg_get_section(void);


#ifdef __cplusplus
}
#endif 

#endif
