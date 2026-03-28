#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#include <jni.h>
#include "../exports/armmem/memory.h"

//
// Created by TheChuan1503 on 2026/3/18.
//

extern "C" {
JNIEXPORT jint JNICALL
Java_dev1503_armmem_memory_JNI_getPid__(JNIEnv *env, jclass clazz) {
    return getpid();
}
JNIEXPORT jint JNICALL
Java_dev1503_armmem_memory_JNI_getPid__Ljava_lang_String_2(JNIEnv *env, jclass clazz, jstring packageName) {
    std::string packageNameStr = env->GetStringUTFChars(packageName, nullptr);
    return ArmMemMemory::getPidByPackage(packageNameStr.c_str());
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchDword__III(JNIEnv *env, jclass clazz, jint pid, jint value,
                                           jint memoryRange) {
    std::vector<MemoryValue> results = ArmMemMemory::searchDword(pid, value, ArmMemMemory::toMemoryRange(memoryRange));
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchDword__II_3J(JNIEnv *env, jclass clazz, jint pid, jint value, jlongArray prevList) {
    jsize len = env->GetArrayLength(prevList);
    jlong* prevListElements = env->GetLongArrayElements(prevList, nullptr);
    std::vector<uintptr_t> prevListVector(prevListElements, prevListElements + len);
    env->ReleaseLongArrayElements(prevList, prevListElements, JNI_ABORT);

    std::vector<MemoryValue> results = ArmMemMemory::searchDword(pid, value, prevListVector);
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);

    return resultArray;
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchDword__II(JNIEnv *env, jclass clazz, jint value, jint memoryRange) {
    return Java_dev1503_armmem_memory_JNI_searchDword__III(env, clazz, getpid(), value, memoryRange);
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchDword__I_3J(JNIEnv *env, jclass clazz, jint value, jlongArray prevList) {
    return Java_dev1503_armmem_memory_JNI_searchDword__II_3J(env, clazz, getpid(), value, prevList);
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchFloat__IFFI(JNIEnv *env, jclass clazz, jint pid, jfloat value, jfloat radius,
                                                jint memoryRange) {
    std::vector<MemoryValue> results = ArmMemMemory::searchFloat(pid, value, radius, ArmMemMemory::toMemoryRange(memoryRange));
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchFloat__IFF_3J(JNIEnv *env, jclass clazz, jint pid, jfloat value, jfloat radius, jlongArray prevList) {
    jsize len = env->GetArrayLength(prevList);
    jlong* prevListElements = env->GetLongArrayElements(prevList, nullptr);
    std::vector<uintptr_t> prevListVector(prevListElements, prevListElements + len);
    env->ReleaseLongArrayElements(prevList, prevListElements, JNI_ABORT);

    std::vector<MemoryValue> results = ArmMemMemory::searchFloat(pid, value, radius, prevListVector);
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);

    return resultArray;
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchFloat__FFI(JNIEnv *env, jclass clazz, jfloat value, jfloat radius,
                                                jint memoryRange) {
    return Java_dev1503_armmem_memory_JNI_searchFloat__IFFI(env, clazz, getpid(), value, radius, memoryRange);
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchFloat__FF_3J(JNIEnv *env, jclass clazz, jfloat value, jfloat radius, jlongArray prevList) {
    return Java_dev1503_armmem_memory_JNI_searchFloat__IFF_3J(env, clazz, getpid(), value, radius, prevList);
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchDouble__IDDI(JNIEnv *env, jclass clazz, jint pid, jdouble value, jdouble radius,
                                                 jint memoryRange) {
    std::vector<MemoryValue> results = ArmMemMemory::searchDouble(pid, value, radius, ArmMemMemory::toMemoryRange(memoryRange));
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchDouble__IDD_3J(JNIEnv *env, jclass clazz, jint pid, jdouble value, jdouble radius, jlongArray prevList) {
    jsize len = env->GetArrayLength(prevList);
    jlong* prevListElements = env->GetLongArrayElements(prevList, nullptr);
    std::vector<uintptr_t> prevListVector(prevListElements, prevListElements + len);
    env->ReleaseLongArrayElements(prevList, prevListElements, JNI_ABORT);

    std::vector<MemoryValue> results = ArmMemMemory::searchDouble(pid, value, radius, prevListVector);
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);

    return resultArray;
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchDouble__DDI(JNIEnv *env, jclass clazz, jdouble value, jdouble radius,
                                                jint memoryRange) {
    return Java_dev1503_armmem_memory_JNI_searchDouble__IDDI(env, clazz, getpid(), value, radius, memoryRange);
}
JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchDouble__DD_3J(JNIEnv *env, jclass clazz, jdouble value, jdouble radius, jlongArray prevList) {
    return Java_dev1503_armmem_memory_JNI_searchDouble__IDD_3J(env, clazz, getpid(), value, radius, prevList);
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchQword__IJI(JNIEnv *env, jclass clazz, jint pid, jlong value,
                                                jint memoryRange) {
    std::vector<MemoryValue> results = ArmMemMemory::searchQword(pid, value, ArmMemMemory::toMemoryRange(memoryRange));
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchQword__IJ_3J(JNIEnv *env, jclass clazz, jint pid, jlong value, jlongArray prevList) {
    jsize len = env->GetArrayLength(prevList);
    jlong* prevListElements = env->GetLongArrayElements(prevList, nullptr);
    std::vector<uintptr_t> prevListVector(prevListElements, prevListElements + len);
    env->ReleaseLongArrayElements(prevList, prevListElements, JNI_ABORT);

    std::vector<MemoryValue> results = ArmMemMemory::searchQword(pid, value, prevListVector);
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchQword__JI(JNIEnv *env, jclass clazz, jlong value, jint memoryRange) {
    return Java_dev1503_armmem_memory_JNI_searchQword__IJI(env, clazz, getpid(), value, memoryRange);
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchQword__J_3J(JNIEnv *env, jclass clazz, jlong value, jlongArray prevList) {
    return Java_dev1503_armmem_memory_JNI_searchQword__IJ_3J(env, clazz, getpid(), value, prevList);
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchByte__IBI(JNIEnv *env, jclass clazz, jint pid, jbyte value,
                                               jint memoryRange) {
    std::vector<MemoryValue> results = ArmMemMemory::searchByte(pid, (signed char)value, ArmMemMemory::toMemoryRange(memoryRange));
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchByte__IB_3J(JNIEnv *env, jclass clazz, jint pid, jbyte value, jlongArray prevList) {
    jsize len = env->GetArrayLength(prevList);
    jlong* prevListElements = env->GetLongArrayElements(prevList, nullptr);
    std::vector<uintptr_t> prevListVector(prevListElements, prevListElements + len);
    env->ReleaseLongArrayElements(prevList, prevListElements, JNI_ABORT);

    std::vector<MemoryValue> results = ArmMemMemory::searchByte(pid, (signed char)value, prevListVector);
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchByte__BI(JNIEnv *env, jclass clazz, jbyte value, jint memoryRange) {
    return Java_dev1503_armmem_memory_JNI_searchByte__IBI(env, clazz, getpid(), value, memoryRange);
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchByte__B_3J(JNIEnv *env, jclass clazz, jbyte value, jlongArray prevList) {
    return Java_dev1503_armmem_memory_JNI_searchByte__IB_3J(env, clazz, getpid(), value, prevList);
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchWord__ISI(JNIEnv *env, jclass clazz, jint pid, jshort value,
                                               jint memoryRange) {
    std::vector<MemoryValue> results = ArmMemMemory::searchWord(pid, (short)value, ArmMemMemory::toMemoryRange(memoryRange));
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchWord__IS_3J(JNIEnv *env, jclass clazz, jint pid, jshort value, jlongArray prevList) {
    jsize len = env->GetArrayLength(prevList);
    jlong* prevListElements = env->GetLongArrayElements(prevList, nullptr);
    std::vector<uintptr_t> prevListVector(prevListElements, prevListElements + len);
    env->ReleaseLongArrayElements(prevList, prevListElements, JNI_ABORT);

    std::vector<MemoryValue> results = ArmMemMemory::searchWord(pid, (short)value, prevListVector);
    jlongArray resultArray = env->NewLongArray(results.size());
    jlong* elements = env->GetLongArrayElements(resultArray, nullptr);
    for (size_t i = 0; i < results.size(); i++) {
        elements[i] = (jlong)results[i].address;
    }
    env->ReleaseLongArrayElements(resultArray, elements, 0);
    return resultArray;
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchWord__SI(JNIEnv *env, jclass clazz, jshort value, jint memoryRange) {
    return Java_dev1503_armmem_memory_JNI_searchWord__ISI(env, clazz, getpid(), value, memoryRange);
}

JNIEXPORT jlongArray JNICALL
Java_dev1503_armmem_memory_JNI_searchWord__S_3J(JNIEnv *env, jclass clazz, jshort value, jlongArray prevList) {
    return Java_dev1503_armmem_memory_JNI_searchWord__IS_3J(env, clazz, getpid(), value, prevList);
}

JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeDword__IJI(JNIEnv *env, jclass clazz, jint pid, jlong address,
                                               jint value) {
    ArmMemMemory::writeDword(pid, (uintptr_t)address, value);
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeDword__JI(JNIEnv *env, jclass clazz, jlong address, jint value) {
    ArmMemMemory::writeDword((uintptr_t)address, value);
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeFloat__IJF(JNIEnv *env, jclass clazz, jint pid, jlong address,
                                               jfloat value) {
    ArmMemMemory::writeFloat(pid, (uintptr_t)address, value);
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeFloat__JF(JNIEnv *env, jclass clazz, jlong address, jfloat value) {
    ArmMemMemory::writeFloat((uintptr_t)address, value);
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeDouble__IJD(JNIEnv *env, jclass clazz, jint pid, jlong address,
                                               jdouble value) {
    ArmMemMemory::writeDouble(pid, (uintptr_t)address, value);
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeDouble__JD(JNIEnv *env, jclass clazz, jlong address, jdouble value) {
    ArmMemMemory::writeDouble((uintptr_t)address, value);
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeQword__IJJ(JNIEnv *env, jclass clazz, jint pid, jlong address,
                                               jlong value) {
    ArmMemMemory::writeQword(pid, (uintptr_t)address, value);
}

JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeQword__JJ(JNIEnv *env, jclass clazz, jlong address, jlong value) {
    ArmMemMemory::writeQword((uintptr_t)address, value);
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeByte__IJB(JNIEnv *env, jclass clazz, jint pid, jlong address,
                                              jbyte value) {
    auto val = (signed char)value;
    ArmMemMemory::writeMemory(pid, (uintptr_t)address, &val, sizeof(signed char));
}

JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeByte__JB(JNIEnv *env, jclass clazz, jlong address, jbyte value) {
    auto val = (signed char)value;
    ArmMemMemory::writeMemory((uintptr_t)address, &val, sizeof(signed char));
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeWord__IJS(JNIEnv *env, jclass clazz, jint pid, jlong address,
                                              jshort value) {
    auto val = (short)value;
    ArmMemMemory::writeMemory(pid, (uintptr_t)address, &val, sizeof(short));
}

JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_writeWord__JS(JNIEnv *env, jclass clazz, jlong address, jshort value) {
    auto val = (short)value;
    ArmMemMemory::writeMemory((uintptr_t)address, &val, sizeof(short));
}

JNIEXPORT jint JNICALL
Java_dev1503_armmem_memory_JNI_openMemFile(JNIEnv *env, jclass clazz, jint pid) {
    return ArmMemMemory::openMemFile(pid);
}
JNIEXPORT void JNICALL
Java_dev1503_armmem_memory_JNI_closeMemFile(JNIEnv *env, jclass clazz, jint fd) {
    close(fd);
}

JNIEXPORT jint JNICALL
Java_dev1503_armmem_memory_JNI_readDword__IJ(JNIEnv *env, jclass clazz, jint pid, jlong address) {
    return ArmMemMemory::readDword(pid, (uintptr_t)address);
}
JNIEXPORT jint JNICALL
Java_dev1503_armmem_memory_JNI_readDword__JI(JNIEnv *env, jclass clazz, jlong address, jint fd) {
    return ArmMemMemory::readDword((uintptr_t) address, fd);
}
JNIEXPORT jint JNICALL
Java_dev1503_armmem_memory_JNI_readDword__J(JNIEnv *env, jclass clazz, jlong address) {
    return ArmMemMemory::readDword((uintptr_t) address);
}
JNIEXPORT jfloat JNICALL
Java_dev1503_armmem_memory_JNI_readFloat__IJ(JNIEnv *env, jclass clazz, jint pid, jlong address) {
    return ArmMemMemory::readFloat(pid, (uintptr_t)address);
}
JNIEXPORT jfloat JNICALL
Java_dev1503_armmem_memory_JNI_readFloat__JI(JNIEnv *env, jclass clazz, jlong address, jint fd) {
    return ArmMemMemory::readFloat((uintptr_t) address, fd);
}
JNIEXPORT jfloat JNICALL
Java_dev1503_armmem_memory_JNI_readFloat__J(JNIEnv *env, jclass clazz, jlong address) {
    return ArmMemMemory::readFloat((uintptr_t) address);
}
JNIEXPORT jdouble JNICALL
Java_dev1503_armmem_memory_JNI_readDouble__IJ(JNIEnv *env, jclass clazz, jint pid, jlong address) {
    return ArmMemMemory::readDouble(pid, (uintptr_t)address);
}
JNIEXPORT jdouble JNICALL
Java_dev1503_armmem_memory_JNI_readDouble__JI(JNIEnv *env, jclass clazz, jlong address, jint fd) {
    return ArmMemMemory::readDouble((uintptr_t) address, fd);
}
JNIEXPORT jdouble JNICALL
Java_dev1503_armmem_memory_JNI_readDouble__J(JNIEnv *env, jclass clazz, jlong address) {
    return ArmMemMemory::readDouble((uintptr_t) address);
}
JNIEXPORT jlong JNICALL
Java_dev1503_armmem_memory_JNI_readQword__IJ(JNIEnv *env, jclass clazz, jint pid, jlong address) {
    return (jlong)ArmMemMemory::readQword(pid, (uintptr_t)address);
}

JNIEXPORT jlong JNICALL
Java_dev1503_armmem_memory_JNI_readQword__JI(JNIEnv *env, jclass clazz, jlong address, jint fd) {
    return (jlong)ArmMemMemory::readQword((uintptr_t)address, fd);
}

JNIEXPORT jlong JNICALL
Java_dev1503_armmem_memory_JNI_readQword__J(JNIEnv *env, jclass clazz, jlong address) {
    return (jlong)ArmMemMemory::readQword((uintptr_t)address);
}
JNIEXPORT jbyte JNICALL
Java_dev1503_armmem_memory_JNI_readByte__IJ(JNIEnv *env, jclass clazz, jint pid, jlong address) {
    signed char value = 0;
    int fd = ArmMemMemory::openMemFile(pid);
    if (fd != -1) {
        pread64(fd, &value, sizeof(signed char), (off64_t)address);
        close(fd);
    }
    return (jbyte)value;
}

JNIEXPORT jbyte JNICALL
Java_dev1503_armmem_memory_JNI_readByte__JI(JNIEnv *env, jclass clazz, jlong address, jint fd) {
    signed char value = 0;
    pread64(fd, &value, sizeof(signed char), (off64_t)address);
    return (jbyte)value;
}

JNIEXPORT jbyte JNICALL
Java_dev1503_armmem_memory_JNI_readByte__J(JNIEnv *env, jclass clazz, jlong address) {
    return Java_dev1503_armmem_memory_JNI_readByte__IJ(env, clazz, getpid(), address);
}
JNIEXPORT jshort JNICALL
Java_dev1503_armmem_memory_JNI_readWord__IJ(JNIEnv *env, jclass clazz, jint pid, jlong address) {
    short value = 0;
    int fd = ArmMemMemory::openMemFile(pid);
    if (fd != -1) {
        pread64(fd, &value, sizeof(short), (off64_t)address);
        close(fd);
    }
    return (jshort)value;
}

JNIEXPORT jshort JNICALL
Java_dev1503_armmem_memory_JNI_readWord__JI(JNIEnv *env, jclass clazz, jlong address, jint fd) {
    short value = 0;
    pread64(fd, &value, sizeof(short), (off64_t)address);
    return (jshort)value;
}

JNIEXPORT jshort JNICALL
Java_dev1503_armmem_memory_JNI_readWord__J(JNIEnv *env, jclass clazz, jlong address) {
    return Java_dev1503_armmem_memory_JNI_readWord__IJ(env, clazz, getpid(), address);
}




}

#pragma clang diagnostic pop