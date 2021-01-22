

/*=====================================================================
 * 文件：thread_task_exe.h
 *
 * 描述：任务执行线程
 *
 * 作者：田振超			2020年9月27日17:10:06
 *
 * 修改记录：
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
