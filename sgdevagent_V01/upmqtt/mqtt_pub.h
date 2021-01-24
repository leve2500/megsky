/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description : mqtt pub header file
*/


#ifndef __MQTT_PUB_H__
#define __MQTT_PUB_H__

#include "MQTTClient.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QOS         0
#define TIMEOUT     10000L


typedef struct mqtt_connect_data {
    int        mqtt_init_flag;
    int        mqtt_connect_flag;
}mqtt_connect_data_s;


int sg_agent_mqtt_init(char *server_uri,char *client_id);
int sg_agent_mqtt_connect(void);

int sg_mqtt_msg_publish(char* msg_send, char* pub_topic);
int sg_mqtt_msg_subscribe(char* topic, int qos);

int sg_mqtt_init(void);
void sg_mqtt_exit(void);

int sg_create_sub_topic(void);
void sg_destroy_sub_topic(void);

void sg_set_mqtt_connect_flag(int flag);
int sg_get_mqtt_connect_flag(void);

char *get_topic_sub_dev_com(void);
char *get_topic_sub_dev_res(void);
char *get_topic_sub_ctai_com(void);
char *get_topic_sub_app_com(void);

char *get_topic_device_data_pub(void);
char *get_topic_device_reply_pub(void);
char *get_topic_device_request_pub(void);
char *get_topic_container_reply_pub(void);
char *get_topic_container_data_pub(void);
char *get_topic_app_reply_pub(void);
char *get_topic_app_data_pub(void);


#ifdef __cplusplus
}
#endif

#endif