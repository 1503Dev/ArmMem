//
// Created by TheChuan1503 on 2026/4/4.
//

#ifndef ARMMEM_MEMORY_MONITOR_HANDLE_H
#define ARMMEM_MEMORY_MONITOR_HANDLE_H

#include <cstdint>
#include "memory_monitor_hit.h"

struct MemoryMonitorHandle {
    int type;
    bool isOnce;
    int pid;
    uintptr_t address;
    size_t size;
    void* userData;
    void* (*callback)(MemoryMonitorHandle* handle, MemoryMonitorHit* hit);
    int hash;
};

#endif //ARMMEM_MEMORY_MONITOR_HANDLE_H
