#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "platform.h"
#include "protocol.h"

int transS_payload(T_SOCKEET sock,void*buf,int len);

int transR_payload(T_SOCKEET sock,void*buf,int len);

int transS_all(T_SOCKEET sock,MSGhead_t msghead,void*payload);


int transR_all(T_SOCKEET sock,MSGhead_t* msghead,void**payload);
//接收端必须用payload二级指针,因为接收方应用城不知道要收多少.

#endif
