

/*=====================================================================
 * 文件：thread_interact.h
 *
 * 描述：业务交互线程
 *
 * 作者：田振超			2020年9月27日17:09:55
 *
 * 修改记录：
 =====================================================================*/


#ifndef _THREAD_BUS_INTER_H_
#define _THREAD_BUS_INTER_H_

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

void set_device_upgrade_status(dev_status_reply_s sta);
void set_container_install_status(dev_status_reply_s sta);
void set_container_upgrade_status(dev_status_reply_s sta);
void set_app_install_status(dev_status_reply_s sta);
void set_app_upgrade_status(dev_status_reply_s sta);

int bus_inter_thread(void);


#ifdef __cplusplus
}
#endif 

#endif
