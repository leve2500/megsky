

/*=====================================================================
 * �ļ���thread_data_manage.h
 *
 * ���������ݴ���  ����PRC������ ������  MQTT����
 *
 * ���ߣ�����			2020��9��27��17:09:59
 *
 * �޸ļ�¼��
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
