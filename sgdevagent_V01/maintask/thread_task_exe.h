

/*=====================================================================
 * �ļ���thread_task_exe.h
 *
 * ����������ִ���߳�
 *
 * ���ߣ�����			2020��9��27��17:10:06
 *
 * �޸ļ�¼��
 =====================================================================*/


#ifndef _TASK_EXECUTION_H_
#define _TASK_EXECUTION_H_

#ifdef __cplusplus
extern "C" {
#endif

 //
int sg_task_execution_dev_thread(void);

int sg_task_execution_container_thread(void);

int sg_task_execution_app_thread(void);


#ifdef __cplusplus
}
#endif 

#endif
