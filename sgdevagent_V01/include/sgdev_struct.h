/*=====================================================================
 * 文件：sgdev_agent.h
 *
 * 描述：公共定义的头文件定义
 *
 * 作者：田振超			2020年10月15日10:00:49
 *
 * 修改记录：
 =====================================================================*/

#ifndef _SGDEV_STRUCT_H_
#define _SGDEV_STRUCT_H_

#include "public_struct.h"

#ifdef __cplusplus
extern "C" {  // only need to export C interface if used by C++ source code
#endif

/* 1. (2) 设备接入请求 */
typedef struct dev_acc_req {
    dev_info_s       dev;                       //终端信息，详见附录 A
    cpu_info_s       cpu;                       //CPU 信息，详见附录 A  
    mem_info_s       mem;                       //内存信息，详见附录 A
    disk_info_s      disk;                      //磁盘信息，详见附录 A
    os_info_s        os;                        //操作系统信息，详见附录 A
    link_info_s      *links;         // 其他的终端信息，其中每个元素为一个 JSON对象，其定义详见附录 A   
    int              link_len;
}dev_acc_req_s;

/* 1. (3) 设备接入应答  无param*/

/* 1. (4)设备主动断开上报  单变量不需要定义结构体*/

/* 1. (5) 设备心跳请求  */
typedef struct heart_request {
    char            deviceId[DATA64_LEN];        //边设备唯一标识 
    char            timestamp[DATA32_LEN];       //消息发送的时间戳， CST 时间,精度到秒
    char            type[DATA32_LEN];            //EVENT_HEARTBEAT
}heart_request_s;

/* 1. (6) 设备心跳应答  */
typedef struct heart_reply {
    char            deviceId[DATA64_LEN];          //边设备唯一标识 
    char            timestamp[DATA32_LEN];         //消息发送的时间戳， CST 时间,精度到秒 
    char            type[DATA32_LEN];              //EVENT_HEARTBEAT             
    uint16_t        code;                          //标识应答的返回码:200
}heart_reply_s;

/* 2. (2) 设备升级命令  */
typedef struct device_upgrade {
    int32_t         jobId;                         //本升级操作作为一个工作任务，分配的 ID
    uint32_t        policy;                        //从接收到该升级指令时间后开始升级的时间间隔（单位：秒），缺省或等于 0 时，表示立即升级
    char            version[DATA64_LEN];           //升级后的版本号             
    uint8_t         upgradeType;                   //升级类型， 0 表示补丁升级， 1 表示文件系统升级， 2 表示内核升级， 3 表示全量升级（文件系统+内核升级）
    file_info_s     file;                          //升级文件信息
}device_upgrade_s;

/* 2. (3) 设备升级命令应答  无param*/

/* 2. (4)设备升级状态查询命令  单变量不需要定义结构体*/

/* 2. (5)设备升级状态查询应答 */
typedef struct dev_status_reply {
    int32_t          jobId;                         //升级操作作为一个工作任务，分配的 ID
    uint8_t          progress;                      //进度。百分比，省略百分号
    uint8_t          state;                         //描述当前升级过程执行过程。包括：
                                                    //1：待下载 2：下载中 3：待安装 4：安装中 5：安装完毕//
}dev_status_reply_s;

/* 2. (6)设备升级结果上报 ，容器，应用升级/安装都一样 */
typedef struct dev_upgrede_res_reply {
    int32_t             jobId;                  //升级操作作为一个工作任务，分配的 ID
    uint16_t            code;                   //容器升级结果编码
    char                msg[DATA64_LEN];        //容器升级结果描述，例如下载地址不可用
}dev_upgrede_res_reply_s;

//内存使用情况
typedef struct mem_used {
    int32_t         phy;                            //占用的物理内存（例如 50 表示 50%）
    int32_t         virt;                           //占用的虚拟内存（例如 50 表示 50%）
}mem_used_s;

//外接设备的相关信息
typedef struct link_dev_info {
    char            name[DATA64_LEN];               //接口的名称如为以太网口，则形如“eth1”
    char            status[DATA64_LEN];             //设备接口状态，如up/down
}link_dev_info_s;

/* 3. (1)设备状态上报 */
typedef struct dev_sta_reply {
    int             cpuRate;                       //CPU 负载（例如 50 表示 50%）
    mem_used_s      mem_used;                      //内存使用情况
    uint8_t         diskUsed;                      //磁盘占用率（例如 50 表示 50%）
    int             tempValue;                     //主板（cpu）温度，单位：摄氏度℃
    char            devDateTime[DATA64_LEN];       //设备当前时间
    char            devStDateTime[DATA64_LEN];     //设备最近一次启动时间    
    uint32_t        devRunTime;                    //设备运行时长，单位：秒
    link_dev_info_s *linkState;                    //其他的设备信息，其中每个元素表示一类设备元素
    char            longitude[DATA64_LEN];         //地理位置信息经度 
    char            latitude[DATA64_LEN];          //地理位置信息纬度 
    int             link_len;
}dev_sta_reply_s;

/* 3. (2) 设备状态查询命令  无param*/

/* 3. (3) 设备状态查询命令应答 和设备状态上报一样*/

/* 3. (4) 设备信息查询命令  无param*/

//描述温度监控信息
typedef struct temp_info {
    int         temLow;                      //主板温度监控低温阈值   
    int         temHigh;                     //主板温度监控高温阈值
}temp_info_s;

//状态主动上报的上报时间间隔
typedef struct rep_period {
    uint32_t        devPeriod;                  //终端状态主动上报的时间间隔，单位：秒
    uint32_t        conPeriod;                  //容器状态主动上报的时间间隔，单位：秒
    uint32_t        appPeriod;                  //APP 状态主动上报的时间间隔，单位：秒
    uint32_t        heartPeriod;                //终端应用层心跳上报的时间间隔,单位：秒
}rep_period_s;

/* 3. (5) 设备信息查询命令应答 */
typedef struct dev_info_inq_reply {
    dev_info_s       dev;                       //终端信息，详见附录A
    cpu_info_s       cpu;                       //CPU 信息，详见附录 A  
    mem_info_s       mem;                       //内存信息，详见附录 A
    disk_info_s      disk;                      //磁盘信息，详见附录 A
    temp_info_s      temperature;               //温度监控信息
    os_info_s        os;                        //操作系统信息，详见附录 A
    link_info_s      *links;                     //其他的终端信息，其中每个元素为一个 JSON对象，其定义详见附录 A    
    rep_period_s     rep_period;                //状态主动上报的上报时间间隔
    int              link_len;
}dev_info_inq_reply_s;

/* 3. (6) 设备管理参数修改命令 */
typedef struct dev_man_conf_command {
    char             devName[DATA64_LEN];       //设备名称    
    uint8_t          cpuLmt;                    //CPU 监控阈值,例如 50 表示 50%
    uint8_t          memLmt;                    //内存监控阈值,例如 50 表示 50%   
    uint8_t          diskLmt;                   //磁盘监控阈值,例如 50 表示 50%
    temp_info_s      temperature;               //温度监控信息
    rep_period_s     rep_period;                //状态主动上报的上报时间间隔
}dev_man_conf_command_s;

/* 3. (7) 设备管理参数修改命令应答  无param*/

/* 3. (8) 设备时间同步命令 */
typedef struct dev_time_command {
    char             dateTime[DATA64_LEN];       //日期和时间参数 
    char             timeZone[DATA64_LEN];       //时区
}dev_time_command_s;

/* 3. (9) 设备时间同步命令应答 无param*/

/* 3. (10) 设备事件上报 */
typedef struct dev_event_reply {
    char                event[DATA64_LEN];        //下表中的一些典型事件用 1001,1002,1003 表示
    char                msg[DATA64_LEN];          //msg 给出事件具体情况的描述，最长不超过256 字符
}dev_thing_reply_s;

/* 3. (11) 设备日志召回 */
typedef struct dev_log_recall {
    char                url[DATA64_LEN];        //文件上传路径
    uint8_t             logType;                //日志类型， 0-全部类型日志； 1-系统日志； 2-操作日志； 3-安全日志； 4-驱动日志； 5-broker日志、 6-审计日志； 7-调试日志； 8-255 用于扩展。
}dev_log_recall_s;

/* 3. (12)设备日志召回应答  单变量不需要定义结构体*/

/* 4. (1)设备控制命令  单变量不需要定义结构体*/

/* 4. (2) 设备控制命令应答 无param*/

//描述随容器一起安装到容器内的应用软件信息
typedef struct with_app_info {
    char                version[DATA64_LEN];    //版本号
    file_info_s         file;                   //应用文件信息
    cfg_cpu_info_s      cfgCpu;                 //cpu 资源配置参数
    cfg_mem_info_s      cfgMem;                 //memory 资源配置参数
    char                enable[DATA64_LEN];     //使能/去使能状态定义，使能为“1”，去使能为“0”
}with_app_info_s;

/* 1. (2) 容器安装控制命令 */
typedef struct container_install_cmd {
    int32_t             jobId;                  //作为一个工作任务，分配的 ID
    uint32_t            policy;                 //从接收到该安装指令时间后开始安装的时间间隔（单位：秒），缺省或等于 0 时，表示立即升级
    char                container[DATA64_LEN];  //容器名称
    file_info_s         image;                  //容器镜像
    with_app_info_s     withAPP;                //随容器安装时，被添加的 APP 名称  
    cfg_cpu_info_s      cfgCpu;                 //cpu 资源配置参数
    cfg_mem_info_s      cfgMem;                 //memory 资源配置参数
    cfg_disk_info_s     cfgDisk;                //disk 资源配置参数
    char                port[DATA64_LEN];       //容器端口资源配置参数
    char                mount[DATA64_LEN][DATA64_LEN]; //映射的本地文件目录资源配置参数
    char                dev[DATA64_LEN][DATA64_LEN];   //映射的本地物理接口资源配置参数
    uint16_t            mount_len;
    uint16_t            dev_len;

}container_install_cmd_s;

/* 1. (3)容器安装控制应答  单变量不需要定义结构体*/

/* 1. (4)容器安装状态查询命令  单变量不需要定义结构体*/

/* 1. (5)容器安装状态查询应答 */
typedef struct container_install_inq_reply {
    int32_t             jobId;                  //升级操作作为一个工作任务，分配的 ID
    uint8_t             progress;               //进度。百分比，省略百分号
    uint8_t             state;                  /*描述当前升级过程执行过程。包括：
                                                1：待下载
                                                2：下载中
                                                3：待安装
                                                4：安装中
                                                5：安装完毕*/
}container_install_inq_reply_s;

/* 1. (6)容器安装结果上报 见设备升级结果上报*/

/* 2. (2)容器启动控制命令  单变量不需要定义结构体*/

/* 2. (3) 容器启动控制命令应答 无param*/

/* 3. (2)容器停止控制命令  单变量不需要定义结构体*/

/* 3. (3)容器停止控制应答 无param*/

/* 4. (2)容器停止控制应答 单变量不需要定义结构体*/

/* 4. (3)容器删除控制应答 无param*/

/* 5. (2)容器配置修改命令 */
typedef struct container_conf_cmd {
    char                container[DATA64_LEN];  //容器名称  
    cfg_cpu_info_s      cfgCpu;                 //cpu 资源配置参数,详见附录 A
    cfg_mem_info_s      cfgMem;                 //memory 资源配置参数,详见附录 A
    cfg_disk_info_s     cfgDisk;                //disk 资源配置参数,详见附录 A
    char                port[DATA64_LEN];       //端口资源配置参数
    char                mount[DATA64_LEN][DATA64_LEN]; //映射的本地文件目录资源配置参数
    char                dev[DATA64_LEN][DATA64_LEN];   //映射的本地物理接口资源配置参数
    uint16_t            mount_len;
    uint16_t            dev_len;
}container_conf_cmd_s;

/* 5. (3)容器配置修改应答 无param*/

/* 6. (2)容器配置查询命令 无param*/

/* 6. (3)容器配置查询应答 */
typedef struct container_config_reply {
    container_conf_cmd_s    *contPara;         // 其他的终端信息，其中每个元素为一个 JSON对象，其定义详见附录 A
    uint16_t                contPara_len;
}container_config_reply_s;


/* 7. (2)容器状态查询命令 无param*/

/* 7. (3)容器状态查询应答 */
typedef struct container_status_reply {
    char                container[DATA64_LEN];  //容器名称
    char                version[DATA64_LEN];    //容器版本号
    char                state[DATA64_LEN];      //容器运行状态， running 或 stopped
    int                 cpuRate;                //CPU 占用率，百分比
    int                 memUsed;                //内存占用率，百分比       
    int                 diskUsed;               //磁盘占用率，百分比 
    char                ip[DATA64_LEN];         //IP 地址及端口
    char                created[DATA64_LEN];    //创建时间           ***暂时没有
    char                started[DATA64_LEN];    //最近一次启动时间    
    time_t              lifeTime;               //运行时间，单位：秒
    char                image[DATA64_LEN];      //容器镜像信息       ***暂时没有
    int                 container_len;
}container_status_reply_s;

/* 8 容器状态上报  和7.（3）相同*/

/* 9  容器事件上报 */
typedef struct container_event_report {
    char                container[DATA64_LEN];  //容器名称     
    char                event[DATA64_LEN];      //下表中的一些典型事件用 4001,4002,4003 表示
    char                msg[DATA64_LEN];        //msg 给出事件具体情况的描述，最长不超过 256字符
}container_event_report_s;

/* 10. (2)容器升级命令 */
typedef struct container_upgrade_cmd {
    int32_t             jobId;                  //本升级操作作为一个工作任务，分配的 ID
    uint32_t            policy;                 //从接收到该升级指令时间后开始升级的时间间隔（单位：秒），缺省或等于 0 时，表示立即升级
    char                version[DATA64_LEN];    //升级后的版本号
    file_info_s         file;                   //文件信息
}container_upgrade_cmd_s;

/* 10. (3)容器升级命令应答 无param*/

/* 10. (4)容器升级状态查询命令  单变量不需要定义结构体*/

/* 10. (5)容器升级状态查询应答 */
typedef struct container_upgrede_inq_reply {
    int32_t             jobId;                  //升级操作作为一个工作任务，分配的 ID
    uint8_t             progress;               //进度。百分比，省略百分号
    uint8_t             state;                  /*描述当前升级过程执行过程。包括：
                                                1：待下载
                                                2：下载中
                                                3：待安装
                                                4：安装中
                                                5：安装完毕*/
}container_upgrede_inq_reply_s;

/* 10. (6)容器升级结果上报 在设备中可找到 */


/* 11. (1) 容器日志召回命令 */
typedef struct container_log_recall_cmd {
    char                container[DATA64_LEN];  //容器名称，如果容器名称不出现，则召回所有容器的日志
    char                url[DATA64_LEN];        //文件上传路径
}container_log_recall_cmd_s;

/* 11. (2)容器日志召回应答  单变量不需要定义结构体*/

/* 1. (2) 应用安装控制命令 */
typedef struct app_install_cmd {
    int32_t             jobId;                  //本升级操作作为一个工作任务，分配的 ID
    uint32_t            policy;                 //从接收到该安装指令时间后开始安装的时间间隔（单位：秒），缺省或等于 0 时，表示立即升级
    char                container[DATA64_LEN];  //容器名字
    char                version[DATA64_LEN];    //应用版本号
    file_info_s         file;                   //升级的 APP，详见附录 A
    cfg_cpu_info_s      cfgCpu;                 //cpu 资源配置参数,详见附录 A
    cfg_mem_info_s      cfgMem;                 //memory 资源配置参数,详见附录 A
    char                enable[DATA64_LEN];     //使能/去使能状态定义，使能为“1”，去使能为“0”
    char                app[DATA64_LEN];        //应用名称
}app_install_cmd_s;

/* 1. (3)应用安装控制命令应答 无param*/

/* 1. (4)应用安装状态查询命令  单变量不需要定义结构体*/

/* 1. (5) 应用安装状态查询应答 */
typedef struct app_install_inq_reply {
    int32_t           jobId;                  //升级操作作为一个工作任务，分配的 ID
    uint8_t           progress;               //进度。百分比，省略百分号
    uint8_t           state;                  /*描述当前升级过程执行过程。包括：
                                                1：待下载
                                                2：下载中
                                                3：待安装
                                                4：安装中
                                                5：安装完毕*/
}app_install_inq_reply_s;

/* 1. (6)应用安装结果上报  见设备升级结果上报 */


/* 2. (2)应用控制命令 */
typedef struct app_control_cmd {
    char                container[DATA64_LEN];  //容器名称
    char                app[DATA64_LEN];        //应用名称
}app_control_cmd_s;

/* 2. (3)应用控制命令应答 无param*/

/* 3. (2)应用配置修改命令 */
typedef struct app_conf_cmd {
    char                container[DATA64_LEN];  //容器名字
    char                app[DATA64_LEN];        //应用文件名字
    cfg_cpu_info_s      cfgCpu;                 //cpu 资源配置参数,详见附录 A
    cfg_mem_info_s      cfgMem;                 //memory 资源配置参数,详见附录 A
}app_conf_cmd_s;

/* 3. (3)应用配置修改命令应答 无param*/

/* 4. (2)应用配置查询命令  单变量不需要定义结构体*/

//表71 Array<appCfgs>字段说明
typedef struct app_cfgs_info {
    char                app[DATA64_LEN];        //应用文件名字
    cfg_cpu_info_s      cfgCpu;                 //cpu 资源配置参数,详见附录 A
    cfg_mem_info_s      cfgMem;                 //memory 资源配置参数,详见附录 A  
}app_cfgs_info_s;

/* 4. (3)应用配置查询应答 */
typedef struct app_conf_reply {
    char                container[DATA64_LEN];  //容器名字
    app_cfgs_info_s     appCfgs[DATA16_LEN];    //应用配置参数   翟工说定义的太大，这块最多不超过10个
    uint16_t            app_num;
}app_conf_reply_s;

/* 5. (2)应用状态查询命令  单变量不需要定义结构体*/

//表75 Array<process>字段说明
typedef struct process_info {
    uint32_t            srvIndex;               //进程索引
    char                srvName[DATA64_LEN];    //进程名称
    char                srvEnable[DATA64_LEN];  //服务使能状态， yes 或 no
    char                srvStatus[DATA64_LEN];  //服务状态， running 或 stopped
    uint8_t             cpuLmt;                 //CPU 检测阈值，百分比数据
    uint8_t             cpuRate;                //当前 CPU 使用率，百分比数据
    uint8_t             memLmt;                 //内存检测阈值，百分比数据
    uint8_t             memUsed;                //当前内存使用空间的大小，百分比数据
    char                startTime[DATA64_LEN];  //表示服务启动时间，百分比数据
}process_info_s;

//表74 Array<apps>报文字段说明
typedef struct apps_info {
    char                app[DATA64_LEN];        //APP 名称
    char                version[DATA32_LEN];    //APP 版本
    char                appHash[DATA64_LEN];    //APP 的哈希值
    uint32_t            srvNumber;              //当前 APP 的进程数量
    process_info_s      *process;               //进程字段
}apps_info_s;

/* 5. (3)应用状态查询应答 */
typedef struct app_inq_reply {
    char                container[DATA64_LEN];        //容器名称
    apps_info_s         *apps;                        //app 状态参数 
    uint16_t            apps_num;
}app_inq_reply_s;

/* 6 应用状态上报 */
typedef struct app_status_reply {
    char                container[DATA64_LEN];        //容器名称
    apps_info_s         apps[DATA16_LEN];             //app 状态参数 
    uint16_t            apps_num;
}app_status_reply_s;

/* 7 应用事件上报 */
typedef struct app_event_reply {
    char                container[DATA64_LEN];        //容器名称
    char                app[DATA64_LEN];              //app 名称，如果 APP 名称不出现，则上报的是所有 APP 的统一事件。
    char                event[DATA64_LEN];            //见表 80
    char                msg[DATA64_LEN];              //msg 给出事件具体情况的描述，最长不超过256 字符
}app_event_reply_s;

/* 8. (2)应用升级命令 */
typedef struct app_upgrade_cmd {
    int32_t         jobId;                         //本升级操作作为一个工作任务，分配的 ID
    uint32_t        policy;                        //从接收到该升级指令时间后开始升级的时间间隔（单位：秒），缺省或等于 0 时，表示立即升级
    char            version[DATA64_LEN];           //升级后的版本号             
    char            container[DATA64_LEN];         //容器名称
    file_info_s     file;                          //升级文件
}app_upgrade_cmd_s;

/* 8. (3)应用升级命令应答 无param*/

/* 8. (4)应用升级状态查询命令  单变量不需要定义结构体*/

/* 8. (5)应用升级状态查询应答 */
typedef struct app_upgrede_inq_reply {
    int32_t           jobId;                  //升级操作作为一个工作任务，分配的 ID
    uint8_t           progress;               //进度。百分比，省略百分号
    uint8_t           state;                  /*描述当前升级过程执行过程。包括：
                                                1：待下载
                                                2：下载中
                                                3：待安装
                                                4：安装中
                                                5：安装完毕*/
}app_upgrede_inq_reply_s;

/* 8. (6)应用升级结果上报 见设备升级结果上报*/


/* 9. (1) 应用日志召回命令 */
typedef struct app_log_recall_cmd {
    char              container[DATA64_LEN];  //容器名称
    char              url[DATA64_LEN];        //文件上传路径
    char              app[DATA64_LEN];        //app 名称，如果 APP 名称不出现，则召回的是所有 APP 的日志。
}app_log_recall_cmd_s;

#ifdef __cplusplus
}
#endif 

#endif

