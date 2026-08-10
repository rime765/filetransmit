#ifndef __PLATFORM_H
#define __PLATFORM_H
    #include <minwindef.h>
#ifdef _WIN32
         #ifndef _WIN32_WINNT
            #define _WIN32_WINNT 0x0601
        #endif
        #include<WinSock2.h>
        #include <Windows.h>

        typedef  int socket_len ;
        #define  T_SOCKEET  SOCKET
        #define  T_SOCKET_INVALID   INVALID_SOCKET
        #define  T_SOCKET_ERROR     SOCKET_ERROR 
        #define  T_SOCKET_SUCCESS   0

        #define  T_CloseSocket(socket_id)   close(socket_id)
        #define  T_Error()                  WSAGETLASTERROR();

        static inline int t_init()
        {
            WSADATA wsa_data;
            return WSAStartup(/*拼版本号*/MAKEWORD(2, 2), &wsa_data);
        }

        static inline void t_exit()
        {
            WSACleanup();
        }

    #else
        

    #endif




#endif