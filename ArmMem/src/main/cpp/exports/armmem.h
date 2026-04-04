//
// Created by TheChuan1503 on 2026/4/4.
//

#ifndef ARMMEM_ARMMEM_H
#define ARMMEM_ARMMEM_H

#include <cstdarg>

#define ARMMEM_DEBUG 0

class ArmMem {
public:
    static void (*loggerV)(const char* msg);
    static void (*loggerE)(const char* msg);

    static void logV(const char* clazz, const char* method, const char* format, ...);
    static void logE(const char* clazz, const char* method, const char* format, ...);

    static void setLoggerV(void (*logger)(const char*));
    static void setLoggerE(void (*logger)(const char*));

private:
    static void formatAndLog(void (*logger)(const char*), const char* clazz, const char* method, const char* format, va_list args);
};


#endif //ARMMEM_ARMMEM_H
