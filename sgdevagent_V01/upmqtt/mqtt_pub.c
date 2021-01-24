#include <stdio.h>
#include <stdbool.h>
#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"
#include "mqtt_pub.h"
#include "sgdev_struct.h"
#include "sgdev_debug.h"
#include "sgdev_param.h"
#include "sgdev_queue.h"

#define     SG_KEEP_ALIVE_INTERVAL (20)
#define     SG_VER_SIZE (24)

mqtt_connect_data_s g_mqtt_connect_flag;

uint16_t    g_port;
char        g_ip[DATA_BUF_F32_SIZE];
char        g_ver[SG_VER_SIZE];
char        g_devid[DATA_BUF_F32_SIZE];
char        g_clientid[DATA_BUF_F32_SIZE];
char        g_user[DATA_BUF_F32_SIZE];
char        g_password[DATA_BUF_F32_SIZE];

char        g_top_data_sub_dev_com[DATA_BUF_F256_SIZE];
char        g_top_data_sub_dev_res[DATA_BUF_F256_SIZE];
char        g_top_data_sub_ctai_com[DATA_BUF_F256_SIZE];
char        g_top_data_sub_app_com[DATA_BUF_F256_SIZE];

char        g_device_reply_pub[DATA_BUF_F256_SIZE];
char        g_device_request_pub[DATA_BUF_F256_SIZE];
char        g_device_data_pub[DATA_BUF_F256_SIZE];
char        g_container_reply_pub[DATA_BUF_F256_SIZE];
char        g_container_data_pub[DATA_BUF_F256_SIZE];
char        g_app_reply_pub[DATA_BUF_F256_SIZE];
char        g_app_data_pub[DATA_BUF_F256_SIZE];

MQTTClient      g_client = NULL;
sg_mqtt_msg_arrived_cb g_mqtt_msg_arrived_cb = NULL;
sg_mqtt_conn_lost_cb g_mqtt_conn_lost_cb = NULL;

volatile MQTTClient_deliveryToken g_deliveredtoken;
MQTTClient_connectOptions g_conn_opts = MQTTClient_connectOptions_initializer;

void sg_connLost(void *context, char *cause);
int sg_mqtt_msg_arrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void sg_delivered(void *context, MQTTClient_deliveryToken dt);
void sg_agent_mqtt_disconnect(void);
void sg_agent_mqtt_destroy(void);
int sg_get_mqttclient_isconnected(void);

void sg_delivered(void *context, MQTTClient_deliveryToken dt)
{
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, Message with token value %d delivery confirmed\n", dt);
    g_deliveredtoken = dt;
}

void sg_connLost(void *context, char *cause)
{
    printf("\n cause: %s\n", cause);
    SGDEV_INFO(SYSLOG_LOG, EG_MODULE, "sg_connLost(cause = %s).\n", cause);
    g_mqtt_connect_flag.mqtt_connect_flag = DEVICE_OFFLINE;
    sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
}

int sg_get_mqttclient_isconnected(void)
{
    return MQTTClient_isConnected(g_client);
}

int sg_mqtt_msg_arrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int ret = VOS_OK;
    char *payloadptr = NULL;
    payloadptr = (char*)message->payload;

    printf("ceshi*************\n<topic>: %s.**********\n", topicName);
    printf("ceshi*************\n<payloadptr> == %s.**********\n", payloadptr);

    mqtt_data_info_s *item = NULL;
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    if (item == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "mqtt arrvd malloc item failed.\n");
        return VOS_ERR;
    }

    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    memcpy_s(item->msg_send, MSG_ARRVD_MAX_LEN, payloadptr, message->payloadlen);
    sprintf(item->pubtopic, "%s", topicName);

    if (item->msg_send == NULL || item->pubtopic == NULL) {
        (void)VOS_Free(item);
        item = NULL;
        return VOS_ERR;
    }

    sg_push_unpack_item(item);          //入列
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return ret;
}

void sg_set_mqtt_connect_flag(int flag)
{
    g_mqtt_connect_flag.mqtt_connect_flag = flag;
    if (g_mqtt_connect_flag.mqtt_connect_flag == DEVICE_OFFLINE) {
        sg_set_dev_ins_flag(DEVICE_OFFLINE);
    }
}

mqtt_connect_data_s sg_get_mqtt_connect_flag(void)
{
    if (g_mqtt_connect_flag.mqtt_connect_flag == DEVICE_ONLINE) {
        if (!sg_get_mqttclient_isconnected()) {
            g_mqtt_connect_flag.mqtt_connect_flag = DEVICE_OFFLINE;
        }
    }
    return g_mqtt_connect_flag;
}

int sg_agent_mqtt_init(char *server_uri, char* client_id)
{
    int mqtt_ret;
    int ret = VOS_OK;
    sg_set_mqtt_connect_flag(DEVICE_OFFLINE);

    if (serveruri == NULL || client_id == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "MQTT Init param failed.\n");
        return VOS_ERR;
    }

    g_conn_opts.keepAliveInterval = SG_KEEP_ALIVE_INTERVAL;
    g_conn_opts.cleansession = 1;
    if (strlen(g_user) != 0 && strlen(g_password) != 0) {
        g_conn_opts.username = g_user;
        g_conn_opts.password = g_password;
    }

    mqtt_ret = MQTTClient_create(&g_client, server_uri, client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "MQTTClient_create failed(server_uri = %s,clientid = %s,ret = %d).\n",
            server_uri, client_id, mqtt_ret);
        return VOS_ERR;
    }

    SGDEV_INFO(SYSLOG_LOG, EG_MODULE, "MQTTClient_create success(server_uri = %s,clientid = %s).\n",
        server_uri, client_id);
    mqtt_ret = MQTTClient_setCallbacks(g_client, NULL, sg_connLost, sg_mqtt_msg_arrvd, sg_delivered);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        sg_agent_mqtt_destroy();
        return VOS_ERR;
    }
    SGDEV_INFO(SYSLOG_LOG, EG_MODULE, "sg_agent_mqtt_init success(server_uri = %s,clientid = %s).\n",
        server_uri, client_id);
    return ret;

}

int sg_agent_mqtt_connect(void)
{
    int mqtt_ret = VOS_OK;
    int ret = VOS_OK;
    sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
    if (g_client == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "mqtt client id handle invalid).\n");
        return VOS_ERR;
    }

    SGDEV_INFO(SYSLOG_LOG, EG_MODULE, "mqtt connecting ....\n");
    mqtt_ret = MQTTClient_connect(g_client, &conn_opts));
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        SGDEV_WARN(SYSLOG_LOG, EG_MODULE, "mqtt connect failed(ret = %d)).\n", mqtt_ret);
        // 0成功
        // 1拒绝连接，协议版本不支持
        // 2标识符被拒绝
        // 3服务器不可用
        // 4用户名密码错误
        // 5未授权
        sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
        return VOS_ERR;
    }
    SGDEV_INFO(SYSLOG_LOG, EG_MODULE, "mqtt connect  success.\n");
    g_mqtt_connect_flag.mqtt_connect_flag = DEVICE_ONLINE;
    return ret;
}

void sg_agent_mqtt_disconnect(void)
{
    int mqtt_ret;
    if (g_client != NULL) {
        mqtt_ret = MQTTClient_disconnect(g_client, 10000);
        sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
        if (mqtt_ret != MQTTCLIENT_SUCCESS) {
            SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "mqtt disconnect error).\n");
        }
    }
}

void sg_agent_mqtt_destroy(void)
{
    int mqtt_ret;
    if (g_client != NULL) {
        mqtt_ret = MQTTClient_destroy(&g_client);
        SGDEV_INFO(SYSLOG_LOG, EG_MODULE, "mqtt destroy.\n");
        if (mqtt_ret != MQTTCLIENT_SUCCESS) {
            SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "mqtt destroy error).\n");
            g_client = NULL;
        }
    }
}

int sg_mqtt_msg_publish(char* msg_send, char* pub_topic)
{
    int mqtt_ret = 0;
    int rc = VOS_OK;
    int msglen = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    if (msg_send == NULL || pub_topic == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "mqtt publish param msg_send or pubtopicinvalid.\n");
        return VOS_ERR;
    }

    if (g_client == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "mqtt client id handle invalid).\n");
        return VOS_ERR;
    }

    msglen = (int)strlen(msg_send) + 1;
    pubmsg.payload = msg_send;
    pubmsg.payloadlen = msglen;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    mqtt_ret = MQTTClient_publishMessage(g_client, pub_topic, &pubmsg, &token);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        SGDEV_WARN(SYSLOG_LOG, EG_MODULE, "mqtt publish failed(ret = %d,pub_topic = %s,msg = %s ,len = %d)).\n",
            mqtt_ret, pub_topic, (char *)pubmsg.payload, pubmsg.payloadlen);
        return VOS_ERR;
    }

    mqtt_ret = MQTTClient_waitForCompletion(g_client, token, TIMEOUT);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "MQTTClient_waitForCompletion ret = %d).\n", mqtt_ret);
        rc = VOS_ERR;
    }
    SGDEV_INFO(SYSLOG_LOG, EG_MODULE, "Message with delivery token %d delivered.\n", token);
    return rc;
}

int sg_mqtt_msg_subscribe(char* topic, int qos)
{
    int mqtt_ret;
    int ret = VOS_OK;
    if (topic == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "mqtt subscribe topic invalid.\n");
        return VOS_ERR;
    }

    if (g_client == NULL) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "mqtt subscribe client id handle invalid).\n");
        return VOS_ERR;
    }

    mqtt_ret = MQTTClient_subscribe(g_client, topic, qos);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        SGDEV_WARN(SYSLOG_LOG, EG_MODULE, "mqtt subscribe failed(ret = %d,pub_topic = %s)).\n",mqtt_ret, pub_topic);
        return VOS_ERR;
    }
    SGDEV_INFO(SYSLOG_LOG, EG_MODULE, "mqtt subscribe succeed(pub_topic = %s)).\n", pub_topic);
    return ret;
}

int sg_mqtt_init(void)
{
    int rc = VOS_OK;
    bool b_ret = false;
    sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
    char server_uri[DATA_BUF_F256_SIZE] = { 0 };

    sg_dev_param_info_s param = sg_get_param();

    g_port = param.port;
    memcpy_s(g_ip, DATA_BUF_F32_SIZE, param.ip, strlen(param.ip));
    memcpy_s(g_clientid, DATA_BUF_F32_SIZE, param.clientid, strlen(param.clientid));
    memcpy_s(g_user, DATA_BUF_F32_SIZE, param.user, strlen(param.user));
    memcpy_s(g_password, DATA_BUF_F32_SIZE, param.password, strlen(param.password));
    sprintf_s(server_uri, DATA_BUF_F256_SIZE, "tcp://%s:%u", g_ip, g_port);
    SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE,"mqtt connect url: %s\n", server_uri);

    if (sg_agent_mqtt_init() != VOS_OK) {
        SGDEV_ERROR(SYSLOG_LOG, EG_MODULE, "sg_agent_mqtt_init failed.\n");
        return VOS_ERR;
    }

    if (sg_agent_mqtt_connect() == VOS_OK) {    // 初始化成功后尝试一次连接
        if (sg_create_sub_topic == VOS_OK) {
            sg_set_mqtt_connect_flag(DEVICE_ONLINE);
        }
    }
    return rc;
}

void sg_mqtt_exit(void)
{
    g_connect_flag = DEVICE_OFFLINE;
    sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
    sg_destroy_sub_topic();
    (void)sg_agent_mqtt_disconnect();
    (void)sg_agent_mqtt_destroy();
}

int sg_create_sub_topic(void)
{
    int ret = VOS_OK;
    sg_dev_param_info_s param = sg_get_param();

    memcpy_s(g_ver, SG_VER_SIZE, param.mqtttopicversion, strlen(param.mqtttopicversion));
    memcpy_s(g_devid, DATA_BUF_F32_SIZE, param.devid, strlen(param.devid));

    sprintf(g_device_reply_pub, "/%s/%s/device/reply", g_ver, g_devid); 			// 用于对平台发送的设备控制命令的应答
    sprintf(g_device_request_pub, "/%s/%s/device/request", g_ver, g_devid); 		// 用于终端向平台发送设备管理相关的请求命令， 如请求连接等
    sprintf(g_device_data_pub, "/%s/%s/device/data", g_ver, g_devid); 			    // 用于终端向平台主动上报设备相关的状态、事件 等

    sprintf(g_container_reply_pub, "/%s/%s/container/reply", g_ver, g_devid); 	    // 用于对平台的容器控制请求命令的应答
    sprintf(g_container_data_pub, "/%s/%s/container/data", g_ver, g_devid); 		// 用于终端向平台主动上报容器相关的状态、事件 等

    sprintf(g_app_reply_pub, "/%s/%s/app/reply", g_ver, g_devid); 				    // 用于对平台发送的应用控制请求命令的应答
    sprintf(g_app_data_pub, "/%s/%s/app/data", g_ver, g_devid); 				    // 用于对平台发送的应用状态

    sprintf(g_top_data_sub_dev_com, "/%s/%s/device/command", g_ver, g_devid);       // 用于平台向终端发送设备控制命令，如设备升级、控制设备等
    sg_destroy_sub_topic();

    if (sg_mqtt_msg_subscribe(g_top_data_sub_dev_com, QOS) != VOS_OK) {
        ret = VOS_ERR;
    }

    sprintf(g_top_data_sub_dev_res, "/%s/%s/device/response", g_ver, g_devid); 	    // 用于对终端发送的设备管理相关的请求命令的 应答
    if (sg_mqtt_msg_subscribe(g_top_data_sub_dev_res, QOS) != VOS_OK) {
        ret = VOS_ERR;
    }

    sprintf(g_top_data_sub_ctai_com, "/%s/%s/container/command", g_ver, g_devid);   // 用于平台向终端发送的容器控制请求命令，如容器安装、启动、停止等
    if (sg_mqtt_msg_subscribe(g_top_data_sub_ctai_com, QOS) != VOS_OK) {
        ret = VOS_ERR;
    }

    sprintf(g_top_data_sub_app_com, "/%s/%s/app/command", g_ver, g_devid); 		    // 用于平台向终端发送应用控制请求命令，如应用安装、启动、停止等
    if (sg_mqtt_msg_subscribe(g_top_data_sub_app_com, QOS) != VOS_OK) {
        ret = VOS_ERR;
    }

    return ret;
}

void sg_destroy_sub_topic(void)
{
    MQTTClient_unsubscribe(g_client, g_top_data_sub_dev_com);
    MQTTClient_unsubscribe(g_client, g_top_data_sub_dev_res);
    MQTTClient_unsubscribe(g_client, g_top_data_sub_ctai_com);
    MQTTClient_unsubscribe(g_client, g_top_data_sub_app_com);
}

char *get_topic_sub_dev_com(void)
{
    return g_top_data_sub_dev_com;
}

char *get_topic_sub_dev_res(void)
{
    return g_top_data_sub_dev_res;
}

char *get_topic_sub_ctai_com(void)
{
    return g_top_data_sub_ctai_com;
}

char *get_topic_sub_app_com(void)
{
    return g_top_data_sub_app_com;
}

char *get_topic_device_data_pub(void)
{
    return g_device_data_pub;
}

char *get_topic_device_reply_pub(void)
{
    return g_device_reply_pub;
}

char *get_topic_device_request_pub(void)
{
    return g_device_request_pub;
}

char *get_topic_container_reply_pub(void)
{
    return g_container_reply_pub;
}

char *get_topic_container_data_pub(void)
{
    return g_container_data_pub;
}

char *get_topic_app_reply_pub(void)
{
    return g_app_reply_pub;
}

char *get_topic_app_data_pub(void)
{
    return g_app_data_pub;
}


