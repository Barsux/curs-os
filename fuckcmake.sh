#!/bin/bash
g++ -Wall -Wfatal-errors -ffunction-sections -fdata-sections -Wl,--gc-sections -Wwrite-strings -Wreorder -pthread -fpermissive -fno-rtti -fno-exceptions -Wreturn-type -L/usr/X11/lib -lX11 client.cpp base.cpp -o client.r
g++ -Wall -Wfatal-errors -ffunction-sections -fdata-sections -Wl,--gc-sections -Wwrite-strings -Wreorder -pthread -fpermissive -fno-rtti -fno-exceptions -Wreturn-type -L/usr/X11/lib -lX11 server1.cpp base.cpp -o server1.r
g++ -Wall -Wfatal-errors -ffunction-sections -fdata-sections -Wl,--gc-sections -Wwrite-strings -Wreorder -pthread -fpermissive -fno-rtti -fno-exceptions -Wreturn-type -L/usr/X11/lib -lX11 server2.cpp base.cpp -o server2.r