#include "../include/platform.h"
#include "../include/protocol.h"
#include "../include/transport.h"
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
        struct sockaddr comuc_addr;
        socket_len communicate_len=sizeof(comuc_addr);
        T_SOCKEET comuc_socket;
  

    comuc_socket=accept(conetsock, &comuc_addr, &communicate_len);
    /* ⚠️ BUG: 判断的是 conetsock(监听 socket),应判断 accept 返回的 comuc_socket。
       且习惯上应判 ==T_SOCKET_INVALID(不是 ==T_SOCKET_ERROR),否则新连接的错误会被漏检 */
    if (comuc_socket==T_SOCKET_ERROR||comuc_socket==T_SOCKET_INVALID) {
        T_ERROR_Print("Connect error");
        t_exit();
        return -1;
    }
    
    
    puts("Connect success\n");

    /* ⚠️ 提示:printf 不带 '\n' 且未 fflush(stdout) 时,若后续段错误/异常退出,
       这行输出可能被缓冲丢弃、看起来"没执行"(实际执行了)。调试建议加 '\n' 或 fflush */

    char* recv_buf;   /* ⚠️ 未初始化;这里会用函数内部 malloc 的地址,严格应先置 NULL */
    char* send_buf="server has recieved\n";
    
    MSGhead_t recv_head,send_head;   /* ⚠️ send_head 未初始化,未 MSG_init,payload_size 是栈上垃圾 */

    transR_all(comuc_socket, &recv_head, (void*)&recv_buf);

    puts(recv_buf);
    MSG_init(&send_head, 0, STATUS_OK);
    /* ⚠️ BUG1: send_head 未初始化(未 MSG_init),其 payload_size 是栈上垃圾,
       会被 transS_all 当发送长度用 → 出现"短字符串算出大长度"。
       应先 MSG_init(&send_head, ..., ...) 并 send_head.payload_size=strlen(send_buf)。
       ⚠️ BUG2: send_buf 是字符串字面量,长度应填 strlen(send_buf),不是 sizeof */
    transS_all(comuc_socket, send_head, send_buf);
        
}
