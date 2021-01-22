

/*=====================================================================
 * 文件：thread_data_manage.h
 *
 * 描述：数据处理  包括PRC收数据 发数据  MQTT发布
 *
 * 作者：田振超			2020年9月27日17:09:59
 *
 * 修改记录：
 =====================================================================*/


#ifndef _THREAD_DATA_MGR_H_
#define _THREAD_DATA_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif


int sg_rpc_read_data_thread(void);

int sg_rpc_write_data_thread(void);

int sg_mqtt_pub_thread(void);



#ifdef __cplusplus
}
#endif 

#endif
