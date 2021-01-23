#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "vos_typdef.h"
#include "vos_errno.h"
#include "vrp_mem.h"
#include "vrp_event.h"
#include "sgdev_struct.h"
#include "rmt_socket.h"


typedef socklen_t SOCKLEN;

typedef struct sockaddr SA;


struct sockaddr_in		m_sockaddr;
int				m_socket;

unsigned char	m_Type;	// 0:TCP; 1:UDP
fd_set	m_readSet;		//����¼��
fd_set  m_writeSet;		//д��¼��

fd_set  m_exceptSet;	//�쳣
struct timeval	m_tTimeout;		//�����߳����������������ʱ��

int m_isAlreadyListen;

//socket����
int m_sockettype;		// 0:clientsocket 1:serversocket
// ����״̬
int	m_ConnectState;
// �Է�����Ϣ
s_socket_param m_clientparam;



// ��д�豸��ʼ��
static void sg_init_rw_set(long nSec, long uSec)
{
    FD_ZERO(&m_readSet);
    FD_SET(m_socket, &m_readSet);
    FD_ZERO(&m_writeSet);
    FD_SET(m_socket, &m_writeSet);
    FD_ZERO(&m_exceptSet);
    FD_SET(m_socket, &m_exceptSet);

    m_tTimeout.tv_sec = nSec;//���߳��еĶ�ʱ�ȴ�
    m_tTimeout.tv_usec = uSec;
}


// ��Socket��ַ�ṹת����׼�ṹ
static void sg_convert_sock_addr(s_socket_param addr1, int ListenFlag)
{
    m_sockaddr.sin_family = AF_INET; //AF_UNIX
    //
    m_sockaddr.sin_port = htons(addr1.portNumber);
    m_sockaddr.sin_addr.s_addr = inet_addr(addr1.IPAddress);
}

// �Ƿ��Ѿ�����
int sg_get_connect_state(void)
{
    return m_ConnectState;
}

// �Ͽ�����
int sg_cloese_socket()
{
    if (0 == m_socket)
        return 0;

    close(m_socket);
    m_isAlreadyListen = 0;
    m_socket = 0;
    m_ConnectState = SOCKET_NOTCONNECTTED;
    return 0;
}

static int sg_init_socket(void)
{
    if (!m_socket)
    {
        if (m_Type == UDP_TYPE)
        {
            m_socket = socket(AF_INET, SOCK_DGRAM, 0);
        } else // if (m_Type == TCP_TYPE)
        {
            m_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (0 == m_socket)
                m_socket = socket(AF_INET, SOCK_STREAM, 0);
        }

        if (m_socket > 0)
        {
            // ���÷�������ʽ
            if (fcntl(m_socket, F_SETFL, O_NONBLOCK) < 0)
            {
                sg_cloese_socket();
            }
            // ����IP
            if (m_sockettype)
            {
                const int bBroadcast = 1;
                if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&bBroadcast, sizeof(bBroadcast)) != 0)
                    sg_cloese_socket();
            }
            errno = 0;
        }

        else
        {
            // ���������m_socket��ЧʱӦ����-1 ,�˴���ֵ0�д��о���
            m_socket = 0;
        }
    }
    return m_socket;
}


void sg_sockket_init(void)
{
    m_socket = 0;
    m_Type = Link_Unknown;
    m_ConnectState = SOCKET_NOTCONNECTTED;
}

int sg_sockket_exit()
{
    if (m_socket > 0)
    {
        return sg_cloese_socket();
    }
}

void sg_set_socket_param(s_socket_param sockaddr, int ServerSocketFlag)
{
    m_Type = sockaddr.type;		// ����Э������
    m_socket = 0;
    m_ConnectState = SOCKET_NOTCONNECTTED;
    // memset(&m_clientparam,0,sizeof(m_clientparam));

    sg_convert_sock_addr(sockaddr, ServerSocketFlag);
    m_sockettype = ServerSocketFlag;
    m_isAlreadyListen = 0;
}

int sg_connect_socket(void)
{
    int error = 1;
    int bFlag = 1;
    int len = sizeof(int);
    int nResult = 0;
    if (m_sockettype)// ����socket
        return SOCKET_NOTCONNECTTED;
    if (m_ConnectState == SOCKET_CONNECTTING)			// ��������
    {
        sg_init_rw_set(0, 1);// ��ʼ����ʱʱ���fd_set
        nResult = select(m_socket + 1, NULL, &m_writeSet, NULL, &m_tTimeout);
        if (nResult <= 0)								// ����û�н���
            sg_cloese_socket();
        else if (nResult > 0)
        {
            if (FD_ISSET(m_socket, &m_writeSet))
            {
                m_ConnectState = SOCKET_CONNECTNORMAL;
                getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t*)&len);
                if (!error && bFlag)
                    m_ConnectState = SOCKET_CONNECTNORMAL;
                else
                    sg_cloese_socket();
            } else
                sg_cloese_socket();
        }
    } else if (m_ConnectState == SOCKET_NOTCONNECTTED)
    {
        if (sg_init_socket())							// socket��ʼ��
        {
            int ret = connect(m_socket, (SA*)&m_sockaddr, sizeof(m_sockaddr));
            if (ret < 0)
            {
                if (errno == EINPROGRESS || errno == 0)					// ��������ʽ�����Ӳ����������
                    m_ConnectState = SOCKET_CONNECTTING;
                else// ����ʧ��
                    sg_cloese_socket();
            } else// ���ӳɹ�
                m_ConnectState = SOCKET_CONNECTNORMAL;
        }
    }
    return m_ConnectState;
}

// ��������
int sg_read_data(char* data, int count)
{
    int readCount = 0;
    sg_init_rw_set(0, 1);
    int nResult = select(m_socket + 1, &m_readSet, NULL, &m_exceptSet, &m_tTimeout);
    if (nResult < 0)
    {
        readCount = -1;
    } else if (nResult > 0)
    {
        if (FD_ISSET(m_socket, &m_exceptSet))
        {
            sg_cloese_socket();
            return -1;
        }
        if (FD_ISSET(m_socket, &m_readSet))
        {
            readCount = recv(m_socket, (char*)data, count, 0);
            if (readCount <= 0)
            {
                if (errno == ENOBUFS || errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    readCount = 0;
                } else
                {
                    readCount = -1;
                }
            }
        } else
        {
            readCount = -1;
        }
    } else
    {
        if (FD_ISSET(m_socket, &m_exceptSet))
        {
            readCount = -1;
        }
    }
    if (readCount == -1)
        sg_cloese_socket();
    return readCount;
}

// ��������
int sg_write_data(char* data, int count)
{
    int writeCount = -1;
    sg_init_rw_set(0, 1);
    int nResult = select(m_socket + 1, NULL, &m_writeSet, &m_exceptSet, &m_tTimeout);
    if (nResult < 0)
    {
        // ������
        char tError[32];
        sprintf(tError, "socket error id ��%d!!!!", errno);
        writeCount = -1;
    } else if (nResult > 0)
    {
        if (FD_ISSET(m_socket, &m_exceptSet))
        {
            sg_cloese_socket();
            return -1;
        }
        if (FD_ISSET(m_socket, &m_writeSet))
        {
            writeCount = send(m_socket, data, count, 0);
            if (writeCount < 0)
            {
                if (errno == EWOULDBLOCK)
                {
                    writeCount = 0;
                }

                else
                {
                    writeCount = -1;
                }
            }
        } else
            writeCount = -1;
    } else
    {
        if (FD_ISSET(m_socket, &m_exceptSet))
        {
            writeCount = -1;
        }
    }
    if (writeCount == -1)
        sg_cloese_socket();
    return writeCount;
}






