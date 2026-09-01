#include "../include/platform.h"
#include "../include/protocol.h"
#pragma comment(lib, "ws2_32.lib")
#define DEFAULT_PORT 7889

int main(){
    t_init();
    if(t_init()!=0){
        T_ERROR_Print("platfrom init error.");
        t_exit();
        return -1;
        }

    puts("Platform init success\n");

    T_SOCKEET conetsock=socket(AF_INET, SOCK_STREAM, 0);
    
    if(conetsock==T_SOCKET_ERROR||conetsock==T_SOCKET_INVALID){
        T_ERROR_Print("socket: creat error\n");
        printf("%d",(int)conetsock);
        T_CloseSocket(conetsock);
        t_exit();
        return -1;
    }
    puts("socket Create success\n");

    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(DEFAULT_PORT);
    inet_pton(AF_INET, "172.25.246.128",&server_addr.sin_addr);
    

    if (connect(conetsock, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        T_ERROR_Print("connect erro\n");
        t_exit();
    }
    
    puts("Connect successfully");
    
    MSGhead_t sendbuf,recv_buf;
    MSG_init(&sendbuf,MSG_CWD_REQ , STATUS_OK);
    MSG_hton(&sendbuf);                         /* 发送前转网络字节序 */
    send(conetsock, (char*)&sendbuf, sizeof(sendbuf),0);
    recv(conetsock, (char*)&recv_buf,sizeof(recv_buf),0);
    MSG_ntoh(&recv_buf);
    if (recv_buf.msg_type==MSG_CWD_RESP) {
        puts("ack access");
    }
    return 0;
}