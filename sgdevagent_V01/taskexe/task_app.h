

/*=====================================================================
  *�ļ���task_app.h
 *
  *���������нӿ�
 *
  *���ߣ�����			2021��1��19��20:34:51
  *
  *�޸ļ�¼��

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


