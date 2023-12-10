#ifndef BASEH

    #define BASEH

    #include <stdio.h>
    #include <inttypes.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <cstdio>
    #include <cstdlib>


    #ifdef __linux__
        #include "X11/Xlib.h"
    #endif

    #define PORT1 3387
    #define PORT2 3388
    #define MAX_LISTENERS 16
    #define DEBUG_LISTENERS 1
    #define IP "127.0.0.1"

    typedef uint64_t U64;
    typedef uint32_t U32;
    typedef uint16_t U16;
    typedef uint8_t U8;
    typedef unsigned short U4;
    typedef int64_t I64;
    typedef int32_t I32;
    typedef int16_t I16;
    typedef int8_t I8;
    typedef short I4;
    typedef U64 UTC;
    typedef U8 MAC[6];
    typedef U32 IP4;

    int getCursorPosition(int *pos);
    int getLastError(int &error);
#endif