//
// Created by TheChuan1503 on 2026/3/21.
//

#include "jni.h"
#include "global.h"

jclass g_jclass_HookOnInvokeListener = nullptr;
jmethodID g_jmethodID_HookOnInvokeListener_onInvoke = nullptr;
bool g_isInitialized = false;

extern "C" {
JNIEXPORT void JNICALL
Java_dev1503_armmem_ArmMem_init(JNIEnv *env, jclass clazz) {
    if (g_isInitialized) return;
    g_jclass_HookOnInvokeListener = env->FindClass("dev1503/armmem/hook/HookOnInvokeListener");
    g_jmethodID_HookOnInvokeListener_onInvoke = env->GetMethodID(g_jclass_HookOnInvokeListener, "onInvoke",
                                                                 "()V");
    g_isInitialized = true;
}
}