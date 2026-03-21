package dev1503.armmem.memory;

import static dev1503.armmem.memory.JNI.getPid;

public class Memory {
    public static final int RANGE_ALL = 0;
    public static final int RANGE_C_HEAP = 1;
    public static final int RANGE_JAVA_HEAP = 2;
    public static final int RANGE_C_ALLOC = 3;
    public static final int RANGE_C_DATA = 4;
    public static final int RANGE_C_BSS = 5;
    public static final int RANGE_ANONYMOUS = 6;
    public static final int RANGE_CODE_APP = 7;
    public static final int RANGE_STACK = 8;
    public static final int RANGE_ASHMEM = 9;
    public static final int RANGE_OTHER = 10;

    public static MemoryValueSet searchDword(int pid, int value, int memoryRange) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchDword(pid, value, memoryRange);
        for (long address : addresses) {
            memoryValueSet.addDword(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchDword(int value, int memoryRange) {
        return searchDword(getPid(), value, memoryRange);
    }

    public static MemoryValueSet searchDword(int pid, int value, long[] prevList) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchDword(pid, value, prevList);
        for (long address : addresses) {
            memoryValueSet.addDword(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchDword(int value, long[] prevList) {
        return searchDword(getPid(), value, prevList);
    }
    public static MemoryValueSet searchFloat(int pid, float value, float radius, int memoryRange) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchFloat(pid, value, radius, memoryRange);
        for (long address : addresses) {
            memoryValueSet.addFloat(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchFloat(float value, float radius, int memoryRange) {
        return searchFloat(getPid(), value, radius, memoryRange);
    }

    public static MemoryValueSet searchFloat(int pid, float value, float radius, long[] prevList) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchFloat(pid, value, radius, prevList);
        for (long address : addresses) {
            memoryValueSet.addFloat(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchFloat(float value, float radius, long[] prevList) {
        return searchFloat(getPid(), value, radius, prevList);
    }

    public static MemoryValueSet searchDouble(int pid, double value, double radius, int memoryRange) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchDouble(pid, value, radius, memoryRange);
        for (long address : addresses) {
            memoryValueSet.addDouble(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchDouble(double value, double radius, int memoryRange) {
        return searchDouble(getPid(), value, radius, memoryRange);
    }

    public static MemoryValueSet searchDouble(int pid, double value, double radius, long[] prevList) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchDouble(pid, value, radius, prevList);
        for (long address : addresses) {
            memoryValueSet.addDouble(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchDouble(double value, double radius, long[] prevList) {
        return searchDouble(getPid(), value, radius, prevList);
    }

    public static void writeDword(int pid, long address, int value) {
        JNI.writeDword(pid, address, value);
    }

    public static void writeDword(long address, int value) {
        JNI.writeDword(address, value);
    }

    public static void writeFloat(int pid, long address, float value) {
        JNI.writeFloat(pid, address, value);
    }

    public static void writeFloat(long address, float value) {
        JNI.writeFloat(address, value);
    }

    public static void writeDouble(int pid, long address, double value) {
        JNI.writeDouble(pid, address, value);
    }

    public static void writeDouble(long address, double value) {
        JNI.writeDouble(address, value);
    }

    public static int openMemFile(int pid) {
        return JNI.openMemFile(pid);
    }

    public static int openMemFile() {
        return JNI.openMemFile(getPid());
    }

    public static void closeMemFile(int fd) {
        JNI.closeMemFile(fd);
    }

    public static int readDword(int pid, long address) {
        return JNI.readDword(pid, address);
    }

    public static int readDword(long address, int fd) {
        return JNI.readDword(address, fd);
    }

    public static int readDword(long address) {
        return JNI.readDword(address);
    }

    public static float readFloat(int pid, long address) {
        return JNI.readFloat(pid, address);
    }

    public static float readFloat(long address, int fd) {
        return JNI.readFloat(address, fd);
    }

    public static float readFloat(long address) {
        return JNI.readFloat(address);
    }

    public static double readDouble(int pid, long address) {
        return JNI.readDouble(pid, address);
    }

    public static double readDouble(long address, int fd) {
        return JNI.readDouble(address, fd);
    }

    public static double readDouble(long address) {
        return JNI.readDouble(address);
    }
}
