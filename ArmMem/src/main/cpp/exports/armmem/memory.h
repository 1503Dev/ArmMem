//
// Created by TheChuan1503 on 2026/3/15.
//

#ifndef ARMMEM_MEMORY_H
#define ARMMEM_MEMORY_H

#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "memory_value.h"
#include "memory_region.h"
#include "memory_monitor_handle.h"
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>

class ArmMemMemory {
public:
    static MemoryRange toMemoryRange(int id);

    static int getPidByPackage(const char *packageName);

    static uintptr_t getModuleAddress(const char *moduleName, int pid);

    static uintptr_t getModuleAddress(const char *moduleName);

    static int openMemFile(int pid);

    static std::vector<MemoryRegion> getMemoryRegions(int pid, MemoryRange memoryRange);

    static std::vector<MemoryRegion> getMemoryRegions(MemoryRange memoryRange);

    static std::vector<MemoryValue> searchDword(int pid, int value, MemoryRange memoryRange);

    static std::vector<MemoryValue>
    searchFloat(int pid, float value, float radius, MemoryRange memoryRange);

    static std::vector<MemoryValue>
    searchDouble(int pid, double value, double radius, MemoryRange memoryRange);

    static std::vector<MemoryValue> searchQword(int pid, long long value, MemoryRange memoryRange);

    static std::vector<MemoryValue> searchByte(int pid, signed char value, MemoryRange memoryRange);

    static std::vector<MemoryValue> searchWord(int pid, short wordValue, MemoryRange memoryRange);

    static std::vector<MemoryValue>
    searchDword(int pid, int value, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchFloat(int pid, float value, float radius, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchDouble(int pid, double value, double radius, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchQword(int pid, long long value, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchByte(int pid, signed char value, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchWord(int pid, short wordValue, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue> searchDword(int value, MemoryRange memoryRange);

    static std::vector<MemoryValue> searchFloat(float value, float radius, MemoryRange memoryRange);

    static std::vector<MemoryValue>
    searchDouble(double value, double radius, MemoryRange memoryRange);

    static std::vector<MemoryValue> searchQword(long long value, MemoryRange memoryRange);

    static std::vector<MemoryValue> searchByte(signed char value, MemoryRange memoryRange);

    static std::vector<MemoryValue> searchWord(short wordValue, MemoryRange memoryRange);

    static std::vector<MemoryValue>
    searchDword(int pid, int value, const std::vector<uintptr_t> &prevAddrList);

    static std::vector<MemoryValue>
    searchFloat(int pid, float value, float radius, const std::vector<uintptr_t> &prevAddrList);

    static std::vector<MemoryValue>
    searchDouble(int pid, double value, double radius, const std::vector<uintptr_t> &prevAddrList);

    static std::vector<MemoryValue>
    searchQword(int pid, long long value, const std::vector<uintptr_t> &prevAddrList);

    static std::vector<MemoryValue>
    searchByte(int pid, signed char value, const std::vector<uintptr_t> &prevAddrList);

    static std::vector<MemoryValue>
    searchWord(int pid, short wordValue, const std::vector<uintptr_t> &prevAddrList);

    static std::vector<MemoryValue>
    searchDword(int value, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchFloat(float value, float radius, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchDouble(double value, double radius, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchQword(long long value, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchByte(signed char value, const std::vector<MemoryValue> &prevList);

    static std::vector<MemoryValue>
    searchWord(short wordValue, const std::vector<MemoryValue> &prevList);

    static bool writeMemory(int pid, uintptr_t address, void *buffer, size_t size);

    static bool writeDword(int pid, uintptr_t address, int value);

    static bool writeFloat(int pid, uintptr_t address, float value);

    static bool writeDouble(int pid, uintptr_t address, double value);

    static bool writeQword(int pid, uintptr_t address, long long value);

    static bool writeMemory(uintptr_t address, void *buffer, size_t size);

    static bool writeDword(uintptr_t address, int value);

    static bool writeFloat(uintptr_t address, float value);

    static bool writeDouble(uintptr_t address, double value);

    static bool writeQword(uintptr_t address, long long value);

    static int readDword(int pid, uintptr_t address, bool *success = nullptr);

    static float readFloat(int pid, uintptr_t address, bool *success = nullptr);

    static double readDouble(int pid, uintptr_t address, bool *success = nullptr);

    static long long readQword(int pid, uintptr_t address, bool *success = nullptr);

    static int readDword(uintptr_t address, bool *success = nullptr);

    static float readFloat(uintptr_t address, bool *success = nullptr);

    static double readDouble(uintptr_t address, bool *success = nullptr);

    static long long readQword(uintptr_t address, bool *success = nullptr);

    static int readDword(uintptr_t address, int fd, bool *success = nullptr);

    static float readFloat(uintptr_t address, int fd, bool *success = nullptr);

    static double readDouble(uintptr_t address, int fd, bool *success = nullptr);

    static long long readQword(uintptr_t address, int fd, bool *success = nullptr);

    static MemoryMonitorHandle* listenForWrite(int pid, uintptr_t address, void *callback = nullptr, void *userData = nullptr);
    static MemoryMonitorHandle* listenForWrite(uintptr_t address, void *callback = nullptr, void *userData = nullptr);
    static MemoryMonitorHandle* listenForWriteOnce(int pid, uintptr_t address, void *callback = nullptr, void *userData = nullptr);
    static MemoryMonitorHandle* listenForWriteOnce(uintptr_t address, void *callback = nullptr, void *userData = nullptr);
    static MemoryMonitorHandle* listenForRead(int pid, uintptr_t address, void *callback = nullptr, void *userData = nullptr);
    static MemoryMonitorHandle* listenForRead(uintptr_t address, void *callback = nullptr, void *userData = nullptr);
    static MemoryMonitorHandle* listenForReadOnce(int pid, uintptr_t address, void *callback = nullptr, void *userData = nullptr);
    static MemoryMonitorHandle* listenForReadOnce(uintptr_t address, void *callback = nullptr, void *userData = nullptr);
    static bool unlisten(MemoryMonitorHandle* handle);
private:
    constexpr static char* const TAG = "Memory";

    static void syncMonitorSignalHandler(int sig, siginfo_t* si, void* context);
    static MemoryMonitorHandle* _listenForWrite(MemoryMonitorHandle* handle);
    static void _updatePageProtection(uintptr_t address);
    static MemoryMonitorHandle* listen(int pid, uintptr_t address, int type, void *callback, void *userData);

    static std::unordered_map<int, std::shared_ptr<MemoryMonitorHandle*>> m_monitorHandles;
    static std::mutex m_monitorMutex;
};

#endif