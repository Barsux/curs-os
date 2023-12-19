#ifndef BASEH

    #define BASEH

    #include <unistd.h>
    #include <stdio.h>
    #include <inttypes.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <cstdio>
    #include <cstdlib>
    #include <stdarg.h>
    #include <stdlib.h>
    #include <time.h>
    #include <string.h>
    #include <signal.h>
    #include <bits/pthreadtypes.h>
    #include <pthread.h>

    #ifdef __linux__
        #ifdef __arm__
        #else
            
        #endif
    #endif

    #define PORT1 3387
    #define PORT2 3388
    #define MAX_LISTENERS 16
    #define DEBUG_LISTENERS 1
    #define IP "127.0.0.1"
    #define BUFF_SIZE 1500
    #define DATA_SIZE 128

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

    enum data_flags{
        RECV_FIRST_DATA = 0x01,
        RECV_SECOND_DATA = 0x02,
        RECV_DATA = 0x03,
    };

    struct packet {
        U64 timestamp;
        bool is_request;
        data_flags flag;
        U8 data_first[DATA_SIZE];
        U8 data_second[DATA_SIZE];   
    };

    struct TIME_INFO {
        U32 YYYY, MM, DD, w, hh, mm, ss, ns;
        TIME_INFO(): YYYY(), MM(), DD(), hh(), mm(),ss(), ns(), w() {}
    };

    int getCursorPosition(U8 * str);
    int getLastError(U8 * str);
    void print(char* fmt, ...);
    U64 nanotime();
    int utc2str(char* dst, int cbDstMax, U64 utc);
    int getMemoryUsagePercentage(U8 * str);
    int getSwapMemoryUsagePercentage(U8 * str);
#endif