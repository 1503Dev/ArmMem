//
// Created by TheChuan1503 on 2026/3/21.
//

#ifndef ARMMEM_GLOBAL_H
#define ARMMEM_GLOBAL_H

#include <jni.h>
#include <android/log.h>

extern jclass g_jclass_HookOnInvokeListener;
extern jmethodID g_jmethodID_HookOnInvokeListener_onInvoke;
extern bool g_isInitialized;

static bool throwNotInitializedException(JNIEnv *env){
    if (g_isInitialized){
        return false;
    }
    __android_log_print(ANDROID_LOG_ERROR, "ArmMem", "ArmMem is not initialized");
    env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "ArmMem is not initialized");
    return true;
}

#endif //ARMMEM_GLOBAL_H
