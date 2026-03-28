package dev1503.armmem.memory;

public class MemoryValue {
    private int pid;
    private long address;

    public MemoryValue(int pid, long address) {
        this.pid = pid;
        this.address = address;
    }
    public long getAddress() {
        return address;
    }
    public int getPid() {
        return pid;
    }
    public MemoryValue setPid(int pid) {
        this.pid = pid;
        return this;
    }
    public MemoryValue setAddress(long address) {
        this.address = address;
        return this;
    }

    /* --------------------------------
     * Memory Write
     * -------------------------------- */

    public MemoryValue writeDword(int value) {
        JNI.writeDword(pid, address, value);
        return this;
    }
    public MemoryValue writeFloat(float value) {
        JNI.writeFloat(pid, address, value);
        return this;
    }
    public MemoryValue writeDouble(double value) {
        JNI.writeDouble(pid, address, value);
        return this;
    }
    public MemoryValue writeQword(long value) {
        JNI.writeQword(pid, address, value);
        return this;
    }
    public MemoryValue writeByte(byte value) {
        JNI.writeByte(pid, address, value);
        return this;
    }
    public MemoryValue writeWord(short value) {
        JNI.writeWord(pid, address, value);
        return this;
    }

    /* --------------------------------
     * Memory Read (via PID)
     * -------------------------------- */

    public int readDword() {
        return JNI.readDword(pid, address);
    }
    public float readFloat() {
        return JNI.readFloat(pid, address);
    }
    public double readDouble() {
        return JNI.readDouble(pid, address);
    }
    public long readQword() {
        return JNI.readQword(pid, address);
    }
    public byte readByte() {
        return JNI.readByte(pid, address);
    }
    public short readWord() {
        return JNI.readWord(pid, address);
    }

    /* --------------------------------
     * Memory Read (via FD - Optimized)
     * -------------------------------- */

    public int readDword(int fd) {
        return JNI.readDword(address, fd);
    }
    public float readFloat(int fd) {
        return JNI.readFloat(address, fd);
    }
    public double readDouble(int fd) {
        return JNI.readDouble(address, fd);
    }
    public long readQword(int fd) {
        return JNI.readQword(address, fd);
    }
    public byte readByte(int fd) {
        return JNI.readByte(address, fd);
    }
    public short readWord(int fd) {
        return JNI.readWord(address, fd);
    }
}