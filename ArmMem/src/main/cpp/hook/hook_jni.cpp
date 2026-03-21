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
    char* module = const_cast<char *>(env->GetStringUTFChars(moduleName, nullptr));
    char* function = const_cast<char *>(env->GetStringUTFChars(functionName, nullptr));
    return (jlong)ArmMemHook::getSymbol(module, function);
}
}