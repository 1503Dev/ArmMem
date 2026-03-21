package dev1503.armmem.memory;

public class JNI {
    static {
        System.loadLibrary("ArmMem");
    }

    public static native int getPid();
    public static native int getPid(String packageName);

    public native static long[] searchDword(int pid, int value, int memoryRange);
    public native static long[] searchDword(int pid, int value, long[] prevList);
    public native static long[] searchDword(int value, int memoryRange);
    public native static long[] searchDword(int value, long[] prevList);
    public native static long[] searchFloat(int pid, float value, float radius, int memoryRange);
    public native static long[] searchFloat(int pid, float value, float radius, long[] prevList);
    public native static long[] searchFloat(float value, float radius, long[] prevList);
    public native static long[] searchFloat(float value, float radius, int memoryRange);
    public native static long[] searchDouble(int pid, double value, double radius, int memoryRange);
    public native static long[] searchDouble(int pid, double value, double radius, long[] prevList);
    public native static long[] searchDouble(double value, double radius, long[] prevList);
    public native static long[] searchDouble(double value, double radius, int memoryRange);

    public native static void writeDword(int pid, long address, int value);
    public native static void writeDword(long address, int value);
    public native static void writeFloat(int pid, long address, float value);
    public native static void writeFloat(long address, float value);
    public native static void writeDouble(int pid, long address, double value);
    public native static void writeDouble(long address, double value);

    public native static int openMemFile(int pid);
    public native static void closeMemFile(int fd);

    public native static int readDword(int pid, long address);
    public native static int readDword(long address, int fd);
    public native static int readDword(long address);
    public native static float readFloat(int pid, long address);
    public native static float readFloat(long address, int fd);
    public native static float readFloat(long address);
    public native static double readDouble(int pid, long address);
    public native static double readDouble(long address, int fd);
    public native static double readDouble(long address);
}
