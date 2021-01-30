

/*=====================================================================
  *文件：task_app.h
 *
  *描述：下行接口
 *
  *作者：田振超			2021年1月19日20:34:51
  *
  *修改记录：

 =====================================================================*/


#ifndef _TASK_APP_H_
#define _TASK_APP_H_


#ifdef __cplusplus
extern "C" {
#endif

int sg_app_install(app_install_cmd_s cmd_obj, char *errmsg);
int sg_app_update(app_install_cmd_s cmd_obj, char *errmsg);
int sg_container_with_app_install(container_install_cmd_s *cmd_obj, char *errmsg);

#ifdef __cplusplus
}
#endif 

#endif


