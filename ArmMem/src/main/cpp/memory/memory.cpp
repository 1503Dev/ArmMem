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
#include <random>
#include <atomic>
#include <dlfcn.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>

std::string ArmMemMemory::s_lastSearchError;

struct PendingMonitorEvent {
    uintptr_t faultAddr;
    uintptr_t pc;
    uintptr_t pageStart;
    bool isWrite;
    bool isHit;
};

static const long kPageSize = sysconf(_SC_PAGESIZE);

static constexpr size_t kMaxPendingEvents = 64;
static PendingMonitorEvent g_pendingEvents[kMaxPendingEvents];
static std::atomic<size_t> g_pendingHead{0};
static std::atomic<size_t> g_pendingTail{0};
static std::atomic_flag g_workerRunning = ATOMIC_FLAG_INIT;
static std::atomic<bool> g_needRelock{false};

enum { MONITOR_PAGE_WRITE = 1, MONITOR_PAGE_READ = 2 };
static constexpr size_t kMaxMonitoredPages = 256;
static constexpr size_t kMaxMonitoredAddrs = 256;
struct MonitoredPage {
    std::atomic<uintptr_t> start{0};
    std::atomic<int> type{0};
};
static MonitoredPage g_monitoredPages[kMaxMonitoredPages];
static std::atomic<uintptr_t> g_monitoredAddrs[kMaxMonitoredAddrs];

struct PageProtState {
    uintptr_t pageStart;
    int prot;
};
static std::vector<PageProtState> g_pageProtStates;

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
            strncpy(reg.path, has_path ? path : "anonymous", sizeof(reg.path) - 1);
            reg.path[sizeof(reg.path) - 1] = '\0';
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
 * Signature Search
 * --------------------------------
 */

static bool parseSignature(const char* pattern, std::vector<uint8_t>& bytes, std::vector<bool>& mask) {
    bytes.clear();
    mask.clear();
    const char* p = pattern;

    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;

        if (p[0] == '?' && p[1] == '?') {
            bytes.push_back(0);
            mask.push_back(false);
            p += 2;
        } else {
            char hex[3] = {p[0], p[1], '\0'};
            bytes.push_back((uint8_t)strtoul(hex, nullptr, 16));
            mask.push_back(true);
            p += 2;
        }
    }
    return !bytes.empty();
}

static bool matchAt(const uint8_t* data, const std::vector<uint8_t>& pattern, const std::vector<bool>& mask) {
    for (size_t i = 0; i < pattern.size(); i++) {
        if (mask[i] && data[i] != pattern[i]) return false;
    }
    return true;
}

std::vector<uintptr_t> ArmMemMemory::searchSignature(int pid, const char* pattern, MemoryRange memoryRange) {
    std::vector<uint8_t> bytes;
    std::vector<bool> mask;
    if (!parseSignature(pattern, bytes, mask)) return {};

    std::vector<MemoryRegion> regions = getMemoryRegions(pid, memoryRange);
    std::vector<uintptr_t> results;
    if (regions.empty()) return results;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return results;

    const size_t CHUNK_SIZE = 1024 * 1024;
    std::vector<uint8_t> buffer(CHUNK_SIZE);
    size_t patLen = bytes.size();

    for (const auto& region : regions) {
        uintptr_t currentAddr = region.start;
        uintptr_t endAddr = region.start + region.size;

        while (currentAddr + patLen <= endAddr) {
            size_t remaining = endAddr - currentAddr;
            size_t bytesToRead = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;

            ssize_t ret = pread64(fd, buffer.data(), bytesToRead, (off64_t)currentAddr);
            if (ret <= 0) break;

            size_t searchEnd = (ret >= (ssize_t)patLen) ? ret - patLen + 1 : 0;
            for (size_t i = 0; i < searchEnd; i++) {
                if (matchAt(buffer.data() + i, bytes, mask)) {
                    results.push_back(currentAddr + i);
                }
            }

            if ((size_t)ret == bytesToRead) {
                currentAddr += bytesToRead - patLen + 1;
            } else {
                break;
            }
        }
    }

    close(fd);
    return results;
}

std::vector<uintptr_t> ArmMemMemory::searchSignature(const char* pattern, MemoryRange memoryRange) {
    return searchSignature(getpid(), pattern, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchSignature(int pid, const char* pattern, const std::vector<uintptr_t>& prevList) {
    std::vector<uint8_t> bytes;
    std::vector<bool> mask;
    if (!parseSignature(pattern, bytes, mask)) return {};

    std::vector<uintptr_t> results;
    if (prevList.empty()) return results;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return results;

    size_t patLen = bytes.size();
    std::vector<uint8_t> buffer(patLen);

    for (auto addr : prevList) {
        ssize_t ret = pread64(fd, buffer.data(), patLen, (off64_t)addr);
        if (ret == (ssize_t)patLen && matchAt(buffer.data(), bytes, mask)) {
            results.push_back(addr);
        }
    }

    close(fd);
    return results;
}

std::vector<uintptr_t> ArmMemMemory::searchSignature(const char* pattern, const std::vector<uintptr_t>& prevList) {
    return searchSignature(getpid(), pattern, prevList);
}

/*
 * --------------------------------
 * Combined Search
 * --------------------------------
 */

enum SearchValueType { AUTO, DWORD, FLOAT, WORD, BYTE, DOUBLE_T };

struct SearchValue {
    SearchValueType type;
    union {
        int intVal;
        float floatVal;
        short wordVal;
        uint8_t byteVal;
        double doubleVal;
    };
    float floatRadius = 0.0f;
    double doubleRadius = 0.0;
    size_t size() const {
        switch (type) {
            case DWORD: return 4;
            case FLOAT: return 4;
            case WORD: return 2;
            case BYTE: return 1;
            case DOUBLE_T: return 8;
            default: return 4;
        }
    }
};

struct SearchItem {
    size_t gap = 0;
    SearchValue value;
};

struct SearchGroup {
    std::vector<SearchItem> items;
    int span = 0;
    size_t width = 0;
};

static SearchValueType detectType(const char* s) {
    char* end = nullptr;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        strtol(s, &end, 16);
        return (*end == '\0') ? DWORD : AUTO;
    }
    strtol(s, &end, 10);
    if (*end == '\0') return DWORD;
    strtod(s, &end);
    if (*end == '\0') return FLOAT;
    return AUTO;
}

static bool parseValueToken(const char* s, SearchValue& out) {
    while (*s == ' ') s++;
    if (*s == '\0') return false;

    if (s[0] == 'd' && s[1] == ':' && s[2] == ':') { out.type = DWORD; s += 3; }
    else if (s[0] == 'f' && s[1] == ':' && s[2] == ':') { out.type = FLOAT; s += 3; }
    else if (s[0] == 'w' && s[1] == ':' && s[2] == ':') { out.type = WORD; s += 3; }
    else if (s[0] == 'b' && s[1] == ':' && s[2] == ':') { out.type = BYTE; s += 3; }
    else if (s[0] == 'e' && s[1] == ':' && s[2] == ':') { out.type = DOUBLE_T; s += 3; }
    else { out.type = AUTO; }

    while (*s == ' ') s++;
    if (*s == '\0') return false;

    float floatRadius = 0.0f;
    double doubleRadius = 0.0;
    const char* tilde = strchr(s, '~');
    std::string numPart(s);
    if (tilde != nullptr) {
        if (out.type == DWORD || out.type == WORD || out.type == BYTE) return false; // 显式整数类型不支持半径
        numPart = std::string(s, tilde - s);
        char* rEnd = nullptr;
        double r = strtod(tilde + 1, &rEnd);
        while (*rEnd == ' ') rEnd++;
        if (rEnd == tilde + 1 || *rEnd != '\0') return false;
        floatRadius = (float)r;
        doubleRadius = r;
    }
    while (!numPart.empty() && numPart.back() == ' ') numPart.pop_back();
    if (numPart.empty()) return false;

    if (out.type == AUTO) {
        out.type = detectType(numPart.c_str());
        if (out.type == AUTO) return false; // 无法识别的 token, 拒绝而非静默当作 0
        if (tilde != nullptr && out.type != FLOAT && out.type != DOUBLE_T) return false; // 半径要求浮点/双精度
    }

    char* end = nullptr;
    const char* num = numPart.c_str();
    while (*num == ' ') num++;
    switch (out.type) {
        case DWORD: {
            int base = 10;
            if (num[0] == '0' && (num[1] == 'x' || num[1] == 'X')) base = 16;
            long v = strtol(num, &end, base);
            while (*end == ' ') end++;
            if (end == num || *end != '\0') return false;
            out.intVal = (int)v;
            break;
        }
        case FLOAT: {
            float v = strtof(num, &end);
            while (*end == ' ') end++;
            if (end == num || *end != '\0') return false;
            out.floatVal = v;
            out.floatRadius = floatRadius;
            break;
        }
        case WORD: {
            short v = (short)strtol(num, &end, 10);
            while (*end == ' ') end++;
            if (end == num || *end != '\0') return false;
            out.wordVal = v;
            break;
        }
        case BYTE: {
            int base = 10;
            if (num[0] == '0' && (num[1] == 'x' || num[1] == 'X')) base = 16;
            long v = strtol(num, &end, base);
            while (*end == ' ') end++;
            if (end == num || *end != '\0') return false;
            out.byteVal = (uint8_t)v;
            break;
        }
        case DOUBLE_T: {
            double v = strtod(num, &end);
            while (*end == ' ') end++;
            if (end == num || *end != '\0') return false;
            out.doubleVal = v;
            out.doubleRadius = doubleRadius;
            break;
        }
        default: return false;
    }
    return true;
}

static bool parseGroupValues(const std::string& groupStr, SearchGroup& group) {
    size_t len = groupStr.size();
    size_t i = 0;
    size_t pendingGap = 0;
    bool expectValue = true;

    while (i < len) {
        if (expectValue) {
            size_t j = i;
            while (j < len) {
                char c = groupStr[j];
                if (c == ';') break;
                if (c == ':') {
                    size_t k = j + 1;
                    if (k < len && isdigit((unsigned char)groupStr[k])) {
                        size_t m = k;
                        while (m < len && isdigit((unsigned char)groupStr[m])) m++;
                        if (m < len && groupStr[m] == ':') break; // ":N:" 间隔分隔符
                    }
                }
                j++;
            }
            std::string tok = groupStr.substr(i, j - i);
            while (!tok.empty() && tok.back() == ' ') tok.pop_back();
            if (tok.empty()) return false;
            SearchItem item;
            item.gap = pendingGap;
            pendingGap = 0;
            if (!parseValueToken(tok.c_str(), item.value)) return false;
            group.items.push_back(item);
            i = j;
            expectValue = false;
        } else {
            if (i >= len) break;
            if (groupStr[i] == ';') { i++; expectValue = true; }
            else if (groupStr[i] == ':') {
                size_t k = i + 1;
                size_t m = k;
                while (m < len && isdigit((unsigned char)groupStr[m])) m++;
                if (m >= len || m == k || groupStr[m] != ':') return false; // 悬空 ':' 或空间隔
                pendingGap += (size_t)strtoul(groupStr.c_str() + k, nullptr, 10);
                i = m + 1;
                expectValue = true;
            } else return false;
        }
    }
    return !group.items.empty() && !expectValue;
}

static bool parseExpression(const char* expr, std::vector<SearchGroup>& groups, std::string& error) {
    groups.clear();
    error.clear();
    if (expr == nullptr || *expr == '\0') { error = "empty expression"; return false; }

    std::string input(expr);
    size_t pos = 0;
    while (pos < input.size()) {
        size_t groupEnd = input.find("||", pos);
        if (groupEnd == std::string::npos) groupEnd = input.size();

        std::string groupStr = input.substr(pos, groupEnd - pos);
        pos = groupEnd + 2;

        size_t b = groupStr.find_first_not_of(' ');
        size_t e = groupStr.find_last_not_of(' ');
        if (b == std::string::npos) { error = "empty group in expression: " + input; return false; }
        groupStr = groupStr.substr(b, e - b + 1);

        SearchGroup group;

        if (std::count(groupStr.begin(), groupStr.end(), ':') == 1) {
            size_t p = groupStr.rfind(':');
            if (p > 0 && p + 1 < groupStr.size()) {
                bool digitsOnly = true;
                for (size_t k = p + 1; k < groupStr.size(); k++) {
                    if (!isdigit((unsigned char)groupStr[k]) && groupStr[k] != ' ') { digitsOnly = false; break; }
                }
                if (digitsOnly) {
                    long spanVal = strtol(groupStr.c_str() + p + 1, nullptr, 10);
                    if (spanVal < 0 || spanVal > 0x7FFFFFFF) { error = "invalid span in expression: " + input; return false; }
                    group.span = (int)spanVal;
                    groupStr = groupStr.substr(0, p);
                    while (!groupStr.empty() && groupStr.back() == ' ') groupStr.pop_back();
                }
            }
        }
        if (groupStr.empty()) { error = "empty group in expression: " + input; return false; }

        if (!parseGroupValues(groupStr, group)) {
            error = "invalid group \"" + groupStr + "\" in expression: " + input;
            return false;
        }

        size_t width = 0;
        for (const auto& item : group.items) width += item.gap + item.value.size();
        group.width = width;

        if (group.span > 0 && width > (size_t)group.span) {
            error = "group \"" + groupStr + "\" wider than its span " + std::to_string(group.span);
            return false;
        }
        groups.push_back(group);
    }
    if (groups.empty()) { error = "empty expression"; return false; }
    return true;
}

static bool matchValueAtBuffer(const uint8_t* buf, size_t offset, const SearchValue& val) {
    switch (val.type) {
        case DWORD: { int v = 0; memcpy(&v, buf + offset, sizeof(v)); return v == val.intVal; }
        case FLOAT: { float v = 0; memcpy(&v, buf + offset, sizeof(v)); return std::abs(v - val.floatVal) <= val.floatRadius; }
        case WORD: { short v = 0; memcpy(&v, buf + offset, sizeof(v)); return v == val.wordVal; }
        case BYTE: { return buf[offset] == val.byteVal; }
        case DOUBLE_T: { double v = 0; memcpy(&v, buf + offset, sizeof(v)); return std::abs(v - val.doubleVal) <= val.doubleRadius; }
        default: return false;
    }
}

static bool matchGroupAtBuffer(const uint8_t* buf, size_t bufLen, size_t offset, const SearchGroup& group) {
    if (offset + group.width > bufLen) return false;
    size_t cursor = offset;
    for (const auto& item : group.items) {
        cursor += item.gap;
        if (cursor + item.value.size() > bufLen) return false;
        if (!matchValueAtBuffer(buf, cursor, item.value)) return false;
        cursor += item.value.size();
    }
    if (group.span > 0 && cursor - offset > (size_t)group.span) return false;
    return true;
}

static bool matchValueAtAddress(int fd, uintptr_t addr, const SearchValue& val) {
    switch (val.type) {
        case DWORD: { int v = 0; return pread64(fd, &v, 4, (off64_t)addr) == 4 && v == val.intVal; }
        case FLOAT: { float v = 0; return pread64(fd, &v, 4, (off64_t)addr) == 4 && std::abs(v - val.floatVal) <= val.floatRadius; }
        case WORD: { short v = 0; return pread64(fd, &v, 2, (off64_t)addr) == 2 && v == val.wordVal; }
        case BYTE: { uint8_t v = 0; return pread64(fd, &v, 1, (off64_t)addr) == 1 && v == val.byteVal; }
        case DOUBLE_T: { double v = 0; return pread64(fd, &v, 8, (off64_t)addr) == 8 && std::abs(v - val.doubleVal) <= val.doubleRadius; }
        default: return false;
    }
}

static bool matchGroupAtAddress(int fd, uintptr_t baseAddr, const SearchGroup& group) {
    uintptr_t cursor = baseAddr;
    for (const auto& item : group.items) {
        cursor += item.gap;
        if (!matchValueAtAddress(fd, cursor, item.value)) return false;
        cursor += item.value.size();
    }
    if (group.span > 0 && cursor - baseAddr > (size_t)group.span) return false;
    return true;
}

static std::vector<MemoryRegion> readAllRegions(int pid) {
    std::vector<MemoryRegion> regions;
    char mapPath[128];
    snprintf(mapPath, sizeof(mapPath), "/proc/%d/maps", pid);
    FILE* fp = fopen(mapPath, "r");
    if (!fp) return regions;

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        unsigned long s = 0, e = 0;
        char perms[5];
        char path[256] = {0};
        int res = sscanf(line, "%lx-%lx %4s %*s %*s %*s %s", &s, &e, perms, path);
        if (res < 3) continue;
        MemoryRegion reg;
        reg.start = (uintptr_t)s;
        reg.size = (uintptr_t)(e - s);
        strncpy(reg.path, strlen(path) > 0 ? path : "anonymous", sizeof(reg.path) - 1);
        reg.path[sizeof(reg.path) - 1] = '\0';
        regions.push_back(reg);
    }
    fclose(fp);
    return regions;
}

static std::vector<uintptr_t> scanAll(int fd, const std::vector<MemoryRegion>& regions, const SearchGroup& group) {
    std::vector<uintptr_t> matches;
    const size_t CHUNK = 1024 * 1024;
    std::vector<uint8_t> buffer(CHUNK);
    size_t maxWidth = group.width;
    size_t step = group.items[0].value.size();
    if (step == 0) step = 1;
    size_t overlap = ((maxWidth + step - 1) / step) * step;
    if (overlap > CHUNK / 2) overlap = (CHUNK / 2 / step) * step;
    size_t advance = CHUNK - overlap;

    for (const auto& region : regions) {
        uintptr_t addr = region.start;
        uintptr_t end = region.start + region.size;
        while (addr < end) {
            size_t remaining = (size_t)(end - addr);
            size_t toRead = (remaining > CHUNK) ? CHUNK : remaining;
            ssize_t ret = pread64(fd, buffer.data(), toRead, (off64_t)addr);
            if (ret <= 0) break;
            size_t len = (size_t)ret;
            if (len >= maxWidth) {
                size_t scanEnd = len - maxWidth + 1;
                for (size_t o = 0; o < scanEnd; o += step) {
                    if (matchGroupAtBuffer(buffer.data(), len, o, group)) matches.push_back(addr + o);
                }
            }
            addr += (ret < (ssize_t)toRead) ? (uintptr_t)ret : advance;
        }
    }
    return matches;
}

std::vector<uintptr_t> ArmMemMemory::search(int pid, const char* expression, MemoryRange memoryRange) {
    std::vector<SearchGroup> groups;
    std::string error;
    if (!parseExpression(expression, groups, error)) {
        s_lastSearchError = error;
        __android_log_print(ANDROID_LOG_ERROR, TAG, "search: %s", error.c_str());
        return {};
    }
    s_lastSearchError.clear();

    std::vector<MemoryRegion> regions = getMemoryRegions(pid, memoryRange);
    if (regions.empty()) return {};

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return {};

    std::vector<uintptr_t> matches = scanAll(fd, regions, groups[0]);
    for (size_t g = 1; g < groups.size() && !matches.empty(); g++) {
        std::vector<uintptr_t> matchesG = scanAll(fd, regions, groups[g]);
        if (matchesG.empty()) { matches.clear(); break; }
        uintptr_t prevWidth = groups[g - 1].width;
        std::vector<uintptr_t> kept;
        kept.reserve(matches.size());
        for (uintptr_t base : matches) {
            uintptr_t prevEnd = base + prevWidth;
            auto it = std::lower_bound(matchesG.begin(), matchesG.end(), prevEnd);
            if (it != matchesG.end()) kept.push_back(base);
        }
        matches = std::move(kept);
    }

    close(fd);
    return matches;
}

std::vector<uintptr_t> ArmMemMemory::search(const char* expression, MemoryRange memoryRange) {
    return search(getpid(), expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchDword(int pid, const char* expression, MemoryRange memoryRange) {
    return search(pid, expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchDword(const char* expression, MemoryRange memoryRange) {
    return search(getpid(), expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchFloat(int pid, const char* expression, MemoryRange memoryRange) {
    return search(pid, expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchFloat(const char* expression, MemoryRange memoryRange) {
    return search(getpid(), expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchWord(int pid, const char* expression, MemoryRange memoryRange) {
    return search(pid, expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchWord(const char* expression, MemoryRange memoryRange) {
    return search(getpid(), expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchByte(int pid, const char* expression, MemoryRange memoryRange) {
    return search(pid, expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchByte(const char* expression, MemoryRange memoryRange) {
    return search(getpid(), expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchDouble(int pid, const char* expression, MemoryRange memoryRange) {
    return search(pid, expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::searchDouble(const char* expression, MemoryRange memoryRange) {
    return search(getpid(), expression, memoryRange);
}

std::vector<uintptr_t> ArmMemMemory::search(int pid, const char* expression, const std::vector<uintptr_t>& prevList) {
    std::vector<SearchGroup> groups;
    std::string error;
    if (!parseExpression(expression, groups, error)) {
        s_lastSearchError = error;
        __android_log_print(ANDROID_LOG_ERROR, TAG, "search: %s", error.c_str());
        return {};
    }
    s_lastSearchError.clear();
    if (prevList.empty()) return {};

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) return {};

    std::vector<uintptr_t> matches;
    for (uintptr_t addr : prevList) {
        if (matchGroupAtAddress(fd, addr, groups[0])) matches.push_back(addr);
    }

    if (groups.size() > 1 && !matches.empty()) {
        std::vector<MemoryRegion> allRegions = readAllRegions(pid);
        if (allRegions.empty()) { close(fd); return {}; }
        std::vector<MemoryRegion> regions;
        for (const auto& region : allRegions) {
            for (uintptr_t addr : matches) {
                if (addr >= region.start && addr - region.start < region.size) { regions.push_back(region); break; }
            }
        }
        if (regions.empty()) { close(fd); return {}; }
        for (size_t g = 1; g < groups.size() && !matches.empty(); g++) {
            std::vector<uintptr_t> matchesG = scanAll(fd, regions, groups[g]);
            if (matchesG.empty()) { matches.clear(); break; }
            uintptr_t prevWidth = groups[g - 1].width;
            std::vector<uintptr_t> kept;
            kept.reserve(matches.size());
            for (uintptr_t base : matches) {
                uintptr_t prevEnd = base + prevWidth;
                auto it = std::lower_bound(matchesG.begin(), matchesG.end(), prevEnd);
                if (it != matchesG.end()) kept.push_back(base);
            }
            matches = std::move(kept);
        }
    }

    close(fd);
    return matches;
}

std::vector<uintptr_t> ArmMemMemory::search(const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(getpid(), expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchDword(int pid, const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(pid, expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchDword(const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(getpid(), expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchFloat(int pid, const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(pid, expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchFloat(const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(getpid(), expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchWord(int pid, const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(pid, expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchWord(const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(getpid(), expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchByte(int pid, const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(pid, expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchByte(const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(getpid(), expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchDouble(int pid, const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(pid, expression, prevList);
}

std::vector<uintptr_t> ArmMemMemory::searchDouble(const char* expression, const std::vector<uintptr_t>& prevList) {
    return search(getpid(), expression, prevList);
}

const char* ArmMemMemory::getLastSearchError() {
    return s_lastSearchError.c_str();
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
static struct sigaction g_oldSigaction{};
static bool g_sigactionInstalled = false;

MemoryMonitorHandle* ArmMemMemory::listenForWrite(int pid, uintptr_t address, void *callback, void *userData) {
    return listen(pid, address, 0, callback, userData);
}

static bool recordMonitorPage(uintptr_t address, int type) {
    uintptr_t pageStart = address & ~((uintptr_t)kPageSize - 1);
    int pageType = (type == 0) ? MONITOR_PAGE_WRITE : MONITOR_PAGE_READ;
    bool pageOk = false;
    bool addrOk = false;
    for (auto& p : g_monitoredPages) {
        uintptr_t start = p.start.load(std::memory_order_relaxed);
        if (start == pageStart) {
            p.type.store(p.type.load(std::memory_order_relaxed) | pageType, std::memory_order_relaxed);
            pageOk = true;
            break;
        }
        if (start == 0) {
            p.start.store(pageStart, std::memory_order_relaxed);
            p.type.store(pageType, std::memory_order_relaxed);
            pageOk = true;
            break;
        }
    }
    for (auto& a : g_monitoredAddrs) {
        uintptr_t v = a.load(std::memory_order_relaxed);
        if (v == address) { addrOk = true; break; }
        if (v == 0) { a.store(address, std::memory_order_relaxed); addrOk = true; break; }
    }
    return pageOk && addrOk;
}

static void recordOriginalProt(uintptr_t address) {
    uintptr_t pageStart = address & ~((uintptr_t)kPageSize - 1);
    for (const auto& st : g_pageProtStates) {
        if (st.pageStart == pageStart) return;
    }
    int prot = PROT_READ | PROT_WRITE;
    FILE* fp = fopen("/proc/self/maps", "r");
    if (fp) {
        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
            unsigned long s = 0, e = 0;
            char perms[5];
            if (sscanf(line, "%lx-%lx %4s", &s, &e, perms) < 3) continue;
            if (s <= pageStart && pageStart < e) {
                prot = 0;
                if (perms[0] == 'r') prot |= PROT_READ;
                if (perms[1] == 'w') prot |= PROT_WRITE;
                if (perms[2] == 'x') prot |= PROT_EXEC;
                if (prot == 0) prot = PROT_READ;
                break;
            }
        }
        fclose(fp);
    }
    g_pageProtStates.push_back({pageStart, prot});
}

MemoryMonitorHandle* ArmMemMemory::listen(int pid, uintptr_t address, int type, void *callback, void *userData) {
    if (!address) {
        ArmMem::logE(TAG, __func__, "Invalid address");
        return nullptr;
    }
    if (pid != getpid()) {
        ArmMem::logE(TAG, __func__, "Cross-process monitor is not supported (pid %d != %d)", pid, getpid());
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(m_monitorMutex);

    for (auto& it : m_monitorHandles) {
        if ((*it.second)->address == address) {
            ArmMem::logE(TAG, __func__, "Address already listened");
            return nullptr;
        }
    }
    if (m_monitorHandles.size() >= kMaxMonitoredAddrs) {
        ArmMem::logE(TAG, __func__, "Too many monitors");
        return nullptr;
    }

    static std::atomic<int> s_nextHash{100000000};
    int hash = s_nextHash.fetch_add(1);

    auto* handle = new MemoryMonitorHandle();
    handle->pid = pid;
    handle->address = address;
    handle->size = 4;
    handle->isOnce = false;
    handle->userData = userData;
    handle->callback = reinterpret_cast<void* (*)(MemoryMonitorHandle*, MemoryMonitorHit*)>(callback);
    handle->type = type;
    handle->hash = hash;

    recordOriginalProt(handle->address);
    if (!_listenForWrite(handle)) {
        delete handle;
        return nullptr;
    }
    m_monitorHandles[handle->hash] = std::make_shared<MemoryMonitorHandle*>(handle);
    if (!recordMonitorPage(handle->address, type)) {
        ArmMem::logE(TAG, __func__, "Monitor table full");
        m_monitorHandles.erase(handle->hash);
        delete handle;
        _updatePageProtection(address);
        return nullptr;
    }
    ArmMem::logV(TAG, __func__, "Listened %i[%p] for %s", handle->hash, handle->address, type == 0 ? "WRITE" : "READ");
    return handle;
}

bool ArmMemMemory::_listenForWrite(MemoryMonitorHandle* handle) {
    if (!g_sigactionInstalled) {
        struct sigaction sa{};
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO | SA_RESTART;
        sa.sa_sigaction = syncMonitorSignalHandler;
        sigaction(SIGSEGV, &sa, &g_oldSigaction);
        g_sigactionInstalled = true;
    }

    uintptr_t pageStart = handle->address & ~((uintptr_t)kPageSize - 1);
    if (mprotect(reinterpret_cast<void*>(pageStart), kPageSize, PROT_READ) != 0) {
        ArmMem::logE(TAG, __func__, "mprotect failed at %p", reinterpret_cast<void*>(pageStart));
        return false;
    }

    if (!g_workerRunning.test_and_set(std::memory_order_acquire)) {
        std::thread([]() { _processMonitorEvents(); }).detach();
    }
    return true;
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
        _unrecordMonitor(targetAddr);
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

void ArmMemMemory::_unrecordMonitor(uintptr_t address) {
    uintptr_t pageStart = address & ~((uintptr_t)kPageSize - 1);

    for (auto& a : g_monitoredAddrs) {
        if (a.load(std::memory_order_relaxed) == address) {
            a.store(0, std::memory_order_relaxed);
            break;
        }
    }

    int newType = 0;
    for (auto& pair : m_monitorHandles) {
        MemoryMonitorHandle* h = *pair.second;
        if ((h->address & ~((uintptr_t)kPageSize - 1)) == pageStart) {
            newType |= (h->type == 0) ? MONITOR_PAGE_WRITE : MONITOR_PAGE_READ;
        }
    }

    for (auto& p : g_monitoredPages) {
        if (p.start.load(std::memory_order_relaxed) == pageStart) {
            if (newType == 0) {
                p.start.store(0, std::memory_order_relaxed);
                p.type.store(0, std::memory_order_relaxed);
            } else {
                p.type.store(newType, std::memory_order_relaxed);
            }
            break;
        }
    }
}

static bool enqueueMonitorEvent(uintptr_t faultAddr, uintptr_t pc, uintptr_t pageStart, bool isHit) {
    size_t nextHead = g_pendingHead.load(std::memory_order_relaxed);
    size_t nextNextHead = (nextHead + 1) % kMaxPendingEvents;
    if (nextNextHead != g_pendingTail.load(std::memory_order_acquire)) {
        g_pendingEvents[nextHead] = {faultAddr, pc, pageStart, true, isHit};
        g_pendingHead.store(nextNextHead, std::memory_order_release);
        return true;
    }
    return false;
}

static void chainOldHandler(int sig, siginfo_t* si, void* context) {
    struct sigaction old = g_oldSigaction;
    if (old.sa_flags & SA_SIGINFO) {
        if (old.sa_sigaction) old.sa_sigaction(sig, si, context);
        return;
    }
    if (old.sa_handler == SIG_IGN) return;
    if (old.sa_handler == SIG_DFL) {
        signal(SIGSEGV, SIG_DFL);
        raise(SIGSEGV);
        return;
    }
    old.sa_handler(sig);
}

void ArmMemMemory::syncMonitorSignalHandler(int sig, siginfo_t* si, void* context) {
    if (sig != SIGSEGV) return;

    if (si->si_code != SEGV_ACCERR) {
        chainOldHandler(sig, si, context);
        return;
    }

    auto faultAddr = reinterpret_cast<uintptr_t>(si->si_addr);
    uintptr_t pageStart = faultAddr & ~((uintptr_t)kPageSize - 1);

    int pageType = 0;
    for (auto& p : g_monitoredPages) {
        uintptr_t start = p.start.load(std::memory_order_relaxed);
        if (start == pageStart) {
            pageType = p.type.load(std::memory_order_relaxed);
            break;
        }
    }
    if (pageType == 0) {
        chainOldHandler(sig, si, context);
        return;
    }

    bool isHit = false;
    for (auto& a : g_monitoredAddrs) {
        if (a.load(std::memory_order_relaxed) == faultAddr) {
            isHit = true;
            break;
        }
    }

    auto* ucontext = reinterpret_cast<ucontext_t*>(context);
#ifdef __aarch64__
    uintptr_t pc = ucontext->uc_mcontext.pc;
#elif __arm__
    uintptr_t pc = ucontext->uc_mcontext.arm_pc;
#endif

    bool skipInstruction = (pageType == MONITOR_PAGE_WRITE) && isHit;

    if (!enqueueMonitorEvent(faultAddr, pc, pageStart, isHit)) {
        g_needRelock.store(true, std::memory_order_release);
    }

    mprotect(reinterpret_cast<void*>(pageStart), kPageSize, PROT_READ | PROT_WRITE);

    if (skipInstruction) {
#ifdef __aarch64__
        ucontext->uc_mcontext.pc += 4;
#elif __arm__
        if (ucontext->uc_mcontext.arm_cpsr & 0x20) {
            uint32_t arm_pc = ucontext->uc_mcontext.arm_pc;
            uint16_t ins = *reinterpret_cast<uint16_t*>(arm_pc);
            ucontext->uc_mcontext.arm_pc += ((ins & 0xF800) >= 0xE800) ? 4 : 2; // Thumb-2 32 位指令按实际长度跳
        } else {
            ucontext->uc_mcontext.arm_pc += 4;
        }
#endif
    }
}

void ArmMemMemory::_processMonitorEvents() {
    while (true) {
        size_t head = g_pendingHead.load(std::memory_order_acquire);
        size_t tail = g_pendingTail.load(std::memory_order_relaxed);
        if (tail != head) {
            PendingMonitorEvent event = g_pendingEvents[tail];
            g_pendingTail.store((tail + 1) % kMaxPendingEvents, std::memory_order_release);
            _processMonitorEvent(event);
            continue;
        }
        if (g_needRelock.exchange(false, std::memory_order_acq_rel)) {
            _relockAllPages();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ArmMemMemory::_processMonitorEvent(const PendingMonitorEvent& event) {
    std::unique_lock<std::mutex> lock(m_monitorMutex);

    mprotect(reinterpret_cast<void*>(event.pageStart), kPageSize, PROT_READ | PROT_WRITE);

    MemoryMonitorHandle* matchedHandle = nullptr;
    if (event.isHit) {
        for (auto& pair : m_monitorHandles) {
            MemoryMonitorHandle* target = *pair.second;
            if (event.faultAddr >= target->address && event.faultAddr < (target->address + target->size)) {
                matchedHandle = target;
                break;
            }
        }
    }

    if (matchedHandle) {
        MemoryMonitorHandle info = *matchedHandle;
        uint32_t originalVal = *reinterpret_cast<uint32_t*>(info.address);

        const char* moduleName = nullptr;
        const char* symbolName = nullptr;
        uintptr_t accessorFunction = 0;
        uintptr_t accessorModuleBase = 0;
        Dl_info dlinfo;
        MemoryMonitorHit hit{};
        if (dladdr(reinterpret_cast<void*>(event.pc), &dlinfo)) {
            moduleName = dlinfo.dli_fname;
            symbolName = dlinfo.dli_sname;
            accessorFunction = reinterpret_cast<uintptr_t>(dlinfo.dli_saddr);
            accessorModuleBase = reinterpret_cast<uintptr_t>(dlinfo.dli_fbase);
        }

        hit.originalValue = originalVal;
        hit.accessorAddress = event.pc;
        hit.accessorSymbol = symbolName;
        hit.accessorModuleName = moduleName;
        hit.accessorFunction = accessorFunction;
        hit.accessorModuleBase = accessorModuleBase;

        lock.unlock();

        void* result = nullptr;
        if (info.callback) {
            result = info.callback(&info, &hit);
        }
        if (info.type == 0 && result != nullptr) {
            lock.lock();
            *reinterpret_cast<uint32_t*>(info.address) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(result));
            lock.unlock();
            ArmMem::logV(TAG, __func__, "Monitor hit %s %i[%p], replaced to %i", (info.type == 0 ? "WRITE" : "READ"), info.hash, info.address, result);
        } else {
            ArmMem::logV(TAG, __func__, "Monitor hit %s %i[%p]", (info.type == 0 ? "WRITE" : "READ"), info.hash, info.address);
        }

        if (info.isOnce) {
            lock.lock();
            auto it = m_monitorHandles.find(info.hash);
            if (it != m_monitorHandles.end()) {
                delete (*it->second);
                m_monitorHandles.erase(it);
                _unrecordMonitor(info.address);
            }
            lock.unlock();
        }
        lock.lock();
    }
    _updatePageProtection(event.pageStart);
}

void ArmMemMemory::_relockAllPages() {
    std::lock_guard<std::mutex> lock(m_monitorMutex);
    uintptr_t lastPage = 0;
    for (auto& pair : m_monitorHandles) {
        MemoryMonitorHandle* h = *pair.second;
        uintptr_t pageStart = h->address & ~((uintptr_t)kPageSize - 1);
        if (pageStart != lastPage) {
            _updatePageProtection(h->address);
            lastPage = pageStart;
        }
    }
}

void ArmMemMemory::_updatePageProtection(uintptr_t address) {
    uintptr_t pageStart = address & ~((uintptr_t)kPageSize - 1);

    bool hasReadMonitor = false;
    bool hasWriteMonitor = false;

    for (auto& pair : m_monitorHandles) {
        MemoryMonitorHandle* h = *pair.second;
        if ((h->address & ~((uintptr_t)kPageSize - 1)) == pageStart) {
            if (h->type == 1) hasReadMonitor = true;
            if (h->type == 0) hasWriteMonitor = true;
        }
    }

    if (!hasReadMonitor && !hasWriteMonitor) {
        int prot = PROT_READ | PROT_WRITE;
        for (auto it = g_pageProtStates.begin(); it != g_pageProtStates.end(); ++it) {
            if (it->pageStart == pageStart) {
                prot = it->prot;
                g_pageProtStates.erase(it);
                break;
            }
        }
        mprotect(reinterpret_cast<void*>(pageStart), kPageSize, prot);
        return;
    }

    int prot = PROT_READ | PROT_WRITE;
    if (hasReadMonitor) {
        prot = PROT_NONE;
    } else if (hasWriteMonitor) {
        prot = PROT_READ;
    }
    mprotect(reinterpret_cast<void*>(pageStart), kPageSize, prot);
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
