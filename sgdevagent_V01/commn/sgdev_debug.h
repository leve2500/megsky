/*=====================================================================
  *文件：downlink.h
 *
  *描述：下行接口
 *
  *作者：田振超			2020年10月13日20:14:07
  *
  *修改记录：
=====================================================================*/

#ifndef _SGDEV_DEBUG_H_
#define _SGDEV_DEBUG_H_

#include "ssp_syslog.h"

#ifdef __cplusplus
extern "C" {  // only need to export C interface if used by C++ source code
#endif

#define SGDEV_MODULE   "SGDEV"

//日志
#define SGDEV_EMERG(type, module, fmt, arg...)  ssp_syslog(LOG_EMERG, type, module, fmt, ##arg)
#define SGDEV_ALERT(type, module, fmt, arg...)  ssp_syslog(LOG_ALERT, type, module, fmt, ##arg)
#define SGDEV_CRIT(type, module, fmt, arg...)   ssp_syslog(LOG_CRIT, type, module, fmt, ##arg)
#define SGDEV_ERROR(type, module, fmt, arg...)  ssp_syslog(LOG_ERR, type, module, fmt, ##arg)
#define SGDEV_WARN(type, module, fmt, arg...)   ssp_syslog(LOG_WARNING, type, module, fmt, ##arg)
#define SGDEV_NOTICE(type, module, fmt, arg...) ssp_syslog(LOG_NOTICE, type, module, fmt, ##arg)
#define SGDEV_INFO(type, module, fmt, arg...)   ssp_syslog(LOG_INFO, type, module, fmt, ##arg)
#define SGDEV_DEBUG(type, module, fmt, arg...)  ssp_syslog(LOG_DEBUG, type, module, fmt, ##arg)

//样例 SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "test ssp syslog.");

void sg_log_init(void);
void sg_log_close(void);

#ifdef __cplusplus
}
#endif 

#endif


