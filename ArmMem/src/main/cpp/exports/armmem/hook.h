//
// Created by TheChuan1503 on 2026/3/15.
//

#ifndef ARMMEM_HOOK_H
#define ARMMEM_HOOK_H

#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "hook_function_handle.h"

class ArmMemHook {
public:
    static void* openModule(char* moduleName);
    static void* getSymbol(char* moduleName, char* symbol);
    static HookFunctionHandle* hook(void *target, void *hook, void **originalPtr);
    static HookFunctionHandle* hook(char* moduleName, char* symbol, void *hook, void **originalPtr);
    static void *hookV(void *target, void *hook, void *rwx, uintptr_t rwxSize);
    static bool unhook(HookFunctionHandle* handle);
};

#endif