#include <stdio.h>
#include <stdbool.h>
#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"
#include "mqtt_pub.h"
#include "sgdev_param.h"
#include "sgdev_queue.h"
#include "sgdev_struct.h"

char   m_ip[DATA_BUF_F32_SIZE];
uint16_t   m_port;
char   m_ver[24];
char   m_devid[DATA_BUF_F32_SIZE];
char   m_clientid[DATA_BUF_F32_SIZE];
char   m_user[24];
char   m_password[24];


char   m_top_data_sub_dev_com[DATA_BUF_F256_SIZE];
char   m_top_data_sub_dev_res[DATA_BUF_F256_SIZE];
char   m_top_data_sub_ctai_com[DATA_BUF_F256_SIZE];
char   m_top_data_sub_app_com[DATA_BUF_F256_SIZE];

char   m_device_reply_pub[DATA_BUF_F256_SIZE];
char   m_device_request_pub[DATA_BUF_F256_SIZE];
// char   m_device_response_pub[DATA_BUF_F256_SIZE];
char   m_device_data_pub[DATA_BUF_F256_SIZE];
char   m_container_reply_pub[DATA_BUF_F256_SIZE];
char   m_container_data_pub[DATA_BUF_F256_SIZE];
char   m_app_reply_pub[DATA_BUF_F256_SIZE];
char   m_app_data_pub[DATA_BUF_F256_SIZE];


int				m_connect_flag;	//mqtt连接标志

MQTTClient 			m_client;
volatile MQTTClient_deliveryToken deliveredtoken;


void sg_delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("\nMessage with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

void sg_connLost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("\ncause: %s\n", cause);

    m_connect_flag = DEVICE_OFFLINE;    //设置mqtt断开标志
    sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
}

int sg_get_mqttclient_isconnected(void)
{
    return MQTTClient_isConnected(m_client);
}

void sg_set_mqtt_connect_flag(int flag)
{
    m_connect_flag = flag;
    if (m_connect_flag == DEVICE_OFFLINE)
    {
        sg_set_dev_ins_flag(DEVICE_OFFLINE);
    }
}

int sg_get_mqtt_connect_flag(void)
{
    return m_connect_flag;
}

char *get_topic_sub_dev_com(void)
{
    return m_top_data_sub_dev_com;
}

char *get_topic_sub_dev_res(void)
{
    return m_top_data_sub_dev_res;
}

char *get_topic_sub_ctai_com(void)
{
    return m_top_data_sub_ctai_com;
}

char *get_topic_sub_app_com(void)
{
    return m_top_data_sub_app_com;
}

char *get_topic_device_data_pub(void)
{
    return m_device_data_pub;
}

char *get_topic_device_reply_pub(void)
{
    return m_device_reply_pub;
}

char *get_topic_device_request_pub(void)
{
    return m_device_request_pub;
}

char *get_topic_container_reply_pub(void)
{
    return m_container_reply_pub;
}

char *get_topic_container_data_pub(void)
{
    return m_container_data_pub;
}

char *get_topic_app_reply_pub(void)
{
    return m_app_reply_pub;
}

char *get_topic_app_data_pub(void)
{
    return m_app_data_pub;
}

bool sg_mqtt_init(void)
{
    bool bRet = false;
    m_connect_flag = DEVICE_OFFLINE;
    sg_dev_param_info_s param = sg_get_param();
    m_port = param.port;
    memcpy_s(m_ip, DATA_BUF_F32_SIZE, param.ip, strlen(param.ip));
    memcpy_s(m_clientid, DATA_BUF_F32_SIZE, param.clientid, strlen(param.clientid));
    memcpy_s(m_user, 24, param.user, strlen(param.user));
    memcpy_s(m_password, 24, param.password, strlen(param.password));
    char szTemp[DATA_BUF_F256_SIZE] = { 0 };
    sprintf(szTemp, "tcp://%s:%u", m_ip, m_port);
    printf("url == %s\n", szTemp);
    int rc;
    MQTTClient_create(&m_client, szTemp, m_clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(m_client, NULL, sg_connLost, sg_mqtt_msg_arrvd, sg_delivered);

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer; //要改为用户名密码方式

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if (strlen(m_user) != 0 && strlen(m_password) != 0) {
        conn_opts.username = m_user;
        conn_opts.password = m_password;
    }

    if ((rc = MQTTClient_connect(m_client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("\nFailed to connect, return code %d\n", rc);
        // 0成功
        // 1拒绝连接，协议版本不支持
        // 2标识符被拒绝
        // 3服务器不可用
        // 4用户名密码错误
        // 5未授权  
        m_connect_flag = DEVICE_OFFLINE;
        return bRet;
    }
    //订阅 
    if (MQTTCLIENT_SUCCESS != sg_create_sub_topic()) {
        m_connect_flag = DEVICE_OFFLINE;

    }
    m_connect_flag = DEVICE_ONLINE;
    // bRet = m_Thread.start();
    return bRet;
}

void sg_mqtt_exit(void)
{
    m_connect_flag = DEVICE_OFFLINE;
    sg_set_mqtt_connect_flag(DEVICE_OFFLINE);
    sg_destroy_sub_topic();

    MQTTClient_disconnect(m_client, 10000);
    MQTTClient_destroy(&m_client);
}

int sg_mqtt_msg_pub(char*msg_send, char*pubtopic)
{
    int sendFlag = 0;
    int rc;
    int msglen;
    // char msg_send[MSG_ARRVD_MAX_LEN]={0};
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    // char pubtopic[DATA_BUF_F256_SIZE] = {0};  
    msglen = (int)strlen(msg_send);
    pubmsg.payload = msg_send;
    pubmsg.payloadlen = msglen;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    // if(MQTTCLIENT_SUCCESS != MQTTClient_publishMessage(m_client, pubtopic, &pubmsg, &token)){
    //     return 1;
    // }
    sendFlag = MQTTClient_publishMessage(m_client, pubtopic, &pubmsg, &token);
    printf("\nPublishing topic %s: %s,len is %d.\n\n", pubtopic, (char *)pubmsg.payload, pubmsg.payloadlen);
    rc = MQTTClient_waitForCompletion(m_client, token, TIMEOUT);
    printf("\nMessage with delivery token %d delivered\n\n", token);
    return sendFlag;
}

int sg_mqtt_msg_arrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char*payloadptr;
    payloadptr = (char*)message->payload;


    printf("\nMessage arrived\n");
    printf("topic: %s\n", topicName);

    mqtt_data_info_s *item = NULL;
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    printf("payloadptr == %s.\n", payloadptr);
    // for(i=0; i<message->payloadlen; i++)
    // {
    //     putchar(*payloadptr++);
    // }
    // printf("\n\n");

    //memcpy_s(item->pubtopic,256,topicName,topicLen);
    memcpy_s(item->msg_send, MSG_ARRVD_MAX_LEN, payloadptr, message->payloadlen);
    sprintf(item->pubtopic, "%s", topicName);
    // sprintf(item->msg_send,"%s",payloadptr);

    printf("item->msg_send == %s.\n", item->msg_send);

    sg_push_unpack_item(item);          //入列

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}


int sg_create_sub_topic(void)
{
    int ret = MQTTCLIENT_SUCCESS;
    sg_dev_param_info_s param = sg_get_param();

    memcpy_s(m_ver, 24, param.mqtttopicversion, strlen(param.mqtttopicversion));
    memcpy_s(m_devid, 32, param.devid, strlen(param.devid));

    sprintf(m_device_reply_pub, "/%s/%s/device/reply", m_ver, m_devid); 			//用于对平台发送的设备控制命令的应答
    sprintf(m_device_request_pub, "/%s/%s/device/request", m_ver, m_devid); 		//用于终端向平台发送设备管理相关的请求命令， 如请求连接等
    sprintf(m_device_data_pub, "/%s/%s/device/data", m_ver, m_devid); 			//用于终端向平台主动上报设备相关的状态、事件 等

    sprintf(m_container_reply_pub, "/%s/%s/container/reply", m_ver, m_devid); 	//用于对平台的容器控制请求命令的应答
    sprintf(m_container_data_pub, "/%s/%s/container/data", m_ver, m_devid); 		//用于终端向平台主动上报容器相关的状态、事件 等

    sprintf(m_app_reply_pub, "/%s/%s/app/reply", m_ver, m_devid); 				//用于对平台发送的应用控制请求命令的应答
    sprintf(m_app_data_pub, "/%s/%s/app/data", m_ver, m_devid); 				    //用于对平台发送的应用状态

    sprintf(m_top_data_sub_dev_com, "/%s/%s/device/command", m_ver, m_devid);    //用于平台向终端发送设备控制命令，如设备升级、控制设备等
    printf("\nSubscribing to topic :%s\n", m_top_data_sub_dev_com);
    ret = MQTTClient_subscribe(m_client, m_top_data_sub_dev_com, QOS);

    sprintf(m_top_data_sub_dev_res, "/%s/%s/device/response", m_ver, m_devid); 	//用于对终端发送的设备管理相关的请求命令的 应答
    printf("\nSubscribing to topic :%s\n", m_top_data_sub_dev_res);
    ret = MQTTClient_subscribe(m_client, m_top_data_sub_dev_res, QOS);

    sprintf(m_top_data_sub_ctai_com, "/%s/%s/container/command", m_ver, m_devid);//用于平台向终端发送的容器控制请求命令，如容器安装、启动、停止等
    printf("\nSubscribing to topic :%s\n", m_top_data_sub_ctai_com);
    ret = MQTTClient_subscribe(m_client, m_top_data_sub_ctai_com, QOS);

    sprintf(m_top_data_sub_app_com, "/%s/%s/app/command", m_ver, m_devid); 		//用于平台向终端发送应用控制请求命令，如应用安装、启动、停止等
    printf("\nSubscribing to topic :%s\n", m_top_data_sub_app_com);
    ret = MQTTClient_subscribe(m_client, m_top_data_sub_app_com, QOS);
    return ret;
}

void sg_destroy_sub_topic(void)
{
    MQTTClient_unsubscribe(m_client, m_top_data_sub_dev_com);
    MQTTClient_unsubscribe(m_client, m_top_data_sub_dev_res);
    MQTTClient_unsubscribe(m_client, m_top_data_sub_ctai_com);
    MQTTClient_unsubscribe(m_client, m_top_data_sub_app_com);
}


