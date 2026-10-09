#include "../include/platform.h"
#include "../include/protocol.h"
#include "../include/transport.h"
#pragma comment(lib, "ws2_32.lib")
#define DEFAULT_PORT 7889

int main(){
    T_CHCP;
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
    char payload[]=" 8j4eh239yasfj";
    /* ⚠️ 注意:sizeof(payload) 含结尾 '\0',=17;若只要字符数应 strlen(payload) 或 sizeof-1。
       此处数值本身正确(所以外面打印是 17)。真正的"大数字/内容乱"来自 transS_all 把
       MSG_hton 后的网络序 payload_size 当发送长度(见 models/transport.c 注释) */


    
    transS_all(conetsock, sendbuf, (void*)payload);
    void *recv_payload=NULL;
    transR_all(conetsock, &recv_buf, &recv_payload);
    puts((char*)recv_payload);
    return 0;
}
