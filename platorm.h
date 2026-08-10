#ifndef __PLATFORM_H
#define __PLATFORM_H
    #ifdef _WIN32
         #ifndef _WIN32_WINNT
            #define _WIN32_WINNT 0x0601
        #endif
        #include<WinSock2.h>
        #include <Windows.h>

        typedef  int socket_len ;
        #define  T_SOCKEET  SOCKET
        #define 



    #else

    #endif




#endif