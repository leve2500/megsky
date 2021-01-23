

/*=====================================================================
 * 文件：mqtt_pub.h
 *
 * 描述：负责mqtt处理 包括初始化，连接，发布，订阅等
 *
 * 作者：田振超			2020年9月27日17:09:49
 *
 * 修改记录：
 =====================================================================*/


#ifndef _MQTT_PUB_H_
#define _MQTT_PUB_H_

#include <stdbool.h>
#include "MQTTClient.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QOS         0
#define TIMEOUT     10000L

bool sg_mqtt_init(void);
void sg_mqtt_exit(void);

int sg_create_sub_topic(void);
int sg_mqtt_msg_arrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
int sg_mqtt_msg_pub(char* msg_send, char* pubtopic);
void sg_delivered(void *context, MQTTClient_deliveryToken dt);
void sg_connLost(void *context, char *cause);
void sg_destroy_sub_topic(void);

int sg_get_mqttclient_isconnected(void);

void sg_set_mqtt_connect_flag(int flag);
int sg_get_mqtt_connect_flag(void);

char *get_topic_sub_dev_com(void);
char *get_topic_sub_dev_res(void);
char *get_topic_sub_ctai_com(void);
char *get_topic_sub_app_com(void);

char *get_topic_device_data_pub(void);
char *get_topic_device_reply_pub(void);
char *get_topic_device_request_pub(void);
// char* get_topic_device_response_pub();
char *get_topic_container_reply_pub(void);
char *get_topic_container_data_pub(void);
char *get_topic_app_reply_pub(void);
char *get_topic_app_data_pub(void);


#ifdef __cplusplus
}
#endif 

#endif