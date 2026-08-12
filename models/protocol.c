#include "../protocol.h"
#include <WinSock2.h>
#include <stdint.h>
#include <winbase.h>

/*初始化数据包头*/
int MSG_init(MSGhead_t *packhead,uint16_t msg_type, uint16_t msg_status) {
	memset(packhead,0,sizeof(*packhead));

	packhead->magic     =PROTO_MAGIC;
	packhead->msg_status=msg_status;
	packhead->msg_status=msg_status;
}
/*转换到网络字节序*/
int MSG_hton(MSGhead_t* host_MSGhead){
	host_MSGhead->msg_status	=htons(host_MSGhead->msg_status);
	host_MSGhead->magic			=htonl(host_MSGhead->magic);
	host_MSGhead->msg_type		=htons(host_MSGhead->msg_type);
	host_MSGhead->payload_size	=htonl(host_MSGhead->payload_size);

	uint32_t L_filesize=host_MSGhead->filesize & ~0x1111111100000000;
	uint32_t H_filesize=host_MSGhead->filesize >> 32;

	host_MSGhead->filesize=htonl(L_filesize)   | (uint64_t)htonl(H_filesize)<<32;
	return 1;
}
/*转换回主机字节序*/
int MSG_ntoh(MSGhead_t* net_MSGhead){
	net_MSGhead->msg_status		=ntohs(net_MSGhead->msg_status);
	net_MSGhead->magic			=ntohl( net_MSGhead->magic);
	net_MSGhead->msg_type		=ntohs(net_MSGhead->msg_type);
	net_MSGhead->payload_size	=ntohl(net_MSGhead->payload_size);

	uint32_t L_filesize=net_MSGhead->filesize & ~0x1111111100000000;
	uint32_t H_filesize=net_MSGhead->filesize >> 32;

	net_MSGhead->filesize=ntohl(L_filesize)   | (uint64_t)ntohl(H_filesize)<<32;
	return 1;
}
/*状态转换为字符串*/
const char* MSG_status_str(uint16_t status){
	switch (status) {
	case STATUS_ACCESS_DENIED:	return "ACCESS_DENIED" ;
	case STATUS_ERROR		 :	return "Error";
	case STATUS_INVALID_PATH :	return "Invalid path";
	case STATUS_OK			 :	return "OK";
	case STATUS_NOT_FOUND	 :	return  "Not found";
	default:					return "Unknown";
	}
}
/*类型转换为字符串 */
const char* MSG_type_str(uint16_t type){
	switch (type) {
	case MSG_LIST_REQ 	:return "List request";
	case MSG_LIST_RESP 	:return "List request response";
	case MSG_CD_REQ   	:return "CD request";
	case MSG_CD_RESP  	:return "CD request response";
	case MSG_GET_REQ   	:return "Get request";
	case MSG_GET_RESP  	:return "Get request response.";
	case MSG_PUT_REQ  	:return "Put response";
	case MSG_PUT_RESP 	:return "Put request response.";
	case MSG_FILE_DATA 	:return "Fair data";
	case MSG_FILE_DONE	:return "File data response.";
	case MSG_CWD_REQ   	:return "Get CWD";
	case MSG_CWD_RESP 	:return "Get CWD response";
	case MSG_ERROR    	:return "Message error";
	default				:return "Unknown message";
	}
}
