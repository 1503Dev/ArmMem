//
// Created by TheChuan1503 on 2026/3/15.
//

#ifndef ARMMEM_MEMORY_REGION_H
#define ARMMEM_MEMORY_REGION_H

#include <cstdint>

struct MemoryRegion {
//public:
    uintptr_t start;
    size_t size;
    char path[256];

//    MemoryRegion* subtract(MemoryRegion& other) const;
};

#endif //ARMMEM_MEMORY_REGION_H
