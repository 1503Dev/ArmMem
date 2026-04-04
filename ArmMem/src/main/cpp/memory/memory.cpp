//
// Created by TheChuan1503 on 2026/3/15.
//

#include <jni.h>
#include <android/log.h>
#include "../exports/armmem/memory.h"
#include "../exports/armmem/memory_value.h"
#include "../exports/armmem/memory_region.h"
#include "../exports/armmem.h"
#include "../exports/armmem/memory_monitor_hit.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>
#include <dlfcn.h>

MemoryRange ArmMemMemory::toMemoryRange(int id) {
    switch (id) {
//        case 0: return MemoryRange::ALL;
        case 1: return MemoryRange::C_HEAP;
        case 2: return MemoryRange::JAVA_HEAP;
        case 3: return MemoryRange::C_ALLOC;
        case 4: return MemoryRange::C_DATA;
        case 5: return MemoryRange::C_BSS;
        case 6: return MemoryRange::ANONYMOUS;
        case 7: return MemoryRange::CODE_APP;
        case 8: return MemoryRange::STACK;
        case 9: return MemoryRange::ASHMEM;
        default: return MemoryRange::OTHER;
    }
}

int ArmMemMemory::getPidByPackage(const char *packageName) {
    DIR *dir = opendir("/proc");
    if (dir == nullptr) return -1;

    struct dirent *entry;
    int pid = -1;

    while ((entry = readdir(dir)) != nullptr) {
        int id = atoi(entry->d_name);
        if (id <= 0) continue;

        char path[128];
        snprintf(path, sizeof(path), "/proc/%d/cmdline", id);

        int fd = open(path, O_RDONLY);
        if (fd != -1) {
            char cmdname[256] = {0};
            if (read(fd, cmdname, sizeof(cmdname) - 1) > 0) {
                if (strcmp(cmdname, packageName) == 0) {
                    pid = id;
                    close(fd);
                    break;
                }
            }
            close(fd);
        }
    }
    closedir(dir);
    return pid;
}
uintptr_t ArmMemMemory::getModuleAddress(const char *moduleName, int pid) {
    char mapPath[256];
    char mapLine[1024];
    uintptr_t addr = 0;
    snprintf(mapPath, sizeof(mapPath), "/proc/%d/maps", pid);
    FILE *fp = fopen(mapPath, "r");
    if (fp == nullptr) {
        return 0;
    }
    while (fgets(mapLine, sizeof(mapLine), fp)) {
        if (strstr(mapLine, moduleName)) {
            addr = (uintptr_t)strtoull(mapLine, nullptr, 16);
            break;
        }
    }
    fclose(fp);
    return addr;
}
uintptr_t ArmMemMemory::getModuleAddress(const char *moduleName) {
    return getModuleAddress(moduleName, getpid());
}

std::vector<MemoryRegion> ArmMemMemory::getMemoryRegions(int pid, MemoryRange range) {
    std::vector<MemoryRegion> regions;
    char mapPath[128];
    snprintf(mapPath, sizeof(mapPath), "/proc/%d/maps", pid);

    FILE *fp = fopen(mapPath, "r");
    if (!fp) return regions;

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        uintptr_t start, end;
        char perms[5];
        char path[256] = {0};

        int res = sscanf(line, "%lx-%lx %4s %*s %*s %*s %s", &start, &end, perms, path);
        if (res < 3) continue;

        bool match = false;
        bool is_rw = (perms[0] == 'r' && perms[1] == 'w');
        bool is_x = (perms[2] == 'x');
        bool has_path = (strlen(path) > 0);

        switch (range) {
//            case MemoryRange::ALL:
//                if (strcmp(path, "[heap]") == 0 || // c heap
//                    strstr(path, "/dev/ashmem/dalvik") || strstr(path, "art-kae") || // java heap
//                    strstr(path, "[anon:libc_malloc]") || strstr(path, "[anon:scudo:]") || // c alloc
//                    (!has_path && is_rw) || // anonymous
//                    (has_path && is_rw && path[0] == '/' && (strstr(path, ".so") || strstr(path, "/base.apk"))) || // c data/bss
//                    is_x && strstr(path, "/data/app/") || // code app
//                    strcmp(path, "[stack]") == 0 || // stack
//                    (strstr(path, "/dev/ashmem/") && !strstr(path, "dalvik")) || // ashmem
//                    (has_path && path[0] != '[' && !strstr(path, "/data/app/")) /* other */) match = true;
//                break;
            case MemoryRange::C_HEAP:
                if (strcmp(path, "[heap]") == 0) match = true;
                break;
            case MemoryRange::JAVA_HEAP:
                if (strstr(path, "/dev/ashmem/dalvik") || strstr(path, "art-kae")) match = true;
                break;
            case MemoryRange::C_ALLOC:
                if (strstr(path, "[anon:libc_malloc]") || strstr(path, "[anon:scudo:]")) match = true;
                break;
            case MemoryRange::ANONYMOUS:
                if (!has_path && is_rw) match = true;
                break;
            case MemoryRange::C_DATA:
            case MemoryRange::C_BSS:
                if (has_path && is_rw && path[0] == '/') {
                    if (strstr(path, ".so") || strstr(path, "/base.apk")) match = true;
                }
                break;
            case MemoryRange::CODE_APP:
                if (is_x && strstr(path, "/data/app/")) match = true;
                break;
            case MemoryRange::STACK:
                if (strcmp(path, "[stack]") == 0) match = true;
                break;
            case MemoryRange::ASHMEM:
                if (strstr(path, "/dev/ashmem/") && !strstr(path, "dalvik")) match = true;
                break;
            case MemoryRange::OTHER:
                if (has_path && path[0] != '[' && !strstr(path, "/data/app/")) match = true;
                break;
            default:
                break;
        }

        if (match) {
            MemoryRegion reg;
            reg.start = start;
            reg.size = end - start;
            strncpy(reg.path, has_path ? path : "anonymous", sizeof(reg.path));
            regions.push_back(reg);
        }
    }
    fclose(fp);
    return regions;
}
std::vector<MemoryRegion> ArmMemMemory::getMemoryRegions(MemoryRange memoryRange) {
    return getMemoryRegions(getpid(), memoryRange);
}

/*
 * --------------------------------
 * Memory Search
 * --------------------------------
 */

std::vector<MemoryValue> ArmMemMemory::searchDword(int pid, int value, MemoryRange memoryRange) {
    std::vector<MemoryRegion> regions = getMemoryRegions(pid, memoryRange);
    std::vector<MemoryValue> allResults;
    if (regions.empty()) return allResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return allResults;

    const size_t CHUNK_SIZE = 1024 * 1024;
    std::vector<int> buffer(CHUNK_SIZE / sizeof(int));
    allResults.reserve(10000);

    for (const auto& region : regions) {
        uintptr_t currentAddr = region.start;
        uintptr_t endAddr = region.start + region.size;

        if (currentAddr % 4 != 0) {
            currentAddr += (4 - (currentAddr % 4));
        }

        while (currentAddr + sizeof(int) <= endAddr) {
            size_t remaining = endAddr - currentAddr;
            size_t bytesToRead = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;

            bytesToRead &= ~(sizeof(int) - 1);

            ssize_t ret = pread64(fd, buffer.data(), bytesToRead, (off64_t)currentAddr);
            if (ret <= 0) break;

            int count = (int)(ret / sizeof(int));
            for (int i = 0; i < count; i++) {
                if (buffer[i] == value) {
                    MemoryValue item{};
                    item.address = currentAddr + (i * sizeof(int));
                    item.type = ValueType::DWORD;
                    item.value.dwordValue = buffer[i];
                    allResults.push_back(item);
                }
            }
            currentAddr += ret;
        }
    }

    close(fd);
    return allResults;
}
std::vector<MemoryValue> ArmMemMemory::searchDword(int pid, int value, const std::vector<MemoryValue>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        int currentValue = readDword(prevItem.address, fd, &success);
        if (success && currentValue == value) {
            MemoryValue item = prevItem;
            item.value.dwordValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}
std::vector<MemoryValue> ArmMemMemory::searchDword(int pid, int value, const std::vector<uintptr_t>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        int currentValue = readDword(prevItem, fd, &success);
        if (success && currentValue == value) {
            MemoryValue item{};
            item.address = prevItem;
            item.type = ValueType::DWORD;
            item.value.dwordValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}
std::vector<MemoryValue> ArmMemMemory::searchDword(int value, MemoryRange memoryRange) {
    return searchDword(getpid(), value, memoryRange);
}
std::vector<MemoryValue> ArmMemMemory::searchDword(int value, const std::vector<MemoryValue>& prevList) {
    return searchDword(getpid(), value, prevList);
}

std::vector<MemoryValue> ArmMemMemory::searchFloat(int pid, float value, float radius, MemoryRange memoryRange) {
    std::vector<MemoryRegion> regions = getMemoryRegions(pid, memoryRange);
    std::vector<MemoryValue> allResults;
    if (regions.empty()) return allResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return allResults;

    const size_t CHUNK_SIZE = 1024 * 1024;
    std::vector<float> buffer(CHUNK_SIZE / sizeof(float));
    allResults.reserve(10000);

    for (const auto& region : regions) {
        uintptr_t currentAddr = region.start;
        uintptr_t endAddr = region.start + region.size;

        while (currentAddr + sizeof(float) <= endAddr) {
            size_t remaining = endAddr - currentAddr;
            size_t bytesToRead = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;

            ssize_t ret = pread64(fd, buffer.data(), bytesToRead, (off64_t)currentAddr);
            if (ret <= 0) break;

            int count = (int)(ret / sizeof(float));
            for (int i = 0; i < count; i++) {
                if (std::abs(buffer[i] - value) <= radius) {
                    MemoryValue item{};
                    item.address = currentAddr + (i * sizeof(float));
                    item.type = ValueType::FLOAT;
                    item.value.floatValue = buffer[i];
                    allResults.push_back(item);
                }
            }
            currentAddr += ret;
        }
    }

    close(fd);
    return allResults;
}
std::vector<MemoryValue> ArmMemMemory::searchFloat(int pid, float value, float radius, const std::vector<MemoryValue>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        float currentValue = readFloat(prevItem.address, fd, &success);
        if (success && std::abs(currentValue - value) <= radius) {
            MemoryValue item = prevItem;
            item.value.floatValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}
std::vector<MemoryValue> ArmMemMemory::searchFloat(int pid, float value, float radius, const std::vector<uintptr_t>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        float currentValue = readFloat(prevItem, fd, &success);
        if (success && std::abs(currentValue - value) <= radius) {
            MemoryValue item{};
            item.address = prevItem;
            item.type = ValueType::FLOAT;
            item.value.floatValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}
std::vector<MemoryValue> ArmMemMemory::searchFloat(float value, float radius, MemoryRange memoryRange) {
    return searchFloat(getpid(), value, radius, memoryRange);
}
std::vector<MemoryValue> ArmMemMemory::searchFloat(float value, float radius, const std::vector<MemoryValue>& prevList) {
    return searchFloat(getpid(), value, radius, prevList);
}

std::vector<MemoryValue> ArmMemMemory::searchDouble(int pid, double value, double radius, MemoryRange memoryRange) {
    std::vector<MemoryRegion> regions = getMemoryRegions(pid, memoryRange);
    std::vector<MemoryValue> allResults;
    if (regions.empty()) return allResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return allResults;

    const size_t CHUNK_SIZE = 1024 * 1024;
    std::vector<double> buffer(CHUNK_SIZE / sizeof(double));
    allResults.reserve(10000);

    for (const auto& region : regions) {
        uintptr_t currentAddr = region.start;
        uintptr_t endAddr = region.start + region.size;

        while (currentAddr + sizeof(double) <= endAddr) {
            size_t remaining = endAddr - currentAddr;
            size_t bytesToRead = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;

            ssize_t ret = pread64(fd, buffer.data(), bytesToRead, (off64_t)currentAddr);
            if (ret <= 0) break;

            int count = (int)(ret / sizeof(double));
            for (int i = 0; i < count; i++) {
                if (std::abs(buffer[i] - value) <= radius) {
                    MemoryValue item{};
                    item.address = currentAddr + (i * sizeof(double));
                    item.type = ValueType::DOUBLE;
                    item.value.doubleValue = buffer[i];
                    allResults.push_back(item);
                }
            }
            currentAddr += ret;
        }
    }

    close(fd);
    return allResults;
}
std::vector<MemoryValue> ArmMemMemory::searchDouble(int pid, double value, double radius, const std::vector<MemoryValue>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        double currentValue = readDouble(prevItem.address, fd, &success);
        if (success && std::abs(currentValue - value) <= radius) {
            MemoryValue item = prevItem;
            item.value.doubleValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}
std::vector<MemoryValue> ArmMemMemory::searchDouble(int pid, double value, double radius, const std::vector<uintptr_t>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        double currentValue = readDouble(prevItem, fd, &success);
        if (success && std::abs(currentValue - value) <= radius) {
            MemoryValue item{};
            item.address = prevItem;
            item.type = ValueType::DOUBLE;
            item.value.doubleValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}
std::vector<MemoryValue> ArmMemMemory::searchDouble(double value, double radius, MemoryRange memoryRange) {
    return searchDouble(getpid(), value, radius, memoryRange);
}
std::vector<MemoryValue> ArmMemMemory::searchDouble(double value, double radius, const std::vector<MemoryValue>& prevList) {
    return searchDouble(getpid(), value, radius, prevList);
}

std::vector<MemoryValue> ArmMemMemory::searchQword(int pid, long long value, MemoryRange memoryRange) {
    std::vector<MemoryRegion> regions = getMemoryRegions(pid, memoryRange);
    std::vector<MemoryValue> allResults;
    if (regions.empty()) return allResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return allResults;

    const size_t CHUNK_SIZE = 1024 * 1024;
    std::vector<long long> buffer(CHUNK_SIZE / sizeof(long long));
    allResults.reserve(10000);

    for (const auto& region : regions) {
        uintptr_t currentAddr = region.start;
        uintptr_t endAddr = region.start + region.size;

        while (currentAddr + sizeof(long long) <= endAddr) {
            size_t remaining = endAddr - currentAddr;
            size_t bytesToRead = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;

            ssize_t ret = pread64(fd, buffer.data(), bytesToRead, (off64_t)currentAddr);
            if (ret <= 0) break;

            int count = (int)(ret / sizeof(long long));
            for (int i = 0; i < count; i++) {
                if (buffer[i] == value) {
                    MemoryValue item{};
                    item.address = currentAddr + (i * sizeof(long long));
                    item.type = ValueType::QWORD;
                    item.value.qwordValue = buffer[i];
                    allResults.push_back(item);
                }
            }
            currentAddr += ret;
        }
    }

    close(fd);
    return allResults;
}
std::vector<MemoryValue> ArmMemMemory::searchQword(int pid, long long value, const std::vector<MemoryValue>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        long long currentValue = readQword(prevItem.address, fd, &success);
        if (success && currentValue == value) {
            MemoryValue item = prevItem;
            item.value.qwordValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}
std::vector<MemoryValue> ArmMemMemory::searchQword(int pid, long long value, const std::vector<uintptr_t>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        long long currentValue = readQword(prevItem, fd, &success);
        if (success && currentValue == value) {
            MemoryValue item{};
            item.address = prevItem;
            item.type = ValueType::QWORD;
            item.value.qwordValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}
std::vector<MemoryValue> ArmMemMemory::searchQword(long long value, MemoryRange memoryRange) {
    return searchQword(getpid(), value, memoryRange);
}
std::vector<MemoryValue> ArmMemMemory::searchQword(long long value, const std::vector<MemoryValue>& prevList) {
    return searchQword(getpid(), value, prevList);
}

std::vector<MemoryValue> ArmMemMemory::searchByte(int pid, signed char value, MemoryRange memoryRange) {
    std::vector<MemoryRegion> regions = getMemoryRegions(pid, memoryRange);
    std::vector<MemoryValue> allResults;
    if (regions.empty()) return allResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return allResults;

    const size_t CHUNK_SIZE = 1024 * 1024;
    std::vector<signed char> buffer(CHUNK_SIZE / sizeof(signed char));
    allResults.reserve(10000);

    for (const auto& region : regions) {
        uintptr_t currentAddr = region.start;
        uintptr_t endAddr = region.start + region.size;

        while (currentAddr + sizeof(signed char) <= endAddr) {
            size_t remaining = endAddr - currentAddr;
            size_t bytesToRead = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;

            ssize_t ret = pread64(fd, buffer.data(), bytesToRead, (off64_t)currentAddr);
            if (ret <= 0) break;

            int count = (int)(ret / sizeof(signed char));
            for (int i = 0; i < count; i++) {
                if (buffer[i] == value) {
                    MemoryValue item{};
                    item.address = currentAddr + (i * sizeof(signed char));
                    item.type = ValueType::BYTE;
                    item.value.byteValue = buffer[i];
                    allResults.push_back(item);
                }
            }
            currentAddr += ret;
        }
    }

    close(fd);
    return allResults;
}

std::vector<MemoryValue> ArmMemMemory::searchByte(int pid, signed char value, const std::vector<MemoryValue>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        signed char currentValue = 0;
        ssize_t ret = pread64(fd, &currentValue, sizeof(signed char), (off64_t)prevItem.address);
        success = (ret == sizeof(signed char));
        if (success && currentValue == value) {
            MemoryValue item = prevItem;
            item.value.byteValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}

std::vector<MemoryValue> ArmMemMemory::searchByte(int pid, signed char value, const std::vector<uintptr_t>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        signed char currentValue = 0;
        ssize_t ret = pread64(fd, &currentValue, sizeof(signed char), (off64_t)prevItem);
        success = (ret == sizeof(signed char));
        if (success && currentValue == value) {
            MemoryValue item{};
            item.address = prevItem;
            item.type = ValueType::BYTE;
            item.value.byteValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}

std::vector<MemoryValue> ArmMemMemory::searchByte(signed char value, MemoryRange memoryRange) {
    return searchByte(getpid(), value, memoryRange);
}

std::vector<MemoryValue> ArmMemMemory::searchByte(signed char value, const std::vector<MemoryValue>& prevList) {
    return searchByte(getpid(), value, prevList);
}

std::vector<MemoryValue> ArmMemMemory::searchWord(int pid, short value, MemoryRange memoryRange) {
    std::vector<MemoryRegion> regions = getMemoryRegions(pid, memoryRange);
    std::vector<MemoryValue> allResults;
    if (regions.empty()) return allResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return allResults;

    const size_t CHUNK_SIZE = 1024 * 1024;
    std::vector<short> buffer(CHUNK_SIZE / sizeof(short));
    allResults.reserve(10000);

    for (const auto& region : regions) {
        uintptr_t currentAddr = region.start;
        uintptr_t endAddr = region.start + region.size;

        while (currentAddr + sizeof(short) <= endAddr) {
            size_t remaining = endAddr - currentAddr;
            size_t bytesToRead = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;

            ssize_t ret = pread64(fd, buffer.data(), bytesToRead, (off64_t)currentAddr);
            if (ret <= 0) break;

            int count = (int)(ret / sizeof(short));
            for (int i = 0; i < count; i++) {
                if (buffer[i] == value) {
                    MemoryValue item{};
                    item.address = currentAddr + (i * sizeof(short));
                    item.type = ValueType::WORD;
                    item.value.wordValue = buffer[i];
                    allResults.push_back(item);
                }
            }
            currentAddr += ret;
        }
    }

    close(fd);
    return allResults;
}

std::vector<MemoryValue> ArmMemMemory::searchWord(int pid, short value, const std::vector<MemoryValue>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        short currentValue = 0;
        ssize_t ret = pread64(fd, &currentValue, sizeof(short), (off64_t)prevItem.address);
        success = (ret == sizeof(short));
        if (success && currentValue == value) {
            MemoryValue item = prevItem;
            item.value.wordValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}

std::vector<MemoryValue> ArmMemMemory::searchWord(int pid, short value, const std::vector<uintptr_t>& prevList) {
    std::vector<MemoryValue> nextResults;
    if (prevList.empty()) return nextResults;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return nextResults;

    nextResults.reserve(prevList.size());

    for (const auto& prevItem : prevList) {
        bool success = false;
        short currentValue = 0;
        ssize_t ret = pread64(fd, &currentValue, sizeof(short), (off64_t)prevItem);
        success = (ret == sizeof(short));
        if (success && currentValue == value) {
            MemoryValue item{};
            item.address = prevItem;
            item.type = ValueType::WORD;
            item.value.wordValue = currentValue;
            nextResults.push_back(item);
        }
    }

    close(fd);
    return nextResults;
}

std::vector<MemoryValue> ArmMemMemory::searchWord(short value, MemoryRange memoryRange) {
    return searchWord(getpid(), value, memoryRange);
}

std::vector<MemoryValue> ArmMemMemory::searchWord(short value, const std::vector<MemoryValue>& prevList) {
    return searchWord(getpid(), value, prevList);
}

/*
 * --------------------------------
 * Memory Modify
 * --------------------------------
 */

bool ArmMemMemory::writeMemory(int pid, uintptr_t address, void* buffer, size_t size) {
    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_WRONLY);
    if (fd == -1) return false;

    ssize_t ret = pwrite64(fd, buffer, size, (off64_t)address);
    close(fd);
    return ret == (ssize_t)size;
}
bool ArmMemMemory::writeMemory(uintptr_t address, void* buffer, size_t size) {
    return writeMemory(getpid(), address, buffer, size);
}

bool ArmMemMemory::writeDword(int pid, uintptr_t address, int value) {
    return writeMemory(pid, address, &value, sizeof(int));
}

bool ArmMemMemory::writeFloat(int pid, uintptr_t address, float value) {
    return writeMemory(pid, address, &value, sizeof(float));
}

bool ArmMemMemory::writeDouble(int pid, uintptr_t address, double value) {
    return writeMemory(pid, address, &value, sizeof(double));
}

bool ArmMemMemory::writeQword(int pid, uintptr_t address, long long value) {
    return writeMemory(pid, address, &value, sizeof(long long));
}

bool ArmMemMemory::writeDword(uintptr_t address, int value) {
    return writeMemory(getpid(), address, &value, sizeof(int));
}

bool ArmMemMemory::writeFloat(uintptr_t address, float value) {
    return writeMemory(getpid(), address, &value, sizeof(float));
}

bool ArmMemMemory::writeDouble(uintptr_t address, double value) {
    return writeMemory(getpid(), address, &value, sizeof(double));
}

bool ArmMemMemory::writeQword(uintptr_t address, long long value) {
    return writeMemory(getpid(), address, &value, sizeof(long long));
}

/*
 * --------------------------------
 * Memory Read
 * --------------------------------
 */

int ArmMemMemory::readDword(uintptr_t address, int fd, bool *success) {
    int value = 0;
    ssize_t ret = pread64(fd, &value, sizeof(int), (off64_t)address);
    if (success) *success = (ret == sizeof(int));
    return value;
}

float ArmMemMemory::readFloat(uintptr_t address, int fd, bool *success) {
    float value = 0.0f;
    ssize_t ret = pread64(fd, &value, sizeof(float), (off64_t)address);
    if (success) *success = (ret == sizeof(float));
    return value;
}

double ArmMemMemory::readDouble(uintptr_t address, int fd, bool *success) {
    double value = 0.0;
    ssize_t ret = pread64(fd, &value, sizeof(double), (off64_t)address);
    if (success) *success = (ret == sizeof(double));
    return value;
}

long long ArmMemMemory::readQword(uintptr_t address, int fd, bool *success) {
    long long value = 0;
    ssize_t ret = pread64(fd, &value, sizeof(long long), (off64_t)address);
    if (success) *success = (ret == sizeof(long long));
    return value;
}

int ArmMemMemory::readDword(int pid, uintptr_t address, bool *success) {
    int fd = openMemFile(pid);
    if (fd == -1) {
        if (success) *success = false;
        return 0;
    }
    int result = readDword(address, fd, success);
    close(fd);
    return result;
}

float ArmMemMemory::readFloat(int pid, uintptr_t address, bool *success) {
    int fd = openMemFile(pid);
    if (fd == -1) {
        if (success) *success = false;
        return 0.0f;
    }
    float result = readFloat(address, fd, success);
    close(fd);
    return result;
}

double ArmMemMemory::readDouble(int pid, uintptr_t address, bool *success) {
    int fd = openMemFile(pid);
    if (fd == -1) {
        if (success) *success = false;
        return 0.0;
    }
    double result = readDouble(address, fd, success);
    close(fd);
    return result;
}

long long ArmMemMemory::readQword(int pid, uintptr_t address, bool *success) {
    int fd = openMemFile(pid);
    if (fd == -1) {
        if (success) *success = false;
        return 0;
    }
    long long result = readQword(address, fd, success);
    close(fd);
    return result;
}

int ArmMemMemory::readDword(uintptr_t address, bool *success) {
    return readDword(getpid(), address, success);
}

float ArmMemMemory::readFloat(uintptr_t address, bool *success) {
    return readFloat(getpid(), address, success);
}

double ArmMemMemory::readDouble(uintptr_t address, bool *success) {
    return readDouble(getpid(), address, success);
}

long long ArmMemMemory::readQword(uintptr_t address, bool *success) {
    return readQword(getpid(), address, success);
}

int ArmMemMemory::openMemFile(int pid) {
    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    return open(memPath, O_RDONLY);
}

/*
 * --------------------------------
 * Memory Monitor
 * --------------------------------
 */

std::mutex ArmMemMemory::m_monitorMutex;
std::unordered_map<int, std::shared_ptr<MemoryMonitorHandle*>> ArmMemMemory::m_monitorHandles;

MemoryMonitorHandle* ArmMemMemory::listenForWrite(int pid, uintptr_t address, void *callback, void *userData) {
    return listen(pid, address, 0, callback, userData);
}

MemoryMonitorHandle* ArmMemMemory::listen(int pid, uintptr_t address, int type, void *callback, void *userData) {
    if (!address) {
        ArmMem::logE(TAG, __func__, "Invalid address");
        return nullptr;
    }

    for (auto& it : m_monitorHandles) {
        if ((*it.second)->address == address) {
            ArmMem::logE(TAG, __func__, "Address already listened");
            return nullptr;
        }
    }

    std::lock_guard<std::mutex> lock(m_monitorMutex);

    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist_val(100000000, 999999999);
    std::uniform_int_distribution<int> dist_sign(0, 1);
    int hash = dist_val(gen);
    if (dist_sign(gen) == 0) {
        hash = -hash;
    }

    auto* handle = new MemoryMonitorHandle();
    handle->pid = pid;
    handle->address = address;
    handle->size = 4;
    handle->isOnce = false;
    handle->userData = userData;
    handle->callback = reinterpret_cast<void* (*)(MemoryMonitorHandle*, MemoryMonitorHit*)>(callback);
    handle->type = type;
    handle->hash = hash;
    m_monitorHandles[handle->hash] = std::make_shared<MemoryMonitorHandle*>(handle);

    _listenForWrite(handle);
    ArmMem::logV(TAG, __func__, "Listened %i[%p] for %s", handle->hash, handle->address, type == 0 ? "WRITE" : "READ");
    return handle;
}

MemoryMonitorHandle* ArmMemMemory::_listenForWrite(MemoryMonitorHandle* handle) {
    struct sigaction sa{};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    sa.sa_sigaction = syncMonitorSignalHandler;
    sigaction(SIGSEGV, &sa, nullptr);

    long pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = handle->address & ~(pageSize - 1);
    mprotect(reinterpret_cast<void *>(pageStart), pageSize, PROT_READ);

    return handle;
}

bool ArmMemMemory::unlisten(MemoryMonitorHandle* handle) {
    if (!handle) {
        ArmMem::logE(TAG, __func__, "Invalid handle");
        return false;
    }
    std::lock_guard<std::mutex> lock(m_monitorMutex);

    auto it = m_monitorHandles.find(handle->hash);
    if (it != m_monitorHandles.end()) {
        uintptr_t targetAddr = handle->address;
        m_monitorHandles.erase(it);
        _updatePageProtection(targetAddr);
        ArmMem::logV(TAG, __func__, "Unlistened %i[%p]", handle->hash, targetAddr);
        return true;
    }
    ArmMem::logE(TAG, __func__, "Invalid handle");
    return false;
}

MemoryMonitorHandle* ArmMemMemory::listenForWriteOnce(int pid, uintptr_t address, void *callback, void *userData) {
    MemoryMonitorHandle* handle = listenForWrite(pid, address, callback, userData);
    if (handle) {
        handle->isOnce = true;
    }
    return handle;
}

void ArmMemMemory::syncMonitorSignalHandler(int sig, siginfo_t* si, void* context) {
    if (sig != SIGSEGV) return;

    auto faultAddr = reinterpret_cast<uintptr_t>(si->si_addr);
    long pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = faultAddr & ~(pageSize - 1);

    auto* ucontext = reinterpret_cast<ucontext_t*>(context);
#ifdef __aarch64__
    uintptr_t pc = ucontext->uc_mcontext.pc;
#elif __arm__
    uintptr_t pc = ucontext->uc_mcontext.arm_pc;
#endif

    mprotect(reinterpret_cast<void*>(pageStart), pageSize, PROT_READ | PROT_WRITE);

    bool modified = false;
    if (m_monitorMutex.try_lock()) {
        std::vector<MemoryMonitorHandle*> toRemove;

        for (auto& pair : m_monitorHandles) {
            MemoryMonitorHandle* target = *pair.second;

            if (faultAddr >= target->address && faultAddr < (target->address + target->size)) {
                uint32_t originalVal = *reinterpret_cast<uint32_t*>(target->address);

                const char* moduleName = nullptr;
                const char* symbolName = nullptr;
                uintptr_t accessorFunction = 0;
                uintptr_t accessorModuleBase = 0;
                Dl_info info;
                auto* hit = new MemoryMonitorHit();
                if (dladdr(reinterpret_cast<void*>(pc), &info)) {
                    moduleName = info.dli_fname;
                    symbolName = info.dli_sname;
                    accessorFunction = reinterpret_cast<uintptr_t>(info.dli_saddr);
                    accessorModuleBase = reinterpret_cast<uintptr_t>(info.dli_fbase);
                }

                hit->originalValue = originalVal;
                hit->accessorAddress = pc;
                hit->accessorSymbol = symbolName;
                hit->accessorModuleName = moduleName;
                hit->accessorFunction = accessorFunction;
                hit->accessorModuleBase = accessorModuleBase;

                if (target->callback) {
                    void *result = target->callback(target, hit);
                    if (target->type == 0 && result != nullptr) {
                        *reinterpret_cast<uint32_t *>(target->address) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(result));
                        ArmMem::logV(TAG, __func__, "Monitor hit %s %i[%p], replaced to %i", (target->type == 0 ? "WRITE" : "READ"), target->hash, target->address, result);
                    } else {
                        ArmMem::logV(TAG, __func__, "Monitor hit %s %i[%p]", (target->type == 0 ? "WRITE" : "READ"), target->hash, target->address);
                    }
                    modified = true;
                }

                if (target->isOnce) {
                    toRemove.push_back(target);
                } else {
                    std::thread([target]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        std::lock_guard<std::mutex> lock(m_monitorMutex);
                        _updatePageProtection(target->address);
                    }).detach();
                }
                break;
            }
        }

        for (auto* target : toRemove) m_monitorHandles.erase(target->hash);
        m_monitorMutex.unlock();
    }
    if (modified) {
#ifdef __aarch64__
        ucontext->uc_mcontext.pc += 4;
#elif __arm__
        ucontext->uc_mcontext.arm_pc += (ucontext->uc_mcontext.arm_cpsr & 0x20) ? 2 : 4;
#endif
    }
}

void ArmMemMemory::_updatePageProtection(uintptr_t address) {
    long pageSize = sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = address & ~(pageSize - 1);

    bool hasReadMonitor = false;
    bool hasWriteMonitor = false;

    for (auto& pair : m_monitorHandles) {
        MemoryMonitorHandle* h = *pair.second;
        if ((h->address & ~(pageSize - 1)) == pageStart) {
            if (h->type == 1) hasReadMonitor = true;
            if (h->type == 0) hasWriteMonitor = true;
        }
    }

    int prot = PROT_READ | PROT_WRITE;
    if (hasReadMonitor) {
        prot = PROT_NONE;
    } else if (hasWriteMonitor) {
        prot = PROT_READ;
    }

    mprotect(reinterpret_cast<void *>(pageStart), pageSize, prot);
}

MemoryMonitorHandle* ArmMemMemory::listenForRead(int pid, uintptr_t address, void *callback, void *userData) {
    MemoryMonitorHandle* handle = listen(pid, address, 1, callback, userData);
    if (handle) {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        _updatePageProtection(handle->address);
    }
    return handle;
}

MemoryMonitorHandle* ArmMemMemory::listenForReadOnce(int pid, uintptr_t address, void *callback, void *userData) {
    MemoryMonitorHandle* handle = listenForRead(pid, address, callback, userData);
    if (handle) {
        handle->isOnce = true;
    }
    return handle;
}

MemoryMonitorHandle* ArmMemMemory::listenForWrite(uintptr_t address, void *callback, void *userData) {
    return listenForWrite(getpid(), address, callback, userData);
}
MemoryMonitorHandle* ArmMemMemory::listenForWriteOnce(uintptr_t address, void *callback, void *userData) {
    return listenForWriteOnce(getpid(), address, callback, userData);
}
MemoryMonitorHandle* ArmMemMemory::listenForRead(uintptr_t address, void *callback, void *userData) {
    return listenForRead(getpid(), address, callback, userData);
}
MemoryMonitorHandle* ArmMemMemory::listenForReadOnce(uintptr_t address, void *callback, void *userData) {
    return listenForReadOnce(getpid(), address, callback, userData);
}
