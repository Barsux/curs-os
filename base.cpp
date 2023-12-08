#include "base.h"

/* Фукнция переводит строку с IP-адресом в 32бита */
int str2ip4 (const char * dst, IP4 ip)
{
    unsigned int byte0, byte1, byte2, byte3;
    char fakeString[2];
    if (sscanf(dst, "%u.%u.%u.%u%1s", &byte3, &byte2, &byte1, &byte0, fakeString) == 4)
    {
        if ((byte3 < 256) && (byte2 < 256) && (byte1 < 256) && (byte0 < 256))
        {
            ip  = (byte3 << 24) + (byte2 << 16) + (byte1 << 8) + byte0;
            return 1;
        }
    }return 0;
}

/* Функция для 1-го сервера, принимает массив из двух целых. Заполняет его координатами курсора, X, Y. Возвращает: 1 удача, -1 - неудача */
/* Незапирающая */
int getCursorPosition(int * coordinates) {
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);
    coordinates[0] = -1;
    coordinates[1] = -1;
    Window child;
    int rootX, rootY, winX, winY;
    unsigned int mask;
    if (XQueryPointer(display, root, &root, &child, &rootX, &rootY, &winX, &winY, &mask)) {
        XCloseDisplay(display);
        coordinates[0] = rootX;
        coordinates[1] = rootY;
        return 1;
    } else {
        return -1;
    }
    return -1;
}


/* Функция для 1-го сервера, возвращает последнюю ошибку. 1 удача, -1 - неудача */
/* Незапирающая */
int getLastError(int &error){
    error = errno;
    return 1;
}