#include "../include/transport.h"


int transS_payload(T_SOCKEET sock, void *buf, int len){
    char* send_buf=(char*)buf;
    
    while (len>0) {
        int chunk;
        chunk=len>=INT_MAX?
                INT_MAX
                :len;
        int n=send(sock, send_buf, INT_MAX, 0);
        if (n==-1) {
            T_ERROR_Print("Transport payload:send error\n");
            return -1;
        }
        len-=chunk;
        send_buf+=chunk;
    }
    return 0;
}


int transR_payload(T_SOCKEET sock, void *buf, int len){
    if (buf==NULL) {
        return -1;
    }
    char* recv_buf=(char*)buf;
    
    while (len>0) {
        int chunk;
        chunk=len>=INT_MAX?
                INT_MAX
                :len;
        int n=recv(sock, recv_buf, INT_MAX, 0);
        if (n==-1) {
            T_ERROR_Print("Transport payload:recv error\n");
            return -1;
        }
        else if (n==0) {
            printf("Receive An empty payload\n");
            return -1;
        }
        
        len-=chunk;
        recv_buf+=chunk;
    }
    return 0;
}

int transS_all(T_SOCKEET sock,MSGhead_t msghead,void*payload){
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
        int send_payload=transS_payload(sock, payload, msghead.payload_size);
        if (send_head<0) {
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
    int payloadR_status=transS_payload(sock, *payload, msghead->payload_size);
    if (payloadR_status<0) {
        printf("Payload receive error\n");
        t_exit();
        return -1;
    }
    return 0;
}