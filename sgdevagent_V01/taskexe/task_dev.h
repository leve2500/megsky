

/*=====================================================================
  *文件：task_container.h
 *
  *描述：下行接口
 *
  *作者：田振超			2021年1月21日20:34:51
  *
  *修改记录：

 =====================================================================*/


#ifndef _TASK_DEV_H_
#define _TASK_DEV_H_


#ifdef __cplusplus
extern "C" {
#endif

int sg_dev_install(device_upgrade_s *cmd_obj, char *errmsg);

#ifdef __cplusplus
}
#endif 

#endif


