#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"
#include "ssp_mid.h"

#include "vm_public.h"
#include "vos_typdef.h"
#include "sysman_rpc_api.h"
#include "app_management_service_api.h"

#include <glib-object.h>
#include <thrift/c_glib/thrift.h>

#include "mqtt_json.h"
#include "mqtt_pub.h"
#include "mqtt_dev.h"
#include "sgdev_queue.h"
#include "sgdev_param.h"
#include "sgdev_curl.h"
#include "sgdev_struct.h"
#include "task_link.h"
#include "task_dev.h"
#include "timer_pack.h"

int sg_dev_install(device_upgrade_s *cmdobj, char *errmsg)
{
    int ret = VOS_OK;
    

    return ret;



}