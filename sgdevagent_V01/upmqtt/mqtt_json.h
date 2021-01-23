
#ifndef _MQTT_JSON_H_
#define _MQTT_JSON_H_

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

json_t *load_json(const char *text);

int msg_parse(json_t *root);

int json_into_int8_t(int8_t *idata, json_t *jdata, const char *key);

int json_into_uint8_t(uint8_t *idata, json_t *jdata, const char *key);

int json_into_int32_t(int32_t *idata, json_t *jdata, const char *key);

int json_into_uint32_t(uint32_t *idata, json_t *jdata, const char *key);

int json_into_string(char *sdata, json_t *jdata, const char *key);

int json_into_array_string(char *sdata, json_t *jdata);

//生成时间戳
int sg_mqtttimestr(time_t time_val, char *time_buff, size_t buff_len, int use_utc);

//消息头请求接口
mqtt_request_header_s *sg_unpack_json_msg_header_request(json_t* root);

//消息头应答接口
mqtt_reply_header_s *sg_unpack_json_msg_header_reply(json_t* root);

//消息头组帧接口 param为空时直接调用该函数
char *sg_pack_json_msg_header(uint16_t code, int32_t mid, const char *type, const char *msg, json_t *param);

int sg_getfile(json_t *obj, file_info_s *fileobj);

int sg_getcfgcpu(json_t *obj, cfg_cpu_info_s *cfgcpuobj);

int sg_getcfgmem(json_t *obj, cfg_mem_info_s *cfgmemobj);

int sg_getcfgdisk(json_t *obj, cfg_disk_info_s *cfgdiskobj);

#ifdef __cplusplus
}
#endif 

#endif


