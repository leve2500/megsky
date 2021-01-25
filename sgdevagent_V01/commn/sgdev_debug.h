/*=====================================================================
  *�ļ���downlink.h
 *
  *���������нӿ�
 *
  *���ߣ�����			2020��10��13��20:14:07
  *
  *�޸ļ�¼��
=====================================================================*/

#ifndef _SGDEV_DEBUG_H_
#define _SGDEV_DEBUG_H_

#include "ssp_syslog.h"

#ifdef __cplusplus
extern "C" {  // only need to export C interface if used by C++ source code
#endif

#define SGDEV_MODULE   "SGDEV"

//��־
#define SGDEV_EMERG(type, module, fmt, arg...)  ssp_syslog(LOG_EMERG, type, module, fmt, ##arg)
#define SGDEV_ALERT(type, module, fmt, arg...)  ssp_syslog(LOG_ALERT, type, module, fmt, ##arg)
#define SGDEV_CRIT(type, module, fmt, arg...)   ssp_syslog(LOG_CRIT, type, module, fmt, ##arg)
#define SGDEV_ERROR(type, module, fmt, arg...)  ssp_syslog(LOG_ERR, type, module, fmt, ##arg)
#define SGDEV_WARN(type, module, fmt, arg...)   ssp_syslog(LOG_WARNING, type, module, fmt, ##arg)
#define SGDEV_NOTICE(type, module, fmt, arg...) ssp_syslog(LOG_NOTICE, type, module, fmt, ##arg)
#define SGDEV_INFO(type, module, fmt, arg...)   ssp_syslog(LOG_INFO, type, module, fmt, ##arg)
#define SGDEV_DEBUG(type, module, fmt, arg...)  ssp_syslog(LOG_DEBUG, type, module, fmt, ##arg)

//���� SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "test ssp syslog.");

void sg_log_init(void);
void sg_log_close(void);

#ifdef __cplusplus
}
#endif 

#endif


