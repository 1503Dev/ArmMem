//
// Created by TheChuan1503 on 2026/3/21.
//

#ifndef ARMMEM_GLOBAL_H
#define ARMMEM_GLOBAL_H

#include <jni.h>
#include <android/log.h>
#include <atomic>

extern jclass g_jclass_HookOnInvokeListener;
extern jmethodID g_jmethodID_HookOnInvokeListener_onInvoke;
extern std::atomic<bool> g_isInitialized;

static bool throwNotInitializedException(JNIEnv *env){
    if (g_isInitialized.load(std::memory_order_acquire)){
        return false;
    }
    __android_log_print(ANDROID_LOG_ERROR, "ArmMem", "ArmMem is not initialized");
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "ArmMem is not initialized");
    return true;
}

#endif //ARMMEM_GLOBAL_H
