#include "vos_typdef.h"
#include "rmt_frame.h"
#include "sgdev_struct.h"

uint32_t g_FaultFlag;
const int32_t	FRAME_HEAD = 0x68;		// ֡ͷ

uint8_t	    m_byRecvBuf[RECVBUFLEN];
uint16_t	m_usRecvCnt = 0;		// ���ջ������ڴ���������ݸ���
uint16_t	m_usProcCnt = 0;		// ���ջ��������Ѿ���������ݸ���
uint16_t	m_usRecvNum = 0;		// �������ֽڸ���
uint8_t	    byRecvBuf[RECVBUFLEN];	// ���ջ�����

                     // ����������
uint8_t     m_byAssistBuf[RECVBUFLEN];
uint16_t    m_usAssistCnt = 0;		// ����������ʵ�ʽ��ո���
int32_t			dwRecvCnt = 0;					//�����ֽ���
mqtt_data_info_s        m_faultinfo = { 0 };
uint8_t         m_isCmdCome = 0;


clock_t		m_dwLastCount;	//�ϴ�ʱ��
uint8_t		m_bValidFlag;	//��ʱ��־
uint32_t	m_dwDiffParam;	//��ʱ���


static uint8_t FrmHeadToApciInf(uint8_t *pFrmhead)
{
    // 	bool bRet = true;


    //    VOS_UINT8 byHead = *(VOS_UINT8*)(pFrmhead);
    // 	if (byHead != FRAME_HEAD)
    // 		return !bRet;

    // 	//�ж�У����
    // 	VOS_INT16 nFrmLength = *(VOS_UINT16*)(pFrmhead + 1);
    // 	VOS_INT32 nReadCnt = nFrmLength + 4;


    // 	VOS_UINT16 wCheckSum = 0;
    // 	VOS_UINT8* pcal = (VOS_UINT8*)(pFrmhead);
    //    VOS_UINT32 i;

    // 	for (i = 0; i < nReadCnt; i++)
    // 	{
    // 		wCheckSum += pcal[i] % 256;
    // 	}
    // 	VOS_UINT8 readCheck = *(VOS_UINT8*)(pFrmhead + nReadCnt);
    // 	VOS_UINT8 readTail = *(VOS_UINT8*)(pFrmhead + nReadCnt + 1);
    // 	if (readTail != 0x0A)
    // 		return !bRet;
    // 	if (GETLOBYTE(wCheckSum) != readCheck)
    // 		return !bRet;

    // 	return bRet;
}

static void ProcBuf()
{
    // ���ջ�����βָ��
    uint16_t usTail = m_usRecvCnt - m_usAssistCnt;
    //assert(usTail >= 0 && usTail<RECVBUFLEN);

    int index;

    // �����������������ݿ��������ջ�������
    memcpy(&m_byRecvBuf[usTail], m_byAssistBuf, m_usAssistCnt);

    // ��λ����������
    memset(m_byAssistBuf, 0, RECVBUFLEN);
    m_usAssistCnt = 0;
}

static void AfterUnpackProc()
{
    // ���ջ�����δ�������ݵ�ͷָ��
    uint16_t usHead = m_usProcCnt;

    // ������ջ��������Ѿ����������
    memcpy(m_byAssistBuf, &m_byRecvBuf[usHead], m_usRecvCnt);
    memset(m_byRecvBuf, 0, RECVBUFLEN);
    memcpy(m_byRecvBuf, m_byAssistBuf, m_usRecvCnt);
    memset(m_byAssistBuf, 0, RECVBUFLEN);

    // ��λ��־λ
    m_usProcCnt = 0;
}

uint8_t* GetRecvBuf(uint16_t usPos)
{
    //assert(usPos < RECVBUFLEN);
    return &m_byAssistBuf[usPos];
}


uint16_t GetRecvNum(void)
{
    return (RECVBUFLEN - m_usRecvCnt);
}

void SetRecvCnt(uint16_t usRecvCnt)
{
    //assert((m_usRecvCnt + usRecvCnt <= RECVBUFLEN));

    m_usRecvCnt += usRecvCnt;

    m_usAssistCnt += usRecvCnt;
}

uint32_t Recv(uint8_t* pBuf, uint32_t ulSize, uint16_t usPos)
{
    int index;
    memcpy(&m_byAssistBuf[usPos], pBuf, ulSize);
    return 0;
}

void UnPackFrame(void)
{
    int index = 0;
    // 1.��������
    ProcBuf();

    // 2.���Խ�֡
    uint16_t usPackPos = 0;
    bool bExit = false;
    uint16_t usRecvCnt = m_usRecvCnt;

    while (usPackPos < usRecvCnt)
    {
        if (m_usRecvCnt < 1)
            break;
        printf("HEAD === %0x \n", m_byRecvBuf[usPackPos]);

        if (FRAME_HEAD == m_byRecvBuf[usPackPos])
        {
            //��֡ͷ������֡        
     //�ж�����֡
            if (FrmHeadToApciInf(&m_byRecvBuf[usPackPos]))
            {
                uint8_t *pFrmhead = &m_byRecvBuf[usPackPos];
                int16_t nFrmLength = *(uint16_t*)(pFrmhead + 1);

                nFrmLength = nFrmLength + 6;
                if (nFrmLength <= m_usRecvCnt)
                {
                    dwRecvCnt = nFrmLength;
                    memcpy(byRecvBuf, &m_byRecvBuf[usPackPos], dwRecvCnt);

                    //UnPackFrame_I(byRecvBuf, dwRecvCnt);
                    //ʵ�ʽ�֡

               // for(index = 0; index < usRecvCnt; index++)
               // {
               //    printf("buffer[%d] is %0x.\n", index,byRecvBuf[index]);
               // }

                    m_usProcCnt += nFrmLength;
                    m_usRecvCnt -= nFrmLength;
                    usPackPos += nFrmLength;
                } else
                {
                    bExit = true;
                }
            } else
            {
                usPackPos += 1;
                m_usProcCnt += 1;
                m_usRecvCnt -= 1;
            }
        } else
        {
            usPackPos += 1;
            m_usProcCnt += 1;
            m_usRecvCnt -= 1;
        }

        if (bExit)
            break;
    }
    // 3.��֡����
    AfterUnpackProc();
    //m_usRecvCnt = 0;
}


int16_t sg_packframe(char * msg_send, mqtt_data_info_s* info)
{

    int16_t nlen = 0;
    //   VOS_INT16 nSendCnt = 0;
    //   VOS_UINT8	 byAppBuf[8192] = { 0 };
    //   int i = 0;

    //   VOS_UINT8 *pAppHead = byAppBuf;

    //   //��֡ͷ
    //   *(pAppHead + nSendCnt++) = 0x68;
    //   nSendCnt = nSendCnt + 2;//����
    //   *(pAppHead + nSendCnt++) = info->type;


    //   //id
    //   *(VOS_UINT16*)(pAppHead + nSendCnt) = strlen(info->resource) + 1;
    //   nSendCnt = nSendCnt + 2;//����
    //   memcpy(pAppHead + nSendCnt, info->resource,strlen(info->resource) +1);
    //   nSendCnt = nSendCnt + strlen(info->resource) + 1;

    //   printf("resource data: %s \n",info->resource);

    //   //����
    //   *(VOS_UINT16*)(pAppHead + nSendCnt) = strlen(info->jsonContent) + 1;
    //   nSendCnt = nSendCnt + 2;//����
    //   memcpy(pAppHead + nSendCnt, info->jsonContent,strlen(info->jsonContent) +1);
    //   nSendCnt = nSendCnt + strlen(info->jsonContent) + 1;

    //   // printf("strlen(info->jsonContent): %d \n",strlen(info->jsonContent));
    //   // printf("jsonContent data: %s \n",info->jsonContent);

    //   *(VOS_UINT16*)(pAppHead + 1) = strlen(info->resource) + 1 + strlen(info->jsonContent) + 1 + 4;

    //   if (nSendCnt > 0)
    //   {
    //      VOS_UINT16 wCheckSum = 0;
    //      VOS_UINT8* pcal = pAppHead;
    //      for (i = 0; i < nSendCnt; i++)
    //      {
    //         wCheckSum += pcal[i] % 256;
    //      }
    //      *(pAppHead + nSendCnt++) = GETLOBYTE(wCheckSum);
    //      *(pAppHead + nSendCnt++) = 0x0a;

    //   }



    //   nlen = nSendCnt;  
    //   memcpy(msg_send,pAppHead,8192);
    printf("nlen = %d \n", nlen);
    return nlen;
}
