/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description : sgdevagent public header file
*/


#ifndef __PUBLIC_STRUCT_H__
#define __PUBLIC_STRUCT_H__

#include "jansson.h"

#ifdef __cplusplus
extern "C" {  // only need to export C interface if used by C++ source code
#endif

#define MSG_ARRVD_MAX_LEN   (3 * 1024)

#define REBOOT_EDGE_RESET 1
#define REBOOT_EDGE_SET 0       // 组件重启宏

#define MODE_MQTT 1
#define MODE_RMTMAN 0

 /* 基本数据类型和常量定义 */

#define TYPE_REQUEST 1
#define TYPE_REPLY 2

#define DEVICE_ONLINE       1
#define DEVICE_OFFLINE      0

#define DEV_INFO_MSG_BUFFER_LEN 1024
#define DEV_INFO_MSG_BUFFER_NUM 16

#define HEART_MAX_COUNT 3

#define TIME_LOCAL 0
#define TIME_UTC 1  // 定义使用UTC时间

#define FLAG_NULL       0
#define FLAG_FAULT      1
#define FLAG_RECOVER    2

#define DATA_BUF_F16_SIZE 16
#define DATA_BUF_F32_SIZE 32   // 定义数组长度宏定义
#define DATA_BUF_F64_SIZE 64
#define DATA_BUF_F128_SIZE 128
#define DATA_BUF_F256_SIZE 256
#define DATA_BUF_F512_SIZE 512


#define MSECOND 1000000
#define TEN_MSECOND 100000

#define JSON_BUF_SIZE 256

#define CPU_USAGE 1
#define MEM_USAGE 2
#define STORAGE_USAGE 3


#define UPGRADEPACH 0
#define UPGRADEHOST 3

// 表4 应答 code 编码格式
#define CODE_NULL           0   // null
#define REQUEST_SUCCESS     200 // 请求成功
#define REQUEST_WAIT        202 // 请求被接受，但是服务器未处理完
#define REQUEST_FAILED      400 // 请求失败
#define REQUEST_CERTIFICATE 401 // 请求未认证/认证错误
#define REQUEST_REFUSE      403 // 请求被拒绝
#define REQUEST_NOTEXITS    404 // 请求的资源不存在
#define REQUEST_TIMEOUT     408 // 请求超出了服务器的等待时间
#define OTHER_ERROR         500 // 其他错误

// 表28 设备资源告警事件
#define DEV_CPU_UTIL_EXCEEDS   1001 // cpu 利用率超过阀值  级别：一般
#define DEV_CPU_UTIL_RESTORE   1002 // cpu 利用率超过阀值恢复 级别：一般
#define DEV_MEM_UTIL_EXCEEDS   1003 // 内存 使用率超过阀值 级别：一般
#define DEV_MEM_UTIL_RESTORE   1004 // 内存 使用率超过阀值恢复 级别：一般
#define DEV_DISK_UTIL_EXCEEDS  1005 // 磁盘 空间使用率超过阀值 级别：一般

// 表29 设备安全事件
#define OPEN_ILLEGAL_PORT  2001 // 开放非法端口 级别：紧急
#define NET_OUTLINK_EVENT  2002 // 网络外联事件 级别：紧急
#define USB_ILLEGAL_INSERT 2003 // USB 存储设备非法插入 级别：紧急
#define PORT_OCCUPY        2004 // 串口占用     级别：一般

// 表30 设备故障事件
#define FAN_STOP_RUNNING   3001 // 散热风扇停止运行 级别：一般
#define EXT_DEV_ABNORMAL   3002 // 外接装置异常 级别：一般
#define HIGH_TEMP          3003 // 超高温 级别：紧急
#define HIGH_TEMP_RESTORE  3004 // 超高温恢复 级别：一般
#define LOW_TEMP           3005 // 超低温 级别：紧急
#define LOW_TEMP_RESTORE   3006 // 超低温恢复 级别：一般
#define POWER_DOWN         3007 // 掉电 级别：紧急
#define OPEN_THE_LID       3008 // 上盖开盖 级别：紧急
#define END_CAP_OPEN       3009 // 端盖开盖 级别：紧急
#define COMPONENT_RESTART  3010 // 重要组件异常重启（在 msg 中填写组件名称） 级别：紧急
#define TERMINAL_RESTART   3011 // 终端重启上报 级别：紧急

// 表56 事件编号 4xxx：容器告警事件
#define CON_CPU_UTIL_EXCEEDS   4001 // 容器cpu利用率超过阀值  级别：一般
#define CON_CPU_UTIL_RESTORE   4002 // 容器cpu利用率超过阀值恢复 级别：一般
#define CON_MEM_UTIL_EXCEEDS   4003 // 容器内存使用率超过阀值 级别：一般
#define CON_MEM_UTIL_RESTORE   4004 // 容器内存使用率超过阀值恢复 级别：一般
#define CON_DISK_UTIL_EXCEEDS  4005 // 容器磁盘空间使用率超过阀值 级别：一般
#define CON_RESTART_ABNORMAL   4006 // 容器异常重启 级别：严重
#define CONTAINER_FAILURE      4007 // 容器故障 级别：严重

// 表80 event 应用告警事件
#define APP_CPU_UTIL_EXCEEDS   5001 // 应用cpu利用率超过阀值  级别：一般
#define APP_CPU_UTIL_RESTORE   5002 // 应用cpu利用率超过阀值恢复 级别：一般
#define APP_MEM_UTIL_EXCEEDS   5003 // 应用内存使用率超过阀值 级别：一般
#define APP_MEM_UTIL_RESTORE   5004 // 应用内存使用率超过阀值恢复 级别：一般
#define APP_UPGRADE            5005 // 应用升级 级别：一般
#define APP_RESTART_ABNORMAL   5006 // App异常重启 级别：一般

/* MQTT消息通用部分结构体 */
typedef struct mqtt_header {
    char        token[40];
    char        timestamp[32];
}mqtt_header_s;

typedef struct mqtt_data_info {
    char msg_send[MSG_ARRVD_MAX_LEN];
    char pubtopic[256];
}mqtt_data_info_s;


/********************type字段说明：***************************
所有下发命令以“CMD_”开头
所有上报事件均以“EVENT_”或“REP_”开头
************************************************************/
#define EVENT_LINKUP		"EVENT_LINKUP"       // 设备接入               **
#define EVENT_LINKDOWN		"EVENT_LINKDOWN"     // 设备主动断开上报        **
#define EVENT_HEARTBEAT		"EVENT_HEARTBEAT"    // 设备心跳请求            **
#define CMD_SYS_UPGRADE		"CMD_SYS_UPGRADE"    // 设备升级命令
#define CMD_STATUS_QUERY	"CMD_STATUS_QUERY"   // 设备升级状态查询命令
#define REP_SYS_STATUS		"REP_SYS_STATUS"     // 设备状态上报            **
#define CMD_SYS_STATUS		"CMD_SYS_STATUS"     // 设备状态查询命令
#define CMD_INFO_QUERY		"CMD_INFO_QUERY"     // 设备信息查询命令
#define CMD_SYS_SET_CONFIG	"CMD_SYS_SET_CONFIG" // 设备管理参数修改命令
#define CMD_DATETIME_SYN	"CMD_DATETIME_SYN"   // 设备时间同步命令
#define EVENT_SYS_ALARM 	"EVENT_SYS_ALARM"    // 设备事件上报            **
#define CMD_SYS_LOG			"CMD_SYS_LOG"        // 设备日志召回
#define CMD_CTRL			"CMD_CTRL"           // 设备控制命令
#define CMD_CON_INSTALL		"CMD_CON_INSTALL"
#define CMD_CON_START		"CMD_CON_START"
#define CMD_CON_STOP		"CMD_CON_STOP"
#define CMD_CON_REMOVE		"CMD_CON_REMOVE"
#define CMD_CON_SET_CONFIG	"CMD_CON_SET_CONFIG"
#define CMD_CON_GET_CONFIG	"CMD_CON_GET_CONFIG"
#define CMD_CON_STATUS		"CMD_CON_STATUS"
#define REP_CON_STATUS		"REP_CON_STATUS"
#define EVENT_CON_ALARM		"EVENT_CON_ALARM"
#define CMD_CON_UPGRADE		"CMD_CON_UPGRADE"
#define REP_JOB_RESULT		"REP_JOB_RESULT"
#define CMD_CON_LOG			"CMD_CON_LOG"
#define CMD_APP_INSTALL		"CMD_APP_INSTALL"
#define CMD_APP_START		"CMD_APP_START"
#define CMD_APP_STOP		"CMD_APP_STOP"
#define CMD_APP_REMOVE		"CMD_APP_REMOVE"
#define CMD_APP_ENABLE		"CMD_APP_ENABLE"
#define CMD_APP_UNENABLE	"CMD_APP_UNENABLE"
#define CMD_APP_SET_CONFIG	"CMD_APP_SET_CONFIG"
#define CMD_APP_GET_CONFIG	"CMD_APP_GET_CONFIG"
#define CMD_APP_STATUS		"CMD_APP_STATUS"
#define REP_APP_STATUS		"REP_APP_STATUS"
#define EVENT_APP_ALARM		"EVENT_APP_ALARM"
#define CMD_APP_UPGRADE		"CMD_APP_UPGRADE"
#define CMD_APP_LOG			"CMD_APP_LOG"

typedef enum {
    TASK_EXE_STATUS_NULL = 0,
    TASK_EXE_DEV_UP = 1,                        // 设备升级状态应答
    TASK_EXE_CONTAINER_INSTALL = 2,             // 容器安装状态查询应答
    TASK_EXE_CONTAINER_UP = 3,                  // 容器升级状态查询应答
    TASK_EXE_APP_INSTALL = 4,                   // 应用安装状态查询应答
    TASK_EXE_APP_UP = 5,                        // 应用升级状态查询应答
    TASK_NUM
} exe_task_type_tag_t;

typedef enum {
    STATUS_NULL = 0,
    STATUS_PRE_DOWNLOAD = 1,
    STATUS_EXE_DOWNLOAD = 2,
    STATUS_PRE_INSTALL = 3,
    STATUS_EXE_INSTALL = 4,
    STATUS_FINISH_INSTALL = 5,
    STATUS_NUM
} up_status_tag_t;

typedef enum {
    QUEUE_NULL = 0,
    QUEUE_UNPACK = 1,
    QUEUE_PACK = 2,
    QUEUE_RESULT = 3,
    QUEUE_DEVTASK = 4,
    QUEUE_CONTAINERTASK = 5,
    QUEUE_APPTASK = 6,
    QUEUE_NUM
} queue_tag_t;

typedef enum {
    TAG_CMD_NULL = 0,
    TAG_EVENT_LINKUP = 1,                   // 设备接入
    TAG_EVENT_LINKDOWN,                     // 设备主动断开上报
    TAG_EVENT_HEARTBEAT,                    // 设备心跳请求
    TAG_CMD_SYS_UPGRADE,                    // 设备升级命令
    TAG_CMD_STATUS_QUERY,                   // 设备升级状态查询,设备升级状态查询应答
    TAG_CMD_STATUS_QUERY_CONT_INS,          // 容器安装状态查询命令,容器安装状态查询应答
    TAG_CMD_STATUS_QUERY_CONT_UP,		    // 容器升级状态查询命令,容器升级状态查询应答
    TAG_CMD_STATUS_QUERY_APP_INS,		    // 应用安装状态查询命令,应用安装状态查询应答
    TAG_CMD_STATUS_QUERY_APP_UP,		    // 应用升级状态查询命令,应用升级状态查询应答
    TAG_REP_SYS_STATUS,                     // 设备状态上报
    TAG_CMD_SYS_STATUS,                     // 设备状态查询
    TAG_CMD_INFO_QUERY,                     // 设备信息查询命令,设备信息查询命令应答
    TAG_CMD_SYS_SET_CONFIG,                 // 设备管理参数修改命令,设备管理参数修改命令应答
    TAG_CMD_DATETIME_SYN,                   // 设备时间同步命令,设备时间同步命令应答
    TAG_EVENT_SYS_ALARM,                    // 设备事件上报
    TAG_CMD_SYS_LOG,                        // 设备日志召回,设备日志召回应答
    TAG_CMD_CTRL,                           // 设备控制命令,设备控制命令应答
    TAG_CMD_CON_INSTALL,                    // 容器安装,容器安装控制应答
    TAG_CMD_CON_START,                      // 启动容器,容器启动控制命令应答
    TAG_CMD_CON_STOP,                       // 容器停止,容器停止控制应答
    TAG_CMD_CON_REMOVE,                     // 删除容器,容器删除控制应答
    TAG_CMD_CON_SET_CONFIG,                 // 容器配置修改,容器配置修改应答
    TAG_CMD_CON_GET_CONFIG,                 // 容器配置查询,容器配置查询应答
    TAG_CMD_CON_STATUS,                     // 容器状态查询,容器状态查询应答
    TAG_REP_CON_STATUS,                     // 容器状态上报
    TAG_EVENT_CON_ALARM,                    // 容器事件上报
    TAG_CMD_CON_UPGRADE,                    // 容器升级,容器升级命令应答
    TAG_REP_JOB_RESULT,                     // 容器升级结果上报,应用安装结果上报,应用升级结果上报
    TAG_CMD_CON_LOG,                        // 容器日志召回,容器日志召回应答
    TAG_CMD_APP_INSTALL,                    // 应用安装控制命令,应用安装控制命令应答
    TAG_CMD_APP_START,                      // 应用启动
    TAG_CMD_APP_STOP,                       // 应用停止
    TAG_CMD_APP_REMOVE,                     // 应用卸载
    TAG_CMD_APP_ENABLE,                     // 应用使能
    TAG_CMD_APP_UNENABLE,                   // 应用去使能
    TAG_CMD_APP_SET_CONFIG,                 // 应用配置修改命令,应用配置修改命令应答
    TAG_CMD_APP_GET_CONFIG,                 // 应用配置查询命令,应用配置查询应答
    TAG_CMD_APP_STATUS,                     // 应用状态查询命令,应用状态查询应答
    TAG_REP_APP_STATUS,                     // 应用状态上报
    TAG_EVENT_APP_ALARM,                    // 应用事件上报
    TAG_CMD_APP_UPGRADE,                    // 应用升级命令,应用升级命令应答
    TAG_CMD_APP_LOG                         // 应用日志召回,应用日志召回应答
} cmd_tag_t;


// 请求报文头
typedef struct mqtt_request_header {
    int32_t          mid;
    char             deviceId[DATA_BUF_F64_SIZE];
    char             timestamp[DATA_BUF_F32_SIZE];
    int32_t          expire;
    char             type[DATA_BUF_F32_SIZE];
    json_t*          param;
}mqtt_request_header_s;

// 应答报文头
typedef struct mqtt_reply_header {
    int32_t          mid;
    char             deviceId[DATA_BUF_F64_SIZE];
    char             timestamp[DATA_BUF_F32_SIZE];
    char             type[DATA_BUF_F32_SIZE];
    json_t*          param;
    uint16_t         code;
    char             msg[DATA_BUF_F256_SIZE];
}mqtt_reply_header_s;

// A.1  设备信息字段：dev
typedef struct dev_info {
    char             devType[DATA_BUF_F64_SIZE];     // 终端类型
    char             devName[DATA_BUF_F64_SIZE];     // 终端名称
    char             mfgInfo[DATA_BUF_F64_SIZE];     // 终端厂商信息
    char             devStatus[DATA_BUF_F64_SIZE];   // 终端状态
    char             hardVersion[DATA_BUF_F64_SIZE]; // 终端硬件版本号
}dev_info_s;

// A.2 CPU 信息字段:cpu
typedef struct cpu_info {
    uint8_t          cpus;                          // CPU 核数
    double           frequency;                     // CPU 主频 GHz 为单位
    uint32_t         cache;                         // CPU 缓存以 MB/核为单位
    char             arch[DATA_BUF_F64_SIZE];       // CPU 架构
    uint32_t         cpuLmt;                        // CPU 监控阈值
}cpu_info_s;

// A.3内存信息字段:mem
typedef struct mem_info {
    uint32_t         phy;                     // 物理内存，以M为单位
    uint32_t         virt;                    // 虚拟内存，以M为单位
    uint8_t          memLmt;                  // 内存监控阈值，例如50表示50%
}mem_info_s;

// A.4  硬盘信息字段:disk
typedef struct disk_info {
    uint32_t         disk;                    // 磁盘空间，以M为单位
    uint8_t          diskLmt;                 // 磁盘监控阈值，例如50表示50%
}disk_info_s;

// A.5  外接设备信息字段:links
typedef struct link_info {
    char            type[DATA_BUF_F32_SIZE];         // 接口的类型，如以太网口,4G,"Ethernet","4G"
    char            name[DATA_BUF_F32_SIZE];         // 接口的名称如为以太网口，4G,则形如"eth1","ppp-0"
    char            id[DATA_BUF_F32_SIZE];           // 接口的ID,主要用于HPLC和4G等外部模块
    char            mac[DATA_BUF_F32_SIZE];          // 如接口为以太网，4G，则添加mac地址，形如"B8-85-74-15-A5-3E"
}link_info_s;
// A.6  操作系统信息字段:os
typedef struct os_info {
    char            distro[DATA_BUF_F64_SIZE];        // 操作系统的名称，如"Ubuntu""Redhat"
    char            version[DATA_BUF_F64_SIZE];       // 操作系统版本，如"18.10"
    char            kernel[DATA_BUF_F64_SIZE];        // 操作系统内核，如"3.10-17"
    char            softVersion[DATA_BUF_F64_SIZE];   // 平台软件组件版本，如"V01.024"
    char            patchVersion[DATA_BUF_F64_SIZE];  // 平台软件组件补丁版本，如"PV01.0001"
}os_info_s;

// A.7   cpu 阈值信息字段: cfgCpu
typedef struct cfg_cpu_info {
    unsigned int     cpus;                        // CPU核数（例如值为2,3,4）
    unsigned int     cpuLmt;                      // CPU监控阈值
}cfg_cpu_info_s;

// A.8   内存阈值信息字段: cfgMem
typedef struct cfg_mem_info {
    unsigned int     memory;                      // 内存限值,单位： M Byte
    unsigned int     memLmt;                      // 内存监控阈值，百分数
}cfg_mem_info_s;

// A.9   硬盘阈值信息字段: cfgDisk
typedef struct cfg_disk_info {
    unsigned int     disk;                        // 存储限值，单位： M Byte
    unsigned int     diskLmt;                     // 磁盘存储监控阈值，百分数
}cfg_disk_info_s;

// A.11  数字签证信息字段:sign
typedef struct sign_info {
    char            name[DATA_BUF_F64_SIZE];          // 数字证书文件名字
    char            url[DATA_BUF_F64_SIZE];           // 数字证书文件路径，即 url 地址
    uint32_t        size;                      // 文件的大小，单位： KBytes
    char            md5[DATA_BUF_F64_SIZE];           // 数字证书文件的 md5 值，用于校验文件
}sign_info_s;

// A.10  文件信息字段:file
typedef struct file_info {
    char            name[DATA_BUF_F64_SIZE];          // 文件的名字
    char            fileType[DATA_BUF_F64_SIZE];      // 文件类型，如压缩文件（tar）等，由平台和终端统一规定
    char            url[DATA_BUF_F64_SIZE];           // 文件路径，即 url 地址，
    uint32_t        size;                      // 文件的大小，单位： KBytes
    char            md5[DATA_BUF_F64_SIZE];           // 文件的 md5 值，用于校验文件
    sign_info_s     sign;                      // 文件的数字签证信息
}file_info_s;


/* 设备参数 */
typedef struct sg_dev_param_info {
    uint8_t         startmode;                              // 启动模式 0正常 1专检
    char            mqtttopicversion[DATA_BUF_F32_SIZE];    // 协议版本 目前为v1
    char            devid[DATA_BUF_F32_SIZE];               // 设备ID   字符串
    char            ip[DATA_BUF_F32_SIZE];                  // IP地址   字符串
    uint32_t        port;                                   // 端口号
    char            clientid[DATA_BUF_F32_SIZE];
    char            user[DATA_BUF_F32_SIZE];
    char            password[DATA_BUF_F32_SIZE];
}sg_dev_param_info_s;


/* 断面文件 */
typedef struct sg_dev_section_info {
    uint8_t         rebootReason;               // 重启原因  0：正常重启  1：设备升级重启
    int32_t         jobid;
    int32_t         midid;
    char            version[DATA_BUF_F64_SIZE];
}sg_dev_section_info_s;


// 周期结构体
typedef struct sg_period_info {
    uint32_t        appperiod;                  // APP状态上报周期 单位秒
    uint32_t        containerperiod;            // 容器状态上报周期 单位秒
    uint32_t        devperiod;                  // 设备状态上报周期
    uint32_t        devheartbeatperiod;         // 设备心跳上报周期
}sg_period_info_s;

#ifdef __cplusplus
}
#endif

#endif
