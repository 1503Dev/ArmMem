//
// Created by TheChuan1503 on 2026/3/15.
//

#include "../exports/armmem/hook_function_32.h"
#include "../exports/armmem/hook_function_handle.h"
#include "hook_function_global.h"
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <android/log.h>


#define ARMMEM_HF32_MAX_BACKUPS 128
#define ARMMEM_HF32_TRAMPOLINE_SIZE 64

#define _flush_cache(addr, size) __builtin___clear_cache(reinterpret_cast<char*>(addr), reinterpret_cast<char*>(addr) + size)

static uint32_t gInsnsPoolA32[ARMMEM_HF32_MAX_BACKUPS][ARMMEM_HF32_TRAMPOLINE_SIZE] __attribute__((__aligned__(4096)));
static uintptr_t gPageSize = 0;

static void initPageSize() {
    if (gPageSize == 0) gPageSize = sysconf(_SC_PAGESIZE);
}

static inline bool isThumbMode(uintptr_t addr) { return (addr & 1) != 0; }
static inline uintptr_t getRealAddress(uintptr_t addr) { return addr & ~1; }

static int getInstructionSize(uintptr_t addr, bool isThumb) {
    if (isThumb) {
        uint16_t ins = *reinterpret_cast<uint16_t*>(addr);
        if ((ins & 0xF800) == 0xF000 || (ins & 0xFF00) == 0xE800) return 4;
        return 2;
    }
    return 4;
}

static bool makeMemoryRwx(void* addr, size_t size) {
    initPageSize();
    uintptr_t start = reinterpret_cast<uintptr_t>(addr) & ~(gPageSize - 1);
    uintptr_t end = (reinterpret_cast<uintptr_t>(addr) + size + gPageSize - 1) & ~(gPageSize - 1);
    return mprotect(reinterpret_cast<void*>(start), end - start, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

void ArmMemHookFunction32::init() {
    initPageSize();
    makeMemoryRwx(gInsnsPoolA32, sizeof(gInsnsPoolA32));
}

uint32_t* ArmMemHookFunction32::allocateTrampoline() {
    static volatile int32_t index = -1;
    int32_t i = __sync_add_and_fetch(&index, 1);
    if (i >= 0 && i < ARMMEM_HF32_MAX_BACKUPS) {
        return gInsnsPoolA32[i];
    }
    return nullptr;
}

void ArmMemHookFunction32::fixInstructions(uintptr_t realTarget, int totalBytes, uint32_t* trampoline, bool isThumb) {
    if (isThumb) {
        auto* thumbTramp = reinterpret_cast<uint16_t*>(trampoline);
        auto* src = reinterpret_cast<uint16_t*>(realTarget);

        for (int i = 0; i < totalBytes / 2; i++) {
            thumbTramp[i] = src[i];
        }

        int idx = totalBytes / 2;

        if ((reinterpret_cast<uintptr_t>(&thumbTramp[idx]) % 4) != 0) {
            thumbTramp[idx++] = 0xBF00;
        }

        uintptr_t returnAddr = realTarget + totalBytes;
        returnAddr |= 1;

        thumbTramp[idx] = 0xF8DF;     // LDR.W PC, [PC, #0]
        thumbTramp[idx + 1] = 0xF000;
        *reinterpret_cast<uintptr_t*>(&thumbTramp[idx + 2]) = returnAddr;
    } else {
        uint32_t* armTramp = trampoline;
        auto* src = reinterpret_cast<uint32_t*>(realTarget);
        int insCount = totalBytes / 4;

        for (int i = 0; i < insCount; i++) {
            armTramp[i] = src[i];
        }

        uintptr_t returnAddr = realTarget + totalBytes;
        armTramp[insCount] = 0xE51FF004; // LDR PC, [PC, #-4]
        armTramp[insCount + 1] = returnAddr;
    }

    _flush_cache(trampoline, ARMMEM_HF32_TRAMPOLINE_SIZE * 4);
}

void* ArmMemHookFunction32::hookV(void* const symbol, void* const replace, void* const rwx, const uintptr_t rwxSize) {
    auto targetAddr = reinterpret_cast<uintptr_t>(symbol);
    auto hookAddr = reinterpret_cast<uintptr_t>(replace);
    bool isThumb = isThumbMode(targetAddr);
    uintptr_t realTarget = getRealAddress(targetAddr);

    int minBytes = isThumb ? 8 : 8;
    int totalBytes = 0;
    while (totalBytes < minBytes) {
        totalBytes += getInstructionSize(realTarget + totalBytes, isThumb);
    }

    if (rwx) {
        fixInstructions(realTarget, totalBytes, static_cast<uint32_t*>(rwx), isThumb);
    }

    if (!makeMemoryRwx(reinterpret_cast<void*>(realTarget), totalBytes)) {
        return nullptr;
    }

    if (isThumb) {
        auto* code = reinterpret_cast<uint16_t*>(realTarget);
        code[0] = 0xF8DF; code[1] = 0xF000; // LDR.W PC, [PC]
        *reinterpret_cast<uint32_t*>(&code[2]) = (hookAddr | 1);
        for(int i = 4; i < totalBytes / 2; i++) code[i] = 0xBF00;
    } else {
        auto* code = reinterpret_cast<uint32_t*>(realTarget);
        code[0] = 0xE51FF004; // LDR PC, [PC, #-4]
        code[1] = hookAddr;
        for(int i = 2; i < totalBytes / 4; i++) code[i] = 0xE1A00000; // NOP (MOV R0, R0)
    }

    __builtin___clear_cache(reinterpret_cast<char*>(realTarget), reinterpret_cast<char*>(realTarget) + totalBytes);

    if (rwx) {
        auto resultPtr = reinterpret_cast<uintptr_t>(rwx);
        if (isThumb) resultPtr |= 1;
        return reinterpret_cast<void*>(resultPtr);
    }
    return (void*)1;
}

HookFunctionHandle* ArmMemHookFunction32::hook(void* target, void* hookFunc, void** originalPtr) {
    auto* handle = new HookFunctionHandle();
    handle->isSuccess = false;

    if (!target || !hookFunc) {
        handle->message = strdup(ArmMem_HookFunction_MSG_INVALID_TARGET);
        return handle;
    }

    HookFunctionHandle* poolHandle = nullptr;
    for (auto & i : g_hook_pool) {
        if (!i.isActive) { poolHandle = &i; break; }
    }

    if (!poolHandle) {
        handle->message = strdup(ArmMem_HookFunction_MSG_HOOK_POOL_FULL);
        return handle;
    }
    delete handle;
    handle = poolHandle;

    bool isThumb = isThumbMode(reinterpret_cast<uintptr_t>(target));
    uintptr_t realTarget = getRealAddress(reinterpret_cast<uintptr_t>(target));

    int minBytes = isThumb ? 8 : 8;
    int totalBytes = 0;
    while (totalBytes < minBytes) {
        totalBytes += getInstructionSize(realTarget + totalBytes, isThumb);
    }

    handle->target = reinterpret_cast<uintptr_t>(target);
    handle->backupSize = totalBytes;
    memcpy(handle->backupInsns, reinterpret_cast<void*>(realTarget), totalBytes);

    void* trampoline = nullptr;
    if (originalPtr != nullptr) {
        trampoline = allocateTrampoline();
        *originalPtr = trampoline;
        handle->trampoline = reinterpret_cast<uintptr_t>(trampoline);
        if (!trampoline) {
            handle->isActive = false;
            handle->message = strdup(ArmMem_HookFunction_MSG_ALLOCATE_TRAMPOLINE_FAILED);
            return handle;
        }
    }

    void* result = hookV(target, hookFunc, trampoline, ARMMEM_HF32_TRAMPOLINE_SIZE);
    if (result != nullptr || originalPtr == nullptr) {
        handle->isActive = true;
        handle->isSuccess = true;
        handle->message = strdup(ArmMem_HookFunction_MSG_HOOK_SUCCESS);
        return handle;
    }

    handle->isActive = false;
    handle->message = strdup(ArmMem_HookFunction_MSG_HOOK_FAILED);
    return handle;
}

bool ArmMemHookFunction32::unhook(HookFunctionHandle* handle) {
    if (!handle || !handle->isActive || !handle->target) return false;
    uintptr_t realTarget = getRealAddress(handle->target);
    size_t patchSize = handle->backupSize;
    if (makeMemoryRwx(reinterpret_cast<void*>(realTarget), patchSize)) {
        memcpy(reinterpret_cast<void*>(realTarget), handle->backupInsns, patchSize);
        _flush_cache(realTarget, patchSize);
        handle->isActive = false;
        return true;
    }
    return false;
}