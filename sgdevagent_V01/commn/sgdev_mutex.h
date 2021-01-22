

/*=====================================================================
 * 文件：tzc_mutex.h
 *
 * 描述：锁
 *
 * 作者：田振超			2020年10月12日20:10:37
 *
 * 修改记录：
 =====================================================================*/


#ifndef _SGDEV_MUTEX_H_
#define _SGDEV_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

void sg_mutex_init(void);
void sg_mutex_exit(void);
bool sg_try_lock(void);
void sg_lock(void);
void sg_unlock(void);



#ifdef __cplusplus
}
#endif 

#endif