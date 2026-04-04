#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <armmem/memory_monitor_hit.h>
#include "armmem/hook.h"
#include "armmem/memory.h"
#include "armmem.h"

static jstring (*original_stringFromJNI)(JNIEnv *, jobject) = nullptr;

HookFunctionHandle* h = nullptr;

int iv = 1145141919;
//int iv2 = 1145141919;
//int iv3 = 1145141919;
int iv5 = 12346;
float fv = 114514.1919;
int aaa = 15031503;
short sv = 12346;
char cv = 22;
char cv2 = 21;

jstring hooked_stringFromJNI(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, "ArmMem", "Hello from Hook HOOKHOOKHOOK");
    return env->NewStringUTF("Hello from Hook HOOKHOOKHOOK");
    return original_stringFromJNI(env, thiz);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_dev1503_armmem_app_JNI_stringFromJNI(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, "ArmMem", "Hello from my C++");
#ifdef __aarch64__
    return env->NewStringUTF("Hello from my C++ ARM64");
#endif
    return env->NewStringUTF("Hello from my C++ ARMEABI");
}

extern "C"
JNIEXPORT void JNICALL
Java_dev1503_armmem_app_JNI_hook(JNIEnv *env, jobject thiz) {
//    void* handle = dlopen("libapp.so", RTLD_LAZY | RTLD_NOLOAD);
//    if (!handle) {
//        __android_log_print(ANDROID_LOG_ERROR, "ArmMem", "无法加载库: %s", dlerror());
//    }
    h = ArmMemHook::hook("libapp.so", "Java_dev1503_armmem_app_JNI_stringFromJNI", (void*)hooked_stringFromJNI, (void**)&original_stringFromJNI);
    __android_log_print(ANDROID_LOG_INFO, "HOOK OK", "%s", h->message);
}

extern "C"
JNIEXPORT void JNICALL
Java_dev1503_armmem_app_JNI_unHook(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, "UNHOOK", "%i", ArmMemHook::unhook(h));
}

void logV(const char* msg) {
    __android_log_print(ANDROID_LOG_INFO, "ArmMem", "%s", msg);
}

void logE(const char* msg) {
    __android_log_print(ANDROID_LOG_ERROR, "ArmMem", "%s", msg);
}

extern "C"
JNIEXPORT jint JNICALL
Java_dev1503_armmem_app_JNI_geti(JNIEnv *env, jobject thiz) {
    ArmMem::setLoggerE(&logE);
    ArmMem::setLoggerV(&logV);
    return iv;
}
void setI(int i) {
    iv = i;
}

extern "C"
JNIEXPORT jint JNICALL
Java_dev1503_armmem_app_JNI_modi(JNIEnv *env, jobject thiz) {
//    std::vector<MemoryValue> results = ArmMemMemory::searchDword(1145141919, MemoryRange::C_DATA);
//    __android_log_print(ANDROID_LOG_INFO, "MODI", "%i", results.size());
//    if (!results.empty()) {
//        __android_log_print(ANDROID_LOG_INFO, "MODI", "result: %p: %i", (void*)results[0].address, results[0].value.dwordValue);
//        ArmMemMemory::writeDword(results[0].address, 15031503);
//    }
//
//    results = ArmMemMemory::searchFloat(114514.19, 0.05, MemoryRange::C_DATA);
//    __android_log_print(ANDROID_LOG_INFO, "MODF", "%i", results.size());
//    if (!results.empty()) {
//        __android_log_print(ANDROID_LOG_INFO, "MODF", "result: %p: %f", (void*)results[0].address, results[0].value.floatValue);
//    }

    setI(1145141818);

    return 0;
}
#define LOG_TAG "ArmMem_Monitor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void* cb(MemoryMonitorHandle* handle, MemoryMonitorHit* hit) {
    LOGI("HANDLE\naddr %u\ntype %i\nhash %i\nisOnce %i\npid %i\nsize %u\nuserData %u", handle->address, handle->type, handle->hash, handle->isOnce, handle->pid, handle->size, handle->userData);
    LOGI("HIT\nnoriginalValue %u\naccessorAddress %u\naccessorFunction %u\naccessorModuleBase %u\naccessorSymbol %s\naccessorModuleName %s", hit->originalValue, hit->accessorAddress, hit->accessorFunction, hit->accessorModuleBase, hit->accessorSymbol, hit->accessorModuleName);
//    *prevented = true;
//    return reinterpret_cast<void *>(123456);
    return reinterpret_cast<void*>(11451503);
}

static MemoryMonitorHandle* handleWr = nullptr;
static MemoryMonitorHandle* handleRd = nullptr;

extern "C"
JNIEXPORT jint JNICALL
Java_dev1503_armmem_app_JNI_modi2(JNIEnv *env, jobject thiz, jlong addr) {
    auto addrPtr = static_cast<uintptr_t>(addr);
    handleWr = ArmMemMemory::listenForWrite(addrPtr, (void*)cb, (void*)(uintptr_t)123456);
    return 1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_dev1503_armmem_app_JNI_unlisWr(JNIEnv *env, jobject thiz) {
    if (handleWr) {
        ArmMemMemory::unlisten(handleWr);
    }
    return 1;
}

extern "C"
JNIEXPORT void JNICALL
Java_dev1503_armmem_app_JNI_handleTest(JNIEnv *env, jobject thiz) {
    LOGI("handleTest: %p, hash %i, addr %u", handleWr, handleWr->hash, handleWr->address);
}

extern "C"
JNIEXPORT void JNICALL
Java_dev1503_armmem_app_JNI_lisRd(JNIEnv *env, jobject thiz, jlong addr) {
    auto addrPtr = static_cast<uintptr_t>(addr);
    handleRd = ArmMemMemory::listenForRead(addrPtr, (void*)cb, (void*)(uintptr_t)789113);
}
extern "C"
JNIEXPORT void JNICALL
Java_dev1503_armmem_app_JNI_unlisRd(JNIEnv *env, jobject thiz) {
    if (handleRd) {
        ArmMemMemory::unlisten(handleRd);
    }
}
