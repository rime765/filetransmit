#ifndef DEBUGPRINT_H
#define DEBUGPRINT_H

#include <stdio.h>


/* 样式设置(ANSI 转义序列) */
#define DP_CLR_DEFAULT "\033[0m"
#define DP_CLR_RED     "\033[31m"
#define DP_CLR_GREEN   "\033[32m"
#define DP_CLR_YELLOW  "\033[33m"
#define DP_CLR_BLUE    "\033[34m"
#define DP_CLR_MAGENTA "\033[35m"
#define DP_CLR_CYAN    "\033[36m"

/* 样式开头 + 默认结束 */
#define DP_STYLE_OPEN  DP_CLR_CYAN
#define DP_STYLE_CLOSE DP_CLR_DEFAULT


/* 内联函数:把 msg 夹到样式设置中间,并加上 __FILE__/__LINE__ 前缀 */
static inline void debug_print(const char* file, int line, const char* msg){
    fprintf(stdout, "%s [%s:%d] %s%s\n",
            /*[样式开始]*/DP_CLR_CYAN,
            file, line,
            msg,
            /*[样式结束]*/DP_CLR_DEFAULT);
}

/* 宏:自动带上 __FILE__ / __LINE__ */
#define DEBUG_PRINT(msg) debug_print(__FILE__, __LINE__, (msg))


#endif
