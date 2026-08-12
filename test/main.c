#include "../include/platform.h"
#include "../include/protocol.h"
#define DEFAULT_PORT 7889

int main ()
{
    t_init();
    if(t_init()!=1){
        T_ERROR_Print("Socket create error.");
        t_exit();
        return -1;
        }

    puts("平台初始化成功\n");

    T_SOCKEET conetsock=socket(AF_INET, SOCK_STREAM, 0);
    
    if(conetsock!=T_SOCKET_SUCCESS){
        T_ERROR_Print("套接字创建失败\n");
        T_CloseSocket(conetsock);
        t_exit();
        return -1;
    }
    puts("套接字创建成功\n");

    struct sockaddr_in connect_addr;
    connect_addr.sin_port=htons(DEFAULT_PORT);
    connect_addr.sin_family=AF_INET;
    connect_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    if(
    -1==bind(conetsock,(struct sockaddr*)&connect_addr,sizeof(connect_addr))
    ){
         T_ERROR_Print("套接字绑定失败\n");
        T_CloseSocket(conetsock);
        t_exit();
        return -1;
    }
    puts("套接字绑定成功");

    listen(conetsock, 10);

    while(1){
        struct sockaddr comuc_addr;
        socket_len communicate_len=sizeof(comuc_addr);
        T_SOCKEET comuc_socket=
        accept(conetsock, &comuc_addr, &communicate_len);
        if (comuc_socket!=0) {
        puts("连接成功");
        MSGhead_t recv_buf;
        recv(comuc_socket, (char*)&recv_buf,sizeof(MSGhead_t),0);
        MSG_ntoh(&recv_buf);
        MSG_status_str(recv_buf.msg_status);
        puts("传输完成");
        t_exit();
        return 0;
        }
        
    }

    
}
