#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include "armmem/hook.h"
#include "armmem/memory.h"

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

extern "C"
JNIEXPORT jint JNICALL
Java_dev1503_armmem_app_JNI_geti(JNIEnv *env, jobject thiz) {
    return iv;
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

    iv = 1145141818;

    return 0;
}
#include <jni.h>
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "ArmMem_Monitor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static uintptr_t g_monitored_addr = 0;

void sync_handler(int sig, siginfo_t *si, void *context) {
    ucontext_t *uc = (ucontext_t *)context;

#ifdef __aarch64__
    uintptr_t pc = uc->uc_mcontext.pc;
#else
    uintptr_t pc = uc->uc_mcontext.arm_pc;
#endif
    uintptr_t fault_addr = (uintptr_t)si->si_addr;


}

extern "C"
JNIEXPORT jint JNICALL
Java_dev1503_armmem_app_JNI_modi2(JNIEnv *env, jobject thiz, jlong addr) {


    return 1;
}
