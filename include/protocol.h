#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "platform.h"
#include <stdint.h>


// 数据包头魔数 TINY
#define PROTO_MAGIC 0x54494E59

// 宏值
#define MAX_PATH_LEN  256  // 最大路径长度
#define DATA_BUF_SIZE 8192 // 数据缓冲区大小

// 数据包类型 采用请求+响应模式
#define MSG_LIST_REQ  0x01  // 例举请求
#define MSG_LIST_RESP 0x02  // 例举响应
#define MSG_CD_REQ    0x03  // 切换目录请求
#define MSG_CD_RESP   0x04  // 切换目录响应
#define MSG_GET_REQ   0x05  // 获取文件请求
#define MSG_GET_RESP  0x06  // 获取文件响应
#define MSG_PUT_REQ   0x07  // 上传文件请求
#define MSG_PUT_RESP  0x08  // 上传文件响应
#define MSG_FILE_DATA 0x09  // 文件数据
#define MSG_FILE_DONE 0x0A  // 文件完成
#define MSG_CWD_REQ   0x0B  // 当前目录请求
#define MSG_CWD_RESP  0x0C  // 当前目录响应
#define MSG_ERROR     0xFF  // 错误响应

// 数据包状态
#define STATUS_OK            0x0000 // 成功
#define STATUS_ERROR         0xFFFF // 错误
#define STATUS_NOT_FOUND     0x0100 // 未找到文件或目录
#define STATUS_ACCESS_DENIED 0x0200 // 沒有访问权限
#define STATUS_INVALID_PATH  0x0300 // 无效路径

#pragma pack()

typedef struct{
    uint32_t magic;
    uint16_t msg_type;
    uint16_t msg_status;
    uint64_t filesize;
    uint32_t payload_size;
    char path[MAX_PATH_LEN];
} MSGhead_t;
/*初始化数据包头*/
int MSG_init(MSGhead_t* packhead,uint16_t msg_type, uint16_t msg_status) ;
/*转换到网络字节序*/
int MSG_hton(MSGhead_t *host_MSGhead);
/*转换回主机字节序*/
int MSG_ntoh(MSGhead_t *net_MSGhead);
/*状态转换为字符串*/
const char* MSG_status_str(uint16_t status);
/*类型转换为字符串 */
const char* MSG_type_str(uint16_t type);


#endif