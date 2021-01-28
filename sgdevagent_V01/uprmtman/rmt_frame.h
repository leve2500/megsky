#ifndef	_FRAME_RPC_H_
#define	_FRAME_RPC_H_

#include "sgdev_struct.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RECVBUFLEN (1024 * 16)


int16_t sg_packframe(char * msg_send, mqtt_data_info_s* info);

uint8_t* GetRecvBuf(uint16_t usPos);
uint16_t GetRecvNum(void);
void SetRecvCnt(uint16_t usRecvCnt);
void UnPackFrame(void);
uint32_t Recv(uint8_t* pBuf, uint32_t ulSize, uint16_t usPos);

#ifdef __cplusplus
}
#endif

#endif
