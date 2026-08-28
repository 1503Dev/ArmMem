//
// Created by TheChuan1503 on 2026/3/21.
//

#include "jni.h"
#include "global.h"
#include <atomic>

jclass g_jclass_HookOnInvokeListener = nullptr;
jmethodID g_jmethodID_HookOnInvokeListener_onInvoke = nullptr;
std::atomic<bool> g_isInitialized{false};

extern "C" {
JNIEXPORT void JNICALL
Java_dev1503_armmem_ArmMem_init(JNIEnv *env, jclass clazz) {
    if (g_isInitialized.load(std::memory_order_acquire)) return;
    jclass cls = env->FindClass("dev1503/armmem/hook/HookOnInvokeListener");
    if (cls == nullptr) return;
    g_jclass_HookOnInvokeListener = (jclass)env->NewGlobalRef(cls);
    g_jmethodID_HookOnInvokeListener_onInvoke = env->GetMethodID(g_jclass_HookOnInvokeListener, "onInvoke",
                                                                  "()V");
    g_isInitialized.store(true, std::memory_order_release);
}
}