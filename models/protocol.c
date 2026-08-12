#include "../platorm.h"
#include "../protocol.h"

/*初始化数据包头*/
int MSG_init(MSGhead_t packhead) {
    packhead.magic=
}
/*转换到网络字节序*/
int MSG_hton(MSGhead_t host_MSGhead);
/*转换回主机字节序*/
int MSG_ntoh(MSGhead_t net_MSGhead);
/*状态转换为字符串*/
void MSG_status_str(uint16_t status);
/*类型转换为字符串 */
void MSG_type_str(uint16_t type);
