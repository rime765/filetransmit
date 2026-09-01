#include "../include/platform.h"
#include "../include/protocol.h"
#define DEFAULT_PORT 7889


        #pragma comment(lib, "Ws2_32.lib")

int main ()
{
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

    struct sockaddr_in connect_addr;
    connect_addr.sin_port=htons(DEFAULT_PORT);
    connect_addr.sin_family=AF_INET;
    inet_pton(AF_INET, "172.25.246.128", &connect_addr.sin_addr);

    if(
    -1==bind(conetsock,(struct sockaddr*)&connect_addr,sizeof(connect_addr))
    ){
        T_ERROR_Print("socket bind error\n");
        T_CloseSocket(conetsock);
        t_exit();
        return -1;
    }
    puts("socket bind success\n");

    listen(conetsock, 10);
    puts("Listening...\n");
    while(1){
        struct sockaddr comuc_addr;
        socket_len communicate_len=sizeof(comuc_addr);
        T_SOCKEET comuc_socket=
        accept(conetsock, &comuc_addr, &communicate_len);
        if (conetsock==T_SOCKET_ERROR||conetsock==T_SOCKET_INVALID) {
            T_ERROR_Print("Connect error");
            t_exit();
            return -1;
        }

        puts("Connect success");
        MSGhead_t recv_buf;



        recv(comuc_socket, (char*)&recv_buf,sizeof(recv_buf),0);
        MSG_ntoh(&recv_buf);                /* 接收后转回主机字节序 */

        if (recv_buf.msg_type==MSG_CWD_REQ) {
            puts("recved\n");
            MSGhead_t send_buf;
            MSG_init(&send_buf, MSG_CWD_RESP,STATUS_OK );
            MSG_hton(&send_buf);            /* 发送前转网络字节序 */

            send(comuc_socket, (char*)&send_buf, sizeof(send_buf), 0);
        }


        t_exit();
        return 0;

        
    }
}
