#include "base.h"


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