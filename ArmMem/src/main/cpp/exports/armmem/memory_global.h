//
// Created by TheChuan1503 on 2026/3/15.
//

#ifndef ARMMEM_MEMORY_GLOBAL_H
#define ARMMEM_MEMORY_GLOBAL_H

enum class ValueType { DWORD, FLOAT, DOUBLE, QWORD, BYTE, WORD };
enum class MemoryRange {
    C_HEAP, JAVA_HEAP, C_ALLOC, C_DATA,
    C_BSS, ANONYMOUS, CODE_APP, STACK, ASHMEM, OTHER
};

#endif //ARMMEM_MEMORY_GLOBAL_H
