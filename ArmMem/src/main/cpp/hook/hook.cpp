//
// Created by TheChuan1503 on 2026/3/15.
//

#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include "../exports/armmem/hook_function_64.h"
#include "../exports/armmem/hook_function_32.h"
#include "../exports/armmem/memory.h"
#include "../exports/armmem/hook.h"
#include "hook_function_global.h"

void* ArmMemHook::openModule(char* moduleName){
    void* handle = dlopen(moduleName, RTLD_NOLOAD | RTLD_LAZY);
    return handle;
}
void* ArmMemHook::getSymbol(char* moduleName, char* symbol) {
    void* handle = openModule(moduleName);
    if (!handle) return nullptr;
    return dlsym(handle, symbol);
}

HookFunctionHandle* ArmMemHook::hook(void *target, void *hook, void **originalPtr) {
#ifdef __aarch64__
    return ArmMemHookFunction64::hook(target, hook, originalPtr);
#endif
    return ArmMemHookFunction32::hook(target, hook, originalPtr);
}
HookFunctionHandle* ArmMemHook::hook(char* moduleName, char* symbol, void *hook, void **originalPtr) {
    void* target = getSymbol(moduleName, symbol);
    if (!target) {
        auto* handle = new HookFunctionHandle();
        handle->isSuccess = false;
        handle->message = ArmMem_HookFunction_MSG_INVALID_MODULE_OR_SYMBOL_NAME;
        return handle;
    }
    return ArmMemHook::hook(target, hook, originalPtr);
}
void *ArmMemHook::hookV(void *target, void *hook, void *rwx, uintptr_t rwxSize) {
#ifdef __aarch64__
    return ArmMemHookFunction64::hookV(target, hook, rwx, rwxSize);
#endif
    return ArmMemHookFunction32::hookV(target, hook, rwx, rwxSize);
}
bool ArmMemHook::unhook(HookFunctionHandle *handle) {
#ifdef __aarch64__
    return ArmMemHookFunction64::unhook(handle);
#endif
    return ArmMemHookFunction32::unhook(handle);
}
