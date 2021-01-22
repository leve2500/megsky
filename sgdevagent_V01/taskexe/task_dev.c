#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "thread_business_Interact.h"
#include "vm_public.h"
#include "vm_rpc_api.h"
#include "vos_typdef.h"
#include "sysman_rpc_api.h"
#include "app_management_service_api.h"

#include <glib-object.h>
#include <thrift/c_glib/thrift.h>
#include <thrift/c_glib/processor/thrift_multiservice_processor.h>
#include <thrift/c_glib/protocol/thrift_binary_protocol.h>
#include <thrift/c_glib/protocol/thrift_router_protocol.h>  
#include <thrift/c_glib/server/thrift_server.h>
#include <thrift/c_glib/server/thrift_router_server.h>
#include <thrift/c_glib/transport/thrift_router_transport.h>
#include <thrift/c_glib/transport/thrift_kmsg.h>
#include <thrift_service_def.h>

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