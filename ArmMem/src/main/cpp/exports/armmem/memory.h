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

class ArmMemMemory {
public:
    static MemoryRange toMemoryRange(int id);
    static int getPidByPackage(const char* packageName);
    static uintptr_t getModuleAddress(const char* moduleName, int pid);
    static uintptr_t getModuleAddress(const char* moduleName);
    static int openMemFile(int pid);
    static std::vector<MemoryRegion> getMemoryRegions(int pid, MemoryRange memoryRange);
    static std::vector<MemoryRegion> getMemoryRegions(MemoryRange memoryRange);

    static std::vector<MemoryValue> searchDword(int pid, int value, MemoryRange memoryRange);
    static std::vector<MemoryValue> searchDword(int pid, int value, const std::vector<MemoryValue>& prevList);
    static std::vector<MemoryValue> searchDword(int value, MemoryRange memoryRange);
    static std::vector<MemoryValue> searchDword(int pid, int value, const std::vector<uintptr_t>& prevAddrList);
    static std::vector<MemoryValue> searchDword(int value, const std::vector<MemoryValue>& prevList);
    static std::vector<MemoryValue> searchFloat(int pid, float value, float radius, MemoryRange memoryRange);
    static std::vector<MemoryValue> searchFloat(int pid, float value, float radius, const std::vector<MemoryValue>& prevList);
    static std::vector<MemoryValue> searchFloat(float value, float radius, MemoryRange memoryRange);
    static std::vector<MemoryValue> searchFloat(int pid, float value, float radius, const std::vector<uintptr_t>& prevAddrList);
    static std::vector<MemoryValue> searchFloat(float value, float radius, const std::vector<MemoryValue>& prevList);
    static std::vector<MemoryValue> searchDouble(int pid, double value, double radius, MemoryRange memoryRange);
    static std::vector<MemoryValue> searchDouble(int pid, double value, double radius, const std::vector<uintptr_t>& prevAddrList);
    static std::vector<MemoryValue> searchDouble(int pid, double value, double radius, const std::vector<MemoryValue>& prevList);
    static std::vector<MemoryValue> searchDouble(double value, double radius, MemoryRange memoryRange);
    static std::vector<MemoryValue> searchDouble(double value, double radius, const std::vector<MemoryValue>& prevList);

    static bool writeMemory(int pid, uintptr_t address, void* buffer, size_t size);
    static bool writeMemory(uintptr_t address, void* buffer, size_t size);
    static bool writeDword(int pid, uintptr_t address, int value);
    static bool writeDword(uintptr_t address, int value);
    static bool writeFloat(int pid, uintptr_t address, float value);
    static bool writeFloat(uintptr_t address, float value);
    static bool writeDouble(int pid, uintptr_t address, double value);
    static bool writeDouble(uintptr_t address, double value);
    static bool writeQword(int pid, uintptr_t address, long long value);
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
};

#endif