#include <stdio.h>
#include <string.h>

#include "vos_errno.h"
#include "vrp_monit.h"
#include "ssp_syslog.h"
#include "sgdev_debug.h"


void sg_log_init(void)
{
    (void)ssp_syslog_init(LOG_INFO, SGDEV_MODULE);
}
void sg_log_close(void)
{
    closelog();
}
