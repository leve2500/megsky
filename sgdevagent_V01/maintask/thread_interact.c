#include "vrp_task.h"
#include "vrp_queue.h"

#include "vos_typdef.h"

#include "thread_interact.h"
#include "thread_dev_insert.h"
#include "mqtt_pub.h"
#include "mqtt_dev.h"
#include "mqtt_app.h"
#include "mqtt_container.h"
#include "sgdev_queue.h"
#include "sgdev_struct.h"
#include "timer_pack.h"



dev_status_reply_s device_upgrade_status = { 0 };
dev_status_reply_s container_install_status = { 0 };
dev_status_reply_s container_upgrade_status = { 0 };
dev_status_reply_s app_install_status = { 0 };
dev_status_reply_s app_upgrade_status = { 0 };

uint8_t m_exe_state = TASK_EXE_STATUS_NULL;

static void sg_pack_dev_upgrade_status(int16_t code, int32_t mid)
{

    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;
    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    char errormsg[DATA256_LEN];
    sprintf_s(errormsg, DATA256_LEN, "%s", "command success");

    sprintf_s(item->pubtopic, DATA256_LEN, "%s", get_topic_sub_dev_res());
    ret = sg_pack_dev_install_query(code, mid, errormsg, device_upgrade_status, item->msg_send);
    if (VOS_OK != ret) {
        printf("\n sg_pack_dev_install_query fail.");
    }
    sg_push_pack_item(item);        //入列
}

static void sg_pack_container_upgrade_status(int32_t mid, int32_t jobId)
{
    int16_t code = REQUEST_SUCCESS;
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;

    dev_status_reply_s status;
    if (TASK_EXE_CONTAINER_INSTALL == m_exe_state) {
        status = container_install_status;
    } else if (TASK_EXE_CONTAINER_UP == m_exe_state) {
        status = container_upgrade_status;
    }
    if (jobId != status.jobId) {
        code = REQUEST_FAILED;
    }

    char errormsg[DATA256_LEN];
    sprintf_s(errormsg, DATA256_LEN, "%s", "command success");

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));

    sprintf_s(item->pubtopic, DATA256_LEN, "%s", get_topic_container_reply_pub());
    ret = sg_pack_dev_install_query(code, mid, errormsg, status, item->msg_send);
    if (VOS_OK != ret) {
        printf("\n sg_pack_dev_install_query fail.");
    }
    sg_push_pack_item(item);        //入列
}


static void sg_pack_app_upgrade_status(int32_t mid, int32_t jobId)
{
    int16_t code = REQUEST_SUCCESS;
    int ret = VOS_OK;
    mqtt_data_info_s *item = NULL;

    dev_status_reply_s status;
    if (TASK_EXE_APP_INSTALL == m_exe_state) {
        status = app_install_status;
    } else if (TASK_EXE_APP_UP == m_exe_state) {
        status = app_upgrade_status;
    }
    if (jobId != status.jobId) {
        code = REQUEST_FAILED;
    }

    char errormsg[DATA256_LEN];
    sprintf_s(errormsg, DATA256_LEN, "%s", "command success");

    item = (mqtt_data_info_s*)VOS_Malloc(MID_SGDEV, sizeof(mqtt_data_info_s));
    (void)memset_s(item, sizeof(mqtt_data_info_s), 0, sizeof(mqtt_data_info_s));
    sprintf_s(item->pubtopic, DATA256_LEN, "%s", get_topic_app_reply_pub());
    ret = sg_pack_dev_install_query(code, mid, errormsg, status, item->msg_send);
    if (VOS_OK != ret) {
        printf("\n sg_pack_dev_install_query fail.");
    }
    sg_push_pack_item(item);        //入列
}

static void sg_dev_com_data_unpack(char* param, char* type, int32_t mid)
{
    uint16_t cmdType = TAG_CMD_NULL;
    int32_t                 jobId;
    int16_t code = REQUEST_SUCCESS;
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == jparam) {
        printf("sg_dev_com_data_unpack fail \n");
        return;
    }
    if (0 == strncmp(type, CMD_SYS_UPGRADE, strlen(CMD_SYS_UPGRADE))) {          //设备升级命令 
        cmdType = TAG_CMD_SYS_UPGRADE;
        m_exe_state = TASK_EXE_DEV_UP;
        sg_push_dev_item(cmdType, mid, param);
        printf("CMD_SYS_UPGRADE \n");   //测试用
    } else if (0 == strncmp(type, CMD_STATUS_QUERY, strlen(CMD_STATUS_QUERY))) {	//设备升级状态查询
        jobId = sg_unpack_dev_install_query(jparam);
        if (jobId != device_upgrade_status.jobId) {
            code = REQUEST_FAILED;
        }
        sg_pack_dev_upgrade_status(code, mid);
        printf("CMD_STATUS_QUERY \n");   //测试用
        if (param != NULL) {
            free(param);
        }
    } else if (0 == strncmp(type, CMD_SYS_STATUS, strlen(CMD_SYS_STATUS))) {        //设备状态查询命令
        cmdType = TAG_CMD_SYS_STATUS;
        sg_push_dev_item(cmdType, mid, param);
        printf("CMD_SYS_STATUS \n");   //测试用
    } else if (0 == strncmp(type, CMD_INFO_QUERY, strlen(CMD_INFO_QUERY))) {        //设备信息查询命令
        cmdType = TAG_CMD_INFO_QUERY;
        sg_push_dev_item(cmdType, mid, param);
        printf("CMD_INFO_QUERY \n");   //测试用
    } else if (0 == strncmp(type, CMD_SYS_SET_CONFIG, strlen(CMD_SYS_SET_CONFIG))) {//设备管理参数修改命令
        cmdType = TAG_CMD_SYS_SET_CONFIG;
        sg_push_dev_item(cmdType, mid, param);
        printf("CMD_SYS_SET_CONFIG \n"); //测试用

    } else if (0 == strncmp(type, CMD_DATETIME_SYN, strlen(CMD_DATETIME_SYN))) {    //设备时间同步命令
        cmdType = TAG_CMD_DATETIME_SYN;
        sg_push_dev_item(cmdType, mid, param);
        printf("CMD_DATETIME_SYN \n");  //测试用
    } else if (0 == strncmp(type, CMD_SYS_LOG, strlen(CMD_SYS_LOG))) {              //设备日志召回
        cmdType = TAG_CMD_SYS_LOG;
        sg_push_dev_item(cmdType, mid, param);
        printf("CMD_SYS_LOG \n");       //测试用 
    } else if (0 == strncmp(type, CMD_CTRL, strlen(CMD_CTRL))) {                    //设备控制命令
        cmdType = TAG_CMD_CTRL;
        sg_push_dev_item(cmdType, mid, param);
        printf("CMD_CTRL \n");          //测试用 
    }
    if (NULL != jparam) {
        json_decref(jparam);//会将所有的子节点都free
    }
}

static void sg_dev_res_data_unpack(char *param, char *type)
{
    uint8_t heartFlag = 0;
    if (0 == strncmp(type, EVENT_LINKUP, strlen(EVENT_LINKUP))) {                //上线 
        (void)sg_set_dev_ins_flag(DEVICE_ONLINE);
    } else if (0 == strncmp(type, EVENT_HEARTBEAT, strlen(EVENT_HEARTBEAT))) {     //心跳应答
        heartFlag = 1;
        (void)sg_set_dev_heart_flag(heartFlag);
    }
    // if(param != NULL){
    //     VOS_Free(param);
    //     param = NULL;
    // }
}

static void sg_ctai_com_data_unpack(char *param, char *type, int32_t mid)
{
    uint16_t cmdType = TAG_CMD_NULL;
    int32_t          jobId;
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == jparam) {
        printf("\n sg_dev_com_data_unpack fail");
        return;
    }
    if (0 == strncmp(type, CMD_STATUS_QUERY, strlen(CMD_STATUS_QUERY))) {                       //容器升级状态查询命令,容器安装状态查询命令
        jobId = sg_unpack_dev_install_query(jparam);
        sg_pack_container_upgrade_status(mid, jobId);
        printf("CMD_STATUS_QUERY!\n");

        if (param != NULL) {
            VOS_Free(param);
        }
    } else {
        if (0 == strncmp(type, CMD_CON_GET_CONFIG, strlen(CMD_CON_GET_CONFIG))) {               //容器配置查询命令 设备队列中
            cmdType = TAG_CMD_CON_GET_CONFIG;
            sg_push_dev_item(cmdType, mid, param);
            printf("CMD_CON_GET_CONFIG!\n");

        } else if (0 == strncmp(type, CMD_CON_STATUS, strlen(CMD_CON_STATUS))) {                //容器状态查询命令 设备队列中
            cmdType = TAG_CMD_CON_STATUS;
            sg_push_dev_item(cmdType, mid, param);
            printf("CMD_CON_STATUS!\n");

        } else {
            if (0 == strncmp(type, CMD_CON_INSTALL, strlen(CMD_CON_INSTALL))) {                 //容器安装控制命令 
                cmdType = TAG_CMD_CON_INSTALL;
                m_exe_state = TASK_EXE_CONTAINER_INSTALL;
                printf("container_install_success!\n");

            } else if (0 == strncmp(type, CMD_CON_START, strlen(CMD_CON_START))) {              //容器启动控制命令
                cmdType = TAG_CMD_CON_START;
                printf("container_start_success!\n");

            } else if (0 == strncmp(type, CMD_CON_STOP, strlen(CMD_CON_STOP))) {                //容器停止控制命令
                cmdType = TAG_CMD_CON_STOP;
                printf("container_stop_success!\n");

            } else if (0 == strncmp(type, CMD_CON_REMOVE, strlen(CMD_CON_REMOVE))) {            //容器删除控制
                cmdType = TAG_CMD_CON_REMOVE;
                printf("container_remove_success!\n");

            } else if (0 == strncmp(type, CMD_CON_SET_CONFIG, strlen(CMD_CON_SET_CONFIG))) {    //容器配置修改命令
                cmdType = TAG_CMD_CON_SET_CONFIG;
                printf("container_set_success!\n");

            } else if (0 == strncmp(type, CMD_CON_UPGRADE, strlen(CMD_CON_UPGRADE))) {          //容器升级命令
                m_exe_state = TASK_EXE_CONTAINER_UP;
                cmdType = TAG_CMD_CON_UPGRADE;
                printf("container_upgrade_success!\n");

            } else if (0 == strncmp(type, CMD_CON_LOG, strlen(CMD_CON_LOG))) {                  //容器日志召回命令
                cmdType = TAG_CMD_CON_LOG;
                printf("container_log_success!\n");
            }
            sg_push_container_item(cmdType, mid, param);
        }
    }
    if (NULL != jparam) {
        json_decref(jparam);//会将所有的子节点都free
    }
}

static void sg_app_com_data_unpack(char* param, char *type, int32_t mid)
{
    uint16_t cmdType = TAG_CMD_NULL;
    int32_t             jobId;
    json_t *jparam = NULL;
    jparam = load_json(param);
    if (NULL == jparam) {
        printf("\n sg_dev_com_data_unpack fail");
        return;
    }
    if (0 == strncmp(type, CMD_STATUS_QUERY, strlen(CMD_STATUS_QUERY))) {			            //app安装状态查询命令/app升级状态查询命令
        jobId = sg_unpack_dev_install_query(jparam);
        sg_pack_app_upgrade_status(mid, jobId);
        printf("CMD_STATUS_QUERY \n");
        if (param != NULL)
            free(param);
    } else {
        if (0 == strncmp(type, CMD_APP_GET_CONFIG, strlen(CMD_APP_GET_CONFIG))) {             //应用配置查询  设备队列中
            cmdType = TAG_CMD_APP_GET_CONFIG;
            sg_push_dev_item(cmdType, mid, param);
            printf("CMD_APP_GET_CONFIG \n");
        } else if (0 == strncmp(type, CMD_APP_STATUS, strlen(CMD_APP_STATUS))) {                //应用状态查询命令  设备队列中
            cmdType = TAG_CMD_APP_STATUS;
            sg_push_dev_item(cmdType, mid, param);
            printf("CMD_APP_STATUS \n");
        } else {
            if (0 == strncmp(type, CMD_APP_INSTALL, strlen(CMD_APP_INSTALL))) {               //应用安装控制命令
                m_exe_state = TASK_EXE_APP_INSTALL;
                cmdType = TAG_CMD_CON_LOG;
                printf("CMD_APP_INSTALL \n");
            } else if (0 == strncmp(type, CMD_APP_START, strlen(CMD_APP_START))) {              //应用启动
                cmdType = TAG_CMD_APP_START;
                printf("CMD_APP_START \n");
            } else if (0 == strncmp(type, CMD_APP_STOP, strlen(CMD_APP_STOP))) {                //应用停止
                cmdType = TAG_CMD_APP_STOP;
                printf("CMD_APP_STOP \n");
            } else if (0 == strncmp(type, CMD_APP_REMOVE, strlen(CMD_APP_REMOVE))) {            //应用卸载
                cmdType = TAG_CMD_APP_REMOVE;
                printf("CMD_APP_REMOVE \n");
            } else if (0 == strncmp(type, CMD_APP_ENABLE, strlen(CMD_APP_ENABLE))) {            //应用使能
                cmdType = TAG_CMD_APP_ENABLE;
                printf("CMD_APP_ENABLE \n");
            } else if (0 == strncmp(type, CMD_APP_UNENABLE, strlen(CMD_APP_UNENABLE))) {		//去使能
                cmdType = TAG_CMD_APP_UNENABLE;
                printf("CMD_APP_UNENABLE \n");
            } else if (0 == strncmp(type, CMD_APP_SET_CONFIG, strlen(CMD_APP_SET_CONFIG))) {	//应用配置修改命令
                cmdType = TAG_CMD_APP_SET_CONFIG;
                printf("CMD_APP_SET_CONFIG \n");
            } else if (0 == strncmp(type, CMD_APP_UPGRADE, strlen(CMD_APP_UPGRADE))) {			//容器升级命令
                m_exe_state = TASK_EXE_APP_UP;
                cmdType = TAG_CMD_APP_UPGRADE;
                printf("CMD_APP_UPGRADE \n");
            } else if (0 == strncmp(type, CMD_APP_LOG, strlen(CMD_APP_LOG))) {			        //应用日志召回命令
                cmdType = TAG_CMD_APP_LOG;
                printf("CMD_APP_LOG \n");
            }
            sg_push_app_item(cmdType, mid, param);
        }
    }
    if (NULL != jparam) {
        json_decref(jparam);//会将所有的子节点都free
    }
}

static void sg_topic_data_unpack(mqtt_data_info_s *item)
{
    if (NULL == item) {
        printf("\n sg_topic_data_unpack fail,item = NULL.");
        // return;
    }
    mqtt_request_header_s *headerReq = NULL;  //请求报文头
    mqtt_reply_header_s *headerRly = NULL;    //应答报文头
    json_t *param = NULL;
    uint8_t type = 0;
    char    msgType[32];
    char *srt_pram = NULL;
    int32_t midID;
    json_t *root = NULL;
    if (strlen(item->msg_send) != 0) {
        printf("***msg_send = %s\n", item->msg_send);  //打印一下接收到的信息  
    }
    root = load_json(item->msg_send);
    if (NULL != root) {
        type = msg_parse(root);     //判断消息类型：根据是否有code判断是请求帧还是应答帧
    }
    if (type == TYPE_REQUEST) {     //请求
        printf("******************request********************\n");
        headerReq = sg_unpack_json_msg_header_request(root);
        param = headerReq->param;
        memcpy_s(msgType, 32, headerReq->type, strlen(headerReq->type));
        midID = headerReq->mid;
        if (headerReq != NULL) {
            VOS_Free(headerReq);
            headerReq = NULL;
        }
    } else if (type == TYPE_REPLY) {  //应答 
        printf("******************reply********************\n");
        headerRly = sg_unpack_json_msg_header_reply(root);
        param = headerRly->param;
        memcpy_s(msgType, 32, headerRly->type, strlen(headerRly->type) + 1);
        midID = headerRly->mid;
        if (headerRly != NULL) {
            VOS_Free(headerRly);
            headerRly = NULL;
        }
    }
    srt_pram = json_dumps(param, JSON_PRESERVE_ORDER);                          //会分配内存,内存申请函数为malloc,释放的时候需要用free

    if (0 == strncmp(item->pubtopic, get_topic_sub_dev_com(), strlen(get_topic_sub_dev_com()))) {         //设备控制命令
        (void)sg_dev_com_data_unpack(srt_pram, msgType, midID);

    } else if (0 == strncmp(item->pubtopic, get_topic_sub_dev_res(), strlen(get_topic_sub_dev_res()))) {    //设备请求命令
        (void)sg_dev_res_data_unpack(srt_pram, msgType);

    } else if (0 == strncmp(item->pubtopic, get_topic_sub_ctai_com(), strlen(get_topic_sub_ctai_com()))) {  //容器控制请求命令
        (void)sg_ctai_com_data_unpack(srt_pram, msgType, midID);

    } else if (0 == strncmp(item->pubtopic, get_topic_sub_app_com(), strlen(get_topic_sub_app_com()))) {     //app控制请求命令
        (void)sg_app_com_data_unpack(srt_pram, msgType, midID);

    }
    if (NULL != root) {
        json_decref(root);//会将所有的子节点都free
    }
    // if (srt_pram != NULL) {
    //     free(srt_pram);
    //     srt_pram = NULL;
    // } 
}

void set_device_upgrade_status(dev_status_reply_s sta)
{
    device_upgrade_status = sta;
}

void set_container_install_status(dev_status_reply_s sta)
{
    container_install_status = sta;
}

void set_container_upgrade_status(dev_status_reply_s sta)
{
    container_upgrade_status = sta;
}

void set_app_install_status(dev_status_reply_s sta)
{
    app_install_status = sta;
}

void set_app_upgrade_status(dev_status_reply_s sta)
{
    app_upgrade_status = sta;
}

int bus_inter_thread(void) //业务交互线程
{
    printf("bus_inter_thread start . \n");
    int bRet = 0;
    VOS_UINT32 freeRet = 0;
    uint32_t unpack_queue_id = sg_get_que_unpack_id();      //获取队列id
    VOS_UINTPTR msg[VOS_QUEUE_MSG_NUM] = { 0 };

    mqtt_data_info_s *item = NULL;
    // uint32_t msgType = 0;
    for (; ;) {
        if (sg_get_mqtt_connect_flag() == DEVICE_OFFLINE) {
            VOS_T_Delay(3 * 1000);
            continue;
        }
        if (VOS_OK == VOS_Que_Read(unpack_queue_id, msg, VOS_WAIT, 0)) {
            switch (msg[0]) {
            case QUEUE_UNPACK:
                item = (mqtt_data_info_s *)(msg[1]);
                printf("megsky --test：unpack_queue  pubtopic. ==%s \n", item->pubtopic);
                (void)sg_topic_data_unpack(item);

                freeRet = VOS_Free(item);
                if (freeRet != VOS_OK) {
                    printf("bus_inter_thread VOS_Free fail \n");
                }
                break;
            default:
                break;
            }
        }
        VOS_T_Delay(10);
    }
    return bRet;
}



