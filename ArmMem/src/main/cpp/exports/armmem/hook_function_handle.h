//
// Created by TheChuan1503 on 2026/3/15.
//

#ifndef ARMMEM_HOOK_FUNCTION_HANDLE_H
#define ARMMEM_HOOK_FUNCTION_HANDLE_H

#include <cstring>
#include <cstdint>

struct HookFunctionHandle {
    bool isSuccess = false;
    char* message = nullptr;
    uintptr_t target;
    uint32_t backupInsns[8]{};
    uint32_t backupSize;
    uintptr_t trampoline;
    bool isActive;
    uint8_t padding[7]{};

    HookFunctionHandle() : target(0), backupSize(0), trampoline(0), isActive(false) {
        memset(backupInsns, 0, sizeof(backupInsns));
        memset(padding, 0, sizeof(padding));
    }
};

static HookFunctionHandle g_hook_pool[256];
static uint32_t g_hook_count = 0;

#endif //ARMMEM_HOOK_FUNCTION_HANDLE_H