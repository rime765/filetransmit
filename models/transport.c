#include "../include/transport.h"
#include "../include/protocol.h"


int transS_payload(T_SOCKEET sock, void *buf, int len){
    char* send_buf=(char*)buf;
    printf("%d\n",len);
    while (len>0) {
        int chunk;
        chunk=len>=INT16_MAX ?
                INT16_MAX
                :len;
        //printf("%d    %d\n", __LINE__, chunk);
        int n=send(sock, send_buf, chunk, 0);
        if (n==-1) {
            T_ERROR_Print("Transport payload:send error\n");
            return -1;
        }
        len-=n;
        send_buf+=n;
    }
    return 0;
}


int transR_payload(T_SOCKEET sock, void *buf, int len){
    if (buf==NULL) {
        return -1;
    }
    char* recv_buf=(char*)buf;
    printf("%d\n", len);
    while (len>0) {
        int chunk;
        chunk=  len>=INT16_MAX ?
                INT16_MAX
                :len;
        //printf("%d    %d\n", __LINE__, chunk);
        int n=recv(sock, recv_buf, chunk, 0);
        if (n==-1) {
            T_ERROR_Print("Transport payload:recv error\n");
            return -1;
        }
        else if (n==0) {
            printf("Receive An empty payload\n");
            return -1;
        }
        
        len-=n;
        recv_buf+=n;
    }
    return 0;
}

int transS_all(T_SOCKEET sock,MSGhead_t msghead,void*payload){

    int len=msghead.payload_size=payload==NULL?0:strlen((char*)payload)+1;
    printf("负载大小payload size:%d\n",len);
    MSG_hton(&msghead);

    int send_head=send(sock, (char*)&msghead, sizeof(MSGhead_t), 0);
    if (send_head==0) {
        printf("Empty msghead\n");
        return -1;
    }
    if (send_head<0) {
        T_ERROR_Print("Send message head error:");
        t_exit();
        return -1;
    }
    if (payload!=NULL) {
        /* ⚠️ BUG: msghead 在 54 行已被 MSG_hton,这里 msghead.payload_size
           是网络字节序(小端上是大数字如 285212672),却被当成"要发送的字节数"。
           应使用 MSG_hton 之前保存的主机序长度,而不是 msghead.payload_size */
        int send_payload=transS_payload(sock, payload, len);
        /* ⚠️ BUG: 判断的是 send_head,应判断 send_payload(本函数返回值) */
        if (send_payload<0) {
            printf("Payload send error\n");
            return -1;
        }    
    }
    return 0;

}


int transR_all(T_SOCKEET sock,MSGhead_t* msghead,void**payload){
    *payload=NULL;
    int headR_status=recv(sock, (char*)msghead, sizeof(MSGhead_t), 0);
    
    MSG_ntoh(msghead);
    
        if(msghead->magic!=PROTO_MAGIC){
            printf("Magic number error\n");
            return -1;
        }
        if(msghead->msg_status!=STATUS_OK){
            printf("Message status error\n");
            return -1;
        }

        if (headR_status==0) {
            printf("recv Empty msghead\n");
            return -1;
        }
        if (headR_status<0) {
            T_ERROR_Print("recv message head error:");
            t_exit();
        return -1;
        }
    *payload=malloc(sizeof(char)*msghead->payload_size);
    int payloadR_status=transR_payload(sock, *payload, msghead->payload_size);
    if (payloadR_status<0) {
        printf("Payload receive error\n");
        t_exit();
        return -1;
    }
    return 0;
}