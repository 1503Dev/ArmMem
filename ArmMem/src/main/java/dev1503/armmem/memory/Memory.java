package dev1503.armmem.memory;

import static dev1503.armmem.memory.JNI.getPid;

public class Memory {
//    public static final int RANGE_ALL = 0;
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
    public static MemoryValueSet searchQword(int pid, long value, int memoryRange) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchQword(pid, value, memoryRange);
        for (long address : addresses) {
            memoryValueSet.addQword(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchQword(long value, int memoryRange) {
        return searchQword(getPid(), value, memoryRange);
    }

    public static MemoryValueSet searchQword(int pid, long value, long[] prevList) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchQword(pid, value, prevList);
        for (long address : addresses) {
            memoryValueSet.addQword(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchQword(long value, long[] prevList) {
        return searchQword(getPid(), value, prevList);
    }

    public static MemoryValueSet searchByte(int pid, byte value, int memoryRange) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchByte(pid, value, memoryRange);
        for (long address : addresses) {
            memoryValueSet.addByte(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchByte(byte value, int memoryRange) {
        return searchByte(getPid(), value, memoryRange);
    }

    public static MemoryValueSet searchByte(int pid, byte value, long[] prevList) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchByte(pid, value, prevList);
        for (long address : addresses) {
            memoryValueSet.addByte(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchByte(byte value, long[] prevList) {
        return searchByte(getPid(), value, prevList);
    }

    public static MemoryValueSet searchWord(int pid, short value, int memoryRange) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchWord(pid, value, memoryRange);
        for (long address : addresses) {
            memoryValueSet.addWord(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchWord(short value, int memoryRange) {
        return searchWord(getPid(), value, memoryRange);
    }

    public static MemoryValueSet searchWord(int pid, short value, long[] prevList) {
        MemoryValueSet memoryValueSet = new MemoryValueSet();
        long[] addresses = JNI.searchWord(pid, value, prevList);
        for (long address : addresses) {
            memoryValueSet.addWord(pid, address, value);
        }
        return memoryValueSet;
    }

    public static MemoryValueSet searchWord(short value, long[] prevList) {
        return searchWord(getPid(), value, prevList);
    }


    public static boolean writeDword(int pid, long address, int value) {
        return JNI.writeDword(pid, address, value);
    }

    public static boolean writeDword(long address, int value) {
        return JNI.writeDword(address, value);
    }

    public static boolean writeFloat(int pid, long address, float value) {
        return JNI.writeFloat(pid, address, value);
    }

    public static boolean writeFloat(long address, float value) {
        return JNI.writeFloat(address, value);
    }

    public static boolean writeDouble(int pid, long address, double value) {
        return JNI.writeDouble(pid, address, value);
    }

    public static boolean writeDouble(long address, double value) {
        return JNI.writeDouble(address, value);
    }

    public static boolean writeQword(int pid, long address, long value) {
        return JNI.writeQword(pid, address, value);
    }

    public static boolean writeQword(long address, long value) {
        return JNI.writeQword(address, value);
    }

    public static boolean writeByte(int pid, long address, byte value) {
        return JNI.writeByte(pid, address, value);
    }

    public static boolean writeByte(long address, byte value) {
        return JNI.writeByte(address, value);
    }

    public static boolean writeWord(int pid, long address, short value) {
        return JNI.writeWord(pid, address, value);
    }

    public static boolean writeWord(long address, short value) {
        return JNI.writeWord(address, value);
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

    public static long readQword(int pid, long address) {
        return JNI.readQword(pid, address);
    }

    public static long readQword(long address, int fd) {
        return JNI.readQword(address, fd);
    }

    public static long readQword(long address) {
        return JNI.readQword(address);
    }

    public static byte readByte(int pid, long address) {
        return JNI.readByte(pid, address);
    }

    public static byte readByte(long address, int fd) {
        return JNI.readByte(address, fd);
    }

    public static byte readByte(long address) {
        return JNI.readByte(address);
    }

    public static short readWord(int pid, long address) {
        return JNI.readWord(pid, address);
    }

    public static short readWord(long address, int fd) {
        return JNI.readWord(address, fd);
    }

    public static short readWord(long address) {
        return JNI.readWord(address);
    }

    public static long[] searchSignature(int pid, String pattern, int memoryRange) {
        return JNI.searchSignature(pid, pattern, memoryRange);
    }

    public static long[] searchSignature(String pattern, int memoryRange) {
        return JNI.searchSignature(pattern, memoryRange);
    }

    public static long[] searchSignature(int pid, String pattern, long[] prevList) {
        return JNI.searchSignature(pid, pattern, prevList);
    }

    public static long[] searchSignature(String pattern, long[] prevList) {
        return JNI.searchSignature(pattern, prevList);
    }

    public static long[] search(int pid, String expression, int memoryRange) {
        return JNI.search(pid, expression, memoryRange);
    }

    public static long[] search(String expression, int memoryRange) {
        return JNI.search(expression, memoryRange);
    }

    public static long[] search(int pid, String expression, long[] prevList) {
        return JNI.search(pid, expression, prevList);
    }

    public static long[] search(String expression, long[] prevList) {
        return JNI.search(expression, prevList);
    }
}
