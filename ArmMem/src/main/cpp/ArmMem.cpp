//
// Created by TheChuan1503 on 2026/4/4.
//

#include "exports/ArmMem.h"
#include <cstdio>
#include <cstdarg>
#include <vector>

void (*ArmMem::loggerV)(const char* msg) = nullptr;
void (*ArmMem::loggerE)(const char* msg) = nullptr;

void ArmMem::formatAndLog(void (*logger)(const char*), const char* clazz, const char* method, const char* format, va_list args) {
    if (!logger) return;

    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (len < 0) return;

    std::vector<char> contentBuf(len + 1);
    vsnprintf(contentBuf.data(), contentBuf.size(), format, args);
    std::string finalMessage = "[" + std::string(clazz) + "] <" + std::string(method) + "> " + contentBuf.data();

    logger(finalMessage.c_str());
}

void ArmMem::logV(const char* clazz, const char* method, const char* format, ...) {
    if (!ARMMEM_DEBUG || !loggerV) return;
    va_list args;
    va_start(args, format);
    formatAndLog(loggerV, clazz, method, format, args);
    va_end(args);
}

void ArmMem::logE(const char* clazz, const char* method, const char* format, ...) {
    if (!ARMMEM_DEBUG || !loggerE) return;
    va_list args;
    va_start(args, format);
    formatAndLog(loggerE, clazz, method, format, args);
    va_end(args);
}

void ArmMem::setLoggerV(void (*logger)(const char*)) {
    loggerV = logger;
}

void ArmMem::setLoggerE(void (*logger)(const char*)) {
    loggerE = logger;
}