#ifndef	_SOCKET_COMM_H_
#define	_SOCKET_COMM_H_



#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET16_LEN 16                  //���¶���궨��

// socket��ַ�ṹ
typedef struct _s_socket_param
{
    unsigned char	type;				// 0:TCP; 1:UDP;2:TCPSERVER
    unsigned short  portNumber;			// �˿ں�
    char			IPAddress[SOCKET16_LEN];		// IP��ַ����"255.255.255.255"
}s_socket_param;

// �ͻ������Ӳ���
typedef struct _s_link_sock
{
    int				s;					// socket��
    s_socket_param	param;				// Զ�˿ͻ������Ӳ���
}s_link_sock;

enum SOCKET_CONNECT_STATE {
    SOCKET_NOTCONNECTTED = -1,
    SOCKET_CONNECTTING,
    SOCKET_CONNECTNORMAL
};
// ����ͨ��Э������
enum
{
    Link_Unknown = -1,
    TCP_TYPE = 0,
    UDP_TYPE = 1,
    TCPSERVER_TYPE = 2,
};

int sg_get_connect_state(void);

void sg_sockket_init(void);
int sg_sockket_exit(void);

//�ͻ���SOCKET����
void sg_set_socket_param(s_socket_param sockaddr, int ServerSocketFlag);

// �������ӣ����أ�1�����ӳɹ���0���������ӣ���1������ʧ��
int sg_connect_socket(void);

// �������ݣ����أ����յ������ֽڸ�����������-1����ʾ�����ж�
int sg_read_data(char* data, int count);

// �������ݣ����أ�ʵ�ʷ��������ֽڸ�����������-1����ʾ�����ж�
int sg_write_data(const char* data, int count);

int sg_cloese_socket(void);


#ifdef __cplusplus
}
#endif 

#endif
