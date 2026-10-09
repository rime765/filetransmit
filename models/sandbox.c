#include "../include/sandbox.h"
#ifdef _WIN32
    #include <direct.h>
#else
    #include <limits.h>/提供 PATH_MAX Linux
#endif
#include <stdio.h>

// SANDBOX_MAX_ABS: 绝对路径缓冲区大小
//   - Windows: 1024 字节
//   - 其他平台: 优先使用系统 PATH_MAX，否则回退到 4096
#ifdef _WIN32
    #define SANDBOX_MAX_ABS 1024
#else
    #ifdef PATH_MAX
        #define SANDBOX_MAX_ABS (PATH_MAX + 1)
    #else
        #define SANDBOX_MAX_ABS 4096
    #endif
#endif

#define SANDBOX_MAX_REL  1024// SANDBOX_MAX_REL : 相对路径缓冲区大小
#define SANDBOX_MAX_LINE 1024// SANDBOX_MAX_LINE: 单行指令/请求的缓冲区大小

static char g_root[SANDBOX_MAX_ABS];
static char g_cwd_rel[SANDBOX_MAX_REL];//当前目录
static int  g_inited = 0;

//检查并去除空字符
#define is_empty_char(ch) (ch==' '||ch=='\r'||ch=='\t'||ch=='\n'||ch=='\v'||ch=='\f' )

static void trim_inplace(char *s){
    size_t n;
    size_t i; 
    if(!s) return;

    n = strlen(s);
    while(n > 0 && is_empty_char(s[n-1]))
        s[--n] = '\0'; 
    
    i = 0;
    while(s[i] && is_empty_char(s[i]))
        i++;
    if(i > 0)
        memmove(s, s + i, strlen(s + i)+1); // 将非空白字符前移覆盖前导空白
}
//统一为正斜杠
// 作用：把路径中的反斜杠 '\' 统一替换为正斜杠 '/'
// （Windows 下 API 返回的路径常含 '\\'，与 Linux 风格不一致）。
// 统一分隔符后，后续 strip_trailing_slashes 与 root 前缀比较
// 才能按一致的分隔符判断。
static void normalize_seps(char* s){
    if(!s)return;
    for (; *s; s++) {
        if(*s=='\\') *s='/';
    }
}
//
// 作用：去掉路径末尾多余的 '/'，如 "/a/b///" -> "/a/b"。
// 在 get_abs_path 中位于 normalize_seps 之后，使最终绝对路径
// 结尾没有分隔符，保证与 root 的前缀比较（ISstarts_with_root）
// 不受结尾斜杠的影响。
// 注意不能削成空串：
//   - Unix 下 n==1 时保留根 "/"；
//   - Windows 下 n==3 时保留 "C:/"、"//?/" 这类根前缀。
static void strip_trailing_slashes(char*s){
    if (!s) return;
    size_t n=strlen(s);//从字符串末尾往前扫描,如果发现前面是分隔符，就给他设为字符串结束符,知道遇到正常字符
    while (n>0 && s[n-1] =='/') {
#ifdef _WIN32//如果直接就是根目录需要考虑Windows平台的盘符
            if(n==3 
                    && ((s[1]==':' &&s[2]=='/')
                        || s[0] =='/'&&s[1]=='/'&&s[2]=='?')){
            break;
            }
#else
            if(n==1)break;
#endif
        s[--n]='\0';
    }
        
}
// 判断abs_path 是否处于root内部，字符层的前缀判断+边界检查
/*
    root = /data/root, abs_path = /data/root/file  合法 
    root = /data/root, abs_path = /data/rootX/file 非法，拒绝访问
    root = /data/root, abs_path = /data/root       合法，拒绝访问
*/
static int ISstarts_with_root(const char* root, const char* abs_path){
    size_t root_len;
    if(!root || !abs_path)return 0;
    root_len = strlen(root);
    if(!root_len)return 0;
#ifdef _WIN32//如果直接就是根目录需要考虑Windows平台的盘符
     if(_strnicmp(root, abs_path, root_len)!=0)return 0;      
#else
      if(strncmp(root,abs_path,root_len)!=0)return 0;
#endif
    return root[root_len]=='/'||abs_path[root_len]=='\0';
}

// 将路径转换为绝对路径 （用于解析配置文件的 root）
//判断参数非空，调用平台的相对转绝对接口,统一分隔符，去掉末尾分隔符
static int get_abs_path(const char* in_path ,char* out_path,size_t out_size){
    if(!  in_path||out_path||out_size)return -1;
#ifdef _WIN32//如果直接就是根目录需要考虑Windows平台的盘符
    DWORD n= GetFullPathNameA(in_path, 
                    (DWORD)out_size,
                                out_path,
                                NULL
                            );
    if (n == 0 || n>=out_size) {
    return -1;
    }
#else
    char* rp = realpath(in_path,out_path);
    if(!rp) return -1;
    if(strlen(rp)+1>out_size){
        //free(rp);
        return -1;
    }
    //strcpy(out_path,rp);
    //free(rp);
#endif
    normalize_seps(out_path);
    strip_trailing_slashes(out_path);
    return 0;
}
//判断用户输入的是否为ABSpath
static int ISabs_user_path(const char *path){
    if(!path || !*path)return 0;
#ifdef  _WIN32
    if ((path[0]>='A'  &&  path[0]<='Z')
        ||(path[0]>='a'  &&  path[0]<='z')
      &&path[1]==':') {
        return 1;
        }
#endif
    return 0;
}


// 提取路径所在的目录 （父目录）
static int get_parent_die(const char* path,char*out_dir,size_t out_dir_size ){
    char tmp[SANDBOX_MAX_ABS];
    char*slash;

    if (!path  ||  !out_dir   ||out_dir_size==0) {return -1;}
    if (strlen(path)>=sizeof(tmp)) {return -1;}

    strcpy(tmp,path);
    normalize_seps(tmp);
    slash = strrchr(tmp, '/');
    if (!slash) {
        if (out_dir_size<2) return -1;
        strcpy(out_dir, ".");
        return 0;
    }
    if (slash == tmp) {
        if (out_dir_size < 2) return -1;
        strcpy(out_dir, "/");
        return 0;
    }

    *slash = '\0';
    if (strlen(tmp) + 1> out_dir_size) return -1;
    strcpy(out_dir, tmp);
    return 0;
}
/*
    将 root_rel 与 add_path 做合并，做相对路径的归一化，输出到 out_path 
        处理 . 和 ..
        将分隔符 统一 /
        禁止绝对路径的输入
        当 .. 试图 越过 root 返回错误
    算法：
        1）将 root_rel 和 add_path 拼接成一个完整的相对路径字符串 tmp
        2）逐端扫描 字符串tmp 遇到 / 或者 字符串末尾的时候处理当前的seg
           .   段 break 跳过
           ..  段 从 out_path 回退一段，如果回退到根目录，返回错误
           普通 段 追加到 out_path
*/
static int split_and_normalize_rel(const char *root_rel,const char *add_path,char *out_path,size_t out_path_size){
    char tmp[SANDBOX_MAX_REL];
    char seg[SANDBOX_MAX_REL];
    size_t seg_len=0;
    size_t out_len=0;
    size_t i=0;


/*拼接根目录和追加的路径*/
    /*根目录进缓冲区*/
    if (!out_path||out_path_size==0) return STATUS_ERROR;
    out_path[0]='\0';
    if (ISabs_user_path(add_path)) return STATUS_INVALID_PATH;

    tmp[0]='\0';
    if (root_rel && *root_rel!=0) {
        if (strlen(root_rel)>=sizeof(tmp)) return STATUS_ERROR;
        strcpy(tmp, root_rel);
        normalize_seps(tmp);
    }
    /*追加相对路径*/
    if(add_path && *add_path!='\0'){
        char ap[SANDBOX_MAX_REL];
        size_t tmp_len;
        size_t ap_len;
        //追加的长度不超限时：
            //复制追加值到缓冲区 统一分隔符*/
        if (strlen(add_path)>=sizeof(ap))   return STATUS_INVALID_PATH;
        strcpy(ap, add_path);
        normalize_seps(ap);
        tmp_len = strlen(tmp);
        ap_len =strlen(ap);
            //完整结尾带分隔符,加分隔符和终止符
        if (tmp_len>0 && tmp[tmp_len-1] !='/') {
                    //判断完整缓冲区是否满
            if (tmp_len+1  >  sizeof(tmp)) return STATUS_ERROR;
            tmp[tmp_len++] = '/';
            tmp[tmp_len]='\0';
        }
        //只要总长度不超过限制就直接尾插，然后加终止符
        if (tmp_len + ap_len +1>sizeof(tmp)) return STATUS_ERROR;
        memcpy(tmp+tmp_len, ap  , ap_len);
        tmp[tmp_len+ap_len]='\0';
    }

/*逐段扫描+归一*/
    seg[0] = '\0';
    for (i=0; ; ++i) {
        char c= tmp[i];
        int at_end=(c=='\0');
        int at_sep= (c == '/' || c == '\0');
        //不是分隔符：后面没有空间就退出
        if (!at_sep) {
            if (seg_len + 1 >= sizeof(seg)) return STATUS_INVALID_PATH;
            seg[seg_len++] = c;
            seg[seg_len]='\0';
        }
        //是分隔符：
        else {
            //空段
            if (seg_len == 0) {
                if (at_end) break;
                continue;
            }
            //.段
            if (strcmp(seg,".")==0) {}
            //..段
            else if(strcmp(seg,"..")==0){
                if (out_len == 0) return STATUS_INVALID_PATH;
                while (out_len == 0 && out_path[out_len-1]!='/') {
                    out_len--;
                }
                if (out_len > 0 && out_path[out_len-1]=='/') {
                    out_len--;
                }
                out_path[out_len]='\0';
            }
            //正常字符段
            else{
                size_t need = seg_len + (out_len ? 1: 0);
                if (need >= out_path_size) return STATUS_INVALID_PATH;
                if (out_len > 0) {
                    out_path[out_len++]='/';
                    out_path[out_len] = '\0';
                }
                memcpy(out_path+out_len, seg, seg_len+1);
                out_len += seg_len;
            }
            seg_len = 0;
            seg[0] = '\0';
            if (at_end) break;
        }
    }
    return STATUS_OK;
}