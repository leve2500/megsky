#ifndef	_FRAME_RPC_H_
#define	_FRAME_RPC_H_

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RECVBUFLEN (1024 * 16)


int16_t sg_packframe(char * msg_send, mqtt_data_info_s* info);

uint8_t* GetRecvBuf(uint16_t us_pos);
uint16_t sg_get_recvive_num(void);
void sg_set_recv_cnt(uint16_t usRecvCnt);
void sg_unpack_frame(void);
void sg_recvive(uint8_t* pBuf, uint32_t ulSize, uint16_t us_pos);

#ifdef __cplusplus
}
#endif

#endif
