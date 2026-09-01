#ifndef __PLATFORM_H
#define __PLATFORM_H
//    #include <minwindef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>


    #ifdef _WIN32
         #ifndef _WIN32_WINNT
            #define _WIN32_WINNT 0x0601
        #endif
        #include<WinSock2.h>
        #include <Windows.h>
        #include <ws2tcpip.h>

        typedef  int socket_len ;
        #define  T_SOCKEET  SOCKET
        #define  T_SOCKET_INVALID   INVALID_SOCKET
        #define  T_SOCKET_ERROR     SOCKET_ERROR 
        #define  T_SOCKET_SUCCESS   (!INVALID_SOCKET&&!SOCKET_ERROR)

        #define  T_CloseSocket(socket_id)   closesocket(socket_id)
        #define  T_Error()                  WSAGETLASTERROR();
        static inline void  T_ERROR_Print(char* str){ fprintf(stderr,"%s",str);}

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
        #include<sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
        #include <sys/errno.h>
        #include <unistd.h>


                typedef  int socket_len ;
        #define  T_SOCKEET  int
        #define  T_SOCKET_INVALID   -1
        #define  T_SOCKET_ERROR     -1 
        #define  T_SOCKET_SUCCESS   0

        #define  T_CloseSocket(socket_id)   close(socket_id)
        #define  T_Error()                  
        static inline void T_ERRO_Print(char* str){
            perror(str);
        }
        static inline int t_init()
        {
        return 0;  
        }

        static inline void t_exit()
        { 
        return;
        }
        static inline void  T_ERROR_Print(char* str){ perror(str);}
    #endif

#endif