#ifndef	_SOCKET_COMM_H_
#define	_SOCKET_COMM_H_



#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET16_LEN 16                  //重新定义宏定义

// socket地址结构
typedef struct _s_socket_param
{
    unsigned char	type;				// 0:TCP; 1:UDP;2:TCPSERVER
    unsigned short  portNumber;			// 端口号
    char			IPAddress[SOCKET16_LEN];		// IP地址，如"255.255.255.255"
}s_socket_param;

// 客户端连接参数
typedef struct _s_link_sock
{
    int				s;					// socket号
    s_socket_param	param;				// 远端客户端连接参数
}s_link_sock;

enum SOCKET_CONNECT_STATE {
    SOCKET_NOTCONNECTTED = -1,
    SOCKET_CONNECTTING,
    SOCKET_CONNECTNORMAL
};
// 网络通信协议类型
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

//客户端SOCKET设置
void sg_set_socket_param(s_socket_param sockaddr, int ServerSocketFlag);

// 建立连接，返回：1：连接成功；0：正在连接；－1：连接失败
int sg_connect_socket(void);

// 接收数据，返回：接收到数据字节个数，若返回-1，表示连接中断
int sg_read_data(char* data, int count);

// 发送数据，返回：实际发送数据字节个数，若返回-1，表示连接中断
int sg_write_data(const char* data, int count);

int sg_cloese_socket(void);


#ifdef __cplusplus
}
#endif 

#endif
