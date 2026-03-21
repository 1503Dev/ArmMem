//
// Created by TheChuan1503 on 2026/3/15.
//

#ifndef ARMMEM_HOOK_FUNCTION_64_H
#define ARMMEM_HOOK_FUNCTION_64_H

#include "hook_function_handle.h"

#define ArmMem_HF64_MAX_BACKUPS (256)

class ArmMemHookFunction64 {
public:
    static HookFunctionHandle* hook(void *target, void *hook, void **originalPtr);
    static void* hookV(void *target, void *hook, void *rwx, uintptr_t rwxSize);
    static bool unhook(HookFunctionHandle* handle);

private:
    static void fixInstructions(uint32_t *inp, int32_t count, uint32_t *outp);
    static uint32_t* allocateTrampoline();
    static void init();
};

#endif //ARMMEM_HOOK_FUNCTION_64_H
