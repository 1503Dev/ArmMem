//
// Created by TheChuan1503 on 2026/4/4.
//

#ifndef ARMMEM_MEMORY_MONITOR_HIT_H
#define ARMMEM_MEMORY_MONITOR_HIT_H

#include <cstdint>

struct MemoryMonitorHit {
    uint32_t originalValue;
    uintptr_t accessorAddress;
    uintptr_t accessorFunction;
    uintptr_t accessorModuleBase;
    const char* accessorSymbol;
    const char* accessorModuleName;
};

#endif //ARMMEM_MEMORY_MONITOR_HIT_H
