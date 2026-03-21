//
// Created by TheChuan1503 on 2026/3/15.
//

#ifndef ARMMEM_MEMORY_REGION_H
#define ARMMEM_MEMORY_REGION_H

#include <cstdint>

struct MemoryRegion {
    uintptr_t start;
    size_t size;
    char path[256];
};

#endif //ARMMEM_MEMORY_REGION_H
