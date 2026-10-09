# tinyFT 传输层排坑手册

> 本文档汇总本项目(tinyFT,Windows/Linux 双平台 + 自研协议头 + TCP)在
> 开发过程中遇到的**传输层 / 协议层**问题、根因与修法,供后续排查复用。
> 关键文件:`include/protocol.h`、`models/transport.c`、`test/client.c`、`test/server.c`。

---

## 0. 一次看懂的排查顺序

遇到"乱码 / 数字巨大 / 段错误 / 类型判断不过"时,按下面顺序找:

```
1. 消息/字段有没有被正确写入?        → MSG_init 是否真的给该字段赋值
2. 字节序(hton/ntoh)是否成对?        → 发送前 hton / 接收后 ntoh,不能只做一半
3. 收发长度用的是不是同一次算的值?    → 别用"已被 hton 洗过"的网络序长度当发送长度
4. 结构体打包/对齐是否两端一致?       → #pragma pack(1) 强制紧凑
5. TCP 是否被当成"一次收一条"?        → 用 recv_all / send_all 循环,按实际字节数计数
6. 字符串缓冲区有没有 '\0' 终止符?    → 接收端 malloc(+1) 并补 '\0',否则 puts 越界读
7. 指针有没有被正常赋值/初始化?       → 二级指针、未初始化栈变量
```

---

## 1. 字段根本没被写入(消息类型判断不过)

### 现象
- 接收端 `msg_type` 永远匹配不上任何 `MSG_*` 宏(恒为 0)。
- 消息类型判断"全都过不了"。

### 根因
`MSG_init` 函数签名为 `(packhead, msg_type, msg_status)`,但函数体里只写了
`msg_status`,**`msg_type` 参数从未被写入结构体**;而函数开头 `memset` 把结构体清零,
于是 `msg_type` 恒为 0。

```c
// 修前(错误)
int MSG_init(MSGhead_t *packhead,uint16_t msg_type, uint16_t msg_status) {
    memset(packhead,0,sizeof(*packhead));
    packhead->magic     =PROTO_MAGIC;
    packhead->msg_status=msg_status;   // 连写两遍
    packhead->msg_status=msg_status;
    return 1;
}
```

### 修法
```c
int MSG_init(MSGhead_t *packhead,uint16_t msg_type, uint16_t msg_status) {
    memset(packhead,0,sizeof(*packhead));
    packhead->magic     =PROTO_MAGIC;
    packhead->msg_type  =msg_type;     // 补上这一行
    packhead->msg_status=msg_status;
    return 1;
}
```

### 排查技巧
- 打印 `typedef` 里每个字段,对照你是否真的赋了值。
- 用 `MSG_type_str(msg_type)` 打印,看返回 "Unknown message"。

---

## 2. 字节序(hton/ntoh)不对称

### 现象
- 单机测试没事,一旦收发路径上"该转的没转、不该转的转了",数值就变大小端错位。
- 典型:小端机上 `ntohs(0x000C)` → `0x0C00`,导致 `0x0C != MSG_CWD_RESP(0x0C)`。

### 根因
协议约定是:发送前 `MSG_hton`、接收后 `MSG_ntoh`。但实际代码里**只有接收端调了
`MSG_ntoh`,三处发送/接收该转的都没做**:

| 位置 | 发送前 hton | 接收后 ntoh |
|------|:-----------:|:-----------:|
| client 发送 | ❌ | — |
| server 接收 | — | ❌ |
| server 发送 | ❌ | — |
| client 接收 | — | ✅ 唯一做了 |

于是 server 发出主机序 `0x0C`(`0C 00`),client 收到后 `MSG_ntoh` 洗成 `0x0C00`,对不上。

### 修法
做到严格成对:发送前调 `MSG_hton`,接收后调 `MSG_ntoh`。

```c
// 发送端
MSG_init(&buf, MSG_CWD_REQ, STATUS_OK);
MSG_hton(&buf);
send(sock, (char*)&buf, sizeof(buf), 0);

// 接收端
recv(sock, (char*)&buf, sizeof(buf), 0);
MSG_ntoh(&buf);
```

### 排查技巧
- 两端各打印一次 `msg_type`(转前/转后),确认链路上是网络序、本地是主机序。

---

## 3. 用"洗过之后"的 payload_size 当发送长度

### 现象
- 外面打印 `payload_size=17`,但 `transS_all` 函数里打印却变成巨大的数字(如 285212672)。
- 发送内容混乱、长度离谱。

### 根因
`transS_all` 先执行 `MSG_hton(&msghead)`,把 `payload_size` 转成网络字节序
(小端上 `htonl(17) = 0x11000000 = 285212672`);随后又用
`msghead.payload_size` 作为要发送的**字节数**——用的是已经被转换过的网络序值。

```c
// 修前(错误)
int transS_all(...){
    MSG_hton(&msghead);
    ...
    transS_payload(sock, payload, msghead.payload_size);   // 网络序大数字
}
```

### 修法
`MSG_hton` 之前先保存一份主机序长度,发送循环用它。

```c
int transS_all(T_SOCKEET sock, MSGhead_t msghead, void*payload){
    int len = msghead.payload_size;      // 先用主机序
    MSG_hton(&msghead);                  // 包头转网络序
    send(sock,(char*)&msghead,sizeof(msghead),0);
    if (payload!=NULL)
        transS_payload(sock, payload, len);   // 用保存的主机序长度
    return 0;
}
```

### 排查技巧
- 别用 `msghead.payload_size` 打印"发送长度",要用 `MSG_hton` 之前的副本。
- 在 `MSG_hton` 前后各打印一次,看它是否被洗成网络序。

---

## 4. 结构体没有真正紧凑打包(跨平台布局不一致)

### 现象
- 本机/同编译器测正常,跨平台、跨架构(Windows 64 / Linux 32/64、不同编译器默认对齐)时,
  数据从错误的偏移被读出 → 乱码。

### 根因
`#pragma pack()` **不带参数等于恢复编译器默认对齐(通常 8 字节)**,不是 `pack(1)`。
于是 `MSGhead_t` 里仍有填充字节,`uint64_t filesize` 需要 8 字节对齐,字段偏移、
`sizeof(MSGhead_t)` 在两端可能不同,payload 就从错位处被读出来。

```c
#pragma pack()            // 恢复默认对齐,不是 1 字节
typedef struct{ uint32_t magic; uint16_t msg_type; ... } MSGhead_t;
```

### 修法
```c
#pragma pack(push,1)      // 强制 1 字节紧凑打包
typedef struct{ uint32_t magic; uint16_t msg_type; ... } MSGhead_t;
#pragma pack(pop)
```

### 排查技巧
- 两端各打印 `sizeof(MSGhead_t)` 与各字段 `offsetof(...)`,必须一致。
- 若有 `#pragma pack()` 空调用,先去掉。

---

## 5. 把 TCP 当成"一次收一条"(部分读/粘包/拆包)

### 现象
- 跨网络、跨平台时突然乱码或丢数据;本机测一般正常。
- 一次 `recv` 只收到半个包头,或把下一个包的分块一起收进来。

### 根因
TCP 是**字节流**,不是报文;一次 `recv` 返回多少字节不确定。原代码用一次
`recv(..., sizeof(MSGhead_t), ...)` 收包头,包头没循环;payload 循环里又用
`chunk` 而不是实际收到的 `n` 去减长度、挪指针,一旦 `n < chunk`(部分读)指针就错位。

```c
// 修前(错误):按 chunk 计数
while (len>0) {
    chunk = len>=INT16_MAX ? INT16_MAX : len;
    int n=recv(sock, recv_buf, chunk, 0);
    ...
    len    -= chunk;     // 应减 n
    recv_buf+= chunk;    // 应加 n
}
```

### 修法
包头也循环收满,payload 循环按实际字节数 `n` 计数。`send` 同理。

```c
// 收满指定字节
ssize_t recv_all(T_SOCKEET s, char* buf, size_t len){
    size_t got=0;
    while (got<len) {
        ssize_t n=recv(s, buf+got, len-got, 0);
        if (n<=0) return n;      // 0=对端关闭, <0=出错/中断
        got += n;
    }
    return len;
}
```

### 排查技巧
- 打印每次 `recv` 实际返回的 `n` 和目标长度,看是否出现 `n < chunk`。
- 调试期加粘包判据:魔数 `PROTO_MAGIC` 校验,无效即重新对齐/丢弃。

---

## 6. payload 没有终止符,打印越界读(Windows 端乱码高发)

### 现象
- 内容明明是纯 ASCII 英文字符,Windows 端 `puts` 出来却是乱码;Linux 端看着正常。
- **结构体没问题、字节也对,唯独打印出来在尾部带垃圾。**

### 根因
`payload_size` 用 `strlen(payload)`(不含 `'\0'`),接收端按这个长度 `malloc`,然后
`puts` 打印。`puts` 会一直读到 `'\0'` 为止,而缓冲区里根本没有 `'\0'`,于是**越界读到
堆内存后面的垃圾**。

Linux 堆内存常恰好为 0,看着正常;Windows 堆后面常是残留字节,打印即乱码。

```c
// 修前
*payload=malloc(sizeof(char)*msghead->payload_size);   // 末尾无 '\0'
...
puts((char*)*payload);   // puts 读到 '0 为止 → 越界
```

### 修法
```c
*payload=malloc(msghead->payload_size + 1);
// 收完 payload_size 字节后:
((char*)*payload)[msghead->payload_size] = '\0';
```
或发送端直接带 `'\0'`:`payload_size = strlen(payload) + 1`。

### 排查技巧
- 用 `printf("%.*s", payload_size, payload)` 限定长度打印,不依赖 `'\0'`,能立刻确认内容其实是对的。
- 逐字节打印 ASCII 码,区分是"读多了越界"还是"字节错位"。

---

## 7. 指针/未初始化变量导致的段错误

### 现象
- 段错误,且 crash 位置有时不在打印的那一行。
- 前面的 `printf("999")` 偶尔也不显示。

### 根因 A:二级指针传成了指针的值
`transR_all` 签名是 `void**payload`(由它内部 `malloc`)。调用处本应传 `&recv_buf`,
却写了 `(void*)recv_buf`(未初始化的垃圾值),函数内 `*payload=...` 写入随机地址 → 段错误。

```c
char* recv_buf;   // 未初始化
transR_all(sock, &head, (void*)recv_buf);   // 错误:应传 &recv_buf
```

### 根因 B:stdout 缓冲导致"看不到已经执行"
`printf("999")` 不带 `'\n'` 且未 `fflush`;若之后段错误(SIGSEGV),进程被信号杀死,
**不走正常退出路径,缓冲区里的 "999" 不会被刷出**,看起来像"没执行"。

### 修法
- 二级指针传 `&recv_buf`,`recv_buf` 置 `NULL`。
- 调试输出加 `'\n'` 或显式 `fflush(stdout)`。

### 排查技巧
- 段错误用调试器/`gdb` 看 backtrace,别被"输出缺失"误导。
- `printf("...\n")` 或 `fflush(stdout)` 让输出及时可见。

---

## 8. 容易忽略的小坑

- **`sizeof` vs `strlen`**:`sizeof(payload)` 含结尾 `'\0'`;要字符数用
  `strlen` 或 `sizeof-1`。字符串字面量同理。
- **`send` 部分写**:长 payload 用循环按实际发送数 `n` 计数。
- **文件内容别用 `strlen`**:传文件时要用文件长度,否则 `'\0'` 处被截断。
- **判断 accept 返回**:用返回的 `comuc_socket`,不是监听用的 `conetsock`;
  判 `==T_SOCKET_INVALID`,别判 `!=0`。
- **`T_Error()` 在 Linux 分支是空的**,报错信息会缺失,统一封装更稳。

---

## 9. 排查速查表(现象 → 优先检查点)

| 现象 | 优先检查 |
|------|----------|
| 消息类型判断不过 | §1 字段没写入 / §2 hton-ntoh 不对称 |
| 短字符串算成超大数字 | §3 用网络序长度当发送长度 / §8 未初始化结构体 |
| 跨平台乱码(结构体正常) | §4 打包 / §5 部分读 / §6 无 `\0` |
| Windows 打印乱码、Linux 正常 | §6 无 `'\0'` 越界读 |
| 段错误、输出疑似缺失 | §7 指针/未初始化 / stdout 缓冲 |
| 本地正常、跨网络乱 | §5 一次 recv 收不满 / 粘包 |

---

> 版本说明:本文修正均保持"只改必要处、不动结构体与业务逻辑"的原则。
> 相关注释已标注在 `models/transport.c`、`test/client.c`、`test/server.c` 源码对应位置。
