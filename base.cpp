#include "base.h"
#include <cstdio>


/* Функция для 1-го сервера, принимает массив из двух целых. Заполняет его координатами курсора, X, Y. Возвращает: 1 удача, -1 - неудача */
/* Незапирающая */


/* Функция для 1-го сервера, возвращает последнюю ошибку. 1 удача, -1 - неудача */
/* Незапирающая */
int getLastError(U8 * str){
    return snprintf((char*)str, DATA_SIZE, "Последняя ошибка: %d:%s", errno, strerror(errno));
}


int getMemoryUsagePercentage(U8 * str) {
    FILE* file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        printf("Ошибка открытия файла /proc/meminfo\n");
        return -1;
    }

    char line[128];
    char attr[16];
    long total_memory = 0;
    long free_memory = 0;
    long buffers = 0;
    long cached = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %ld", attr, &total_memory);
        if (strcmp(attr, "MemTotal:") == 0) {
            break;
        }
    }

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %ld", attr, &free_memory);
        if (strcmp(attr, "MemFree:") == 0) {
            break;
        }
    }

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %ld", attr, &buffers);
        if (strcmp(attr, "Buffers:") == 0) {
            break;
        }
    }

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %ld", attr, &cached);
        if (strcmp(attr, "Cached:") == 0) {
            break;
        }
    }

    fclose(file);

    long used_memory = total_memory - free_memory - buffers - cached;
    double usage_percentage = (double)used_memory * 100 / total_memory;
    snprintf((char*)str, DATA_SIZE, "Физическая память: %f %%", usage_percentage);
    return usage_percentage;
}

int getSwapMemoryUsagePercentage(U8 * str) {
    FILE* file = fopen("/proc/swaps", "r");
    if (file == NULL) {
        printf("Ошибка открытия файла /proc/swaps\n");
        return -1;
    }

    char line[128];
    int total_swap = 0;
    int used_swap = 0;

    // Пропускаем заголовок файла
    fgets(line, sizeof(line), file);
    // Считываем информацию о SWAP
    fgets(line, sizeof(line), file);
    //printf("%s\n", line);
    char loc[64];
    char name[64];
    int priority;
    sscanf(line, "%s %s %d %d %d", loc, name, &total_swap, &used_swap, &priority);
    fclose(file);
    used_swap *= 100;
    double usage_percentage = (double)used_swap / total_swap;
    return snprintf((char*)str, DATA_SIZE, "Виртуальная память: %f %%", usage_percentage);
}


void unpack_utc(TIME_INFO &dst, U64 utc) {
  struct tm tm = {}; dst.ns = utc%1000000000ULL; time_t t = utc/1000000000ULL; gmtime_r(&t, &tm);
  dst.YYYY = 1900+tm.tm_year; dst.MM = 1+tm.tm_mon; dst.DD = tm.tm_mday;
  dst.hh = tm.tm_hour; dst.mm = tm.tm_min; dst.ss = tm.tm_sec; dst.w = tm.tm_wday;
}


int utc2str(char* dst, int cbDstMax, U64 utc) {
  TIME_INFO ti; unpack_utc(ti, utc);
  return snprintf(dst, cbDstMax, "%04u%02u%02u-%02u:%02u:%02u.%06u"
    , ti.YYYY, ti.MM, ti.DD, ti.hh, ti.mm, ti.ss, ti.ns/1000
  );
}


U64 nanotime() {
  #ifdef __linux__
  timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
  return U64(ts.tv_sec)*1000000000ULL + U64(ts.tv_nsec);
  #else
  U64 time[2]; GetSystemTimeAsFileTime((FILETIME*)(&time[0]));
  return U64((time[0])*100ULL - 11644473600000000000ULL);
  #endif
}


void stdout_write(void* src, int cbSrc) {
  write(1, src, cbSrc);
  fflush(stdout); 
}


void stdout_printf(char* fmt, ...) {
  char buff[2048];
  va_list va; va_start(va, fmt);
  int n = vsnprintf(buff, (int)sizeof(buff)-2, fmt, va); buff[sizeof(buff)-1]=0;
  va_end(va);
  stdout_write(buff, n);
}


void print(char* fmt, ...) {
  char t[128]; utc2str(t, sizeof(t), nanotime());
  stdout_printf("[%s] ", t);
  va_list va; va_start(va, fmt); char buff[2048];
  int n = vsnprintf(buff, (int)sizeof(buff)-2, fmt, va);
  buff[sizeof(buff)-3]=0;
  if (n>=sizeof(buff)) n = sizeof(buff)-2;
  buff[n++] = '\r'; buff[n++] = '\n'; stdout_write(buff, n);
  va_end(va);
}
