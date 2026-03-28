//
// Created by TheChuan1503 on 2026/3/15.
//

#ifndef ARMMEM_MEMORY_SEARCH_RESULT
#define ARMMEM_MEMORY_SEARCH_RESULT

#include <cstring>
#include <cstdint>
#include <vector>
#include <sys/uio.h>
#include "memory_global.h"

struct MemoryValue {
    uintptr_t address;
    ValueType type;
    union {
        int dwordValue; // 4
        float floatValue; // 4
        long long qwordValue; // 8
        double doubleValue; // 8
        signed char byteValue; // 1
        short wordValue; // 2
    } value;
};


#endif