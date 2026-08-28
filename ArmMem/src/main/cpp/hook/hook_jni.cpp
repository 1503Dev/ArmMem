//
// Created by TheChuan1503 on 2026/3/18.
//

#include "jni.h"
#include "../global.h"
#include "../exports/armmem/hook.h"



extern "C" {
JNIEXPORT void JNICALL
Java_dev1503_armmem_hook_JNI_hook(JNIEnv *env, jclass clazz, jlong address, jobject listener) {
    if (throwNotInitializedException(env)) return;
    auto ptr = (void*)address;
//    ArmMemHook::hook(ptr, nullptr, nullptr);
}

JNIEXPORT jlong JNICALL
Java_dev1503_armmem_hook_JNI_getFunctionAddress(JNIEnv *env, jclass clazz, jstring moduleName, jstring functionName) {
    const char* module = env->GetStringUTFChars(moduleName, nullptr);
    const char* function = env->GetStringUTFChars(functionName, nullptr);
    auto result = (jlong)ArmMemHook::getSymbol(const_cast<char*>(module), const_cast<char*>(function));
    env->ReleaseStringUTFChars(moduleName, module);
    env->ReleaseStringUTFChars(functionName, function);
    return result;
}
}