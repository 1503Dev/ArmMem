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

    public int readDword() {
        return JNI.readDword(pid, address);
    }
    public float readFloat() {
        return JNI.readFloat(pid, address);
    }
    public double readDouble() {
        return JNI.readDouble(pid, address);
    }
    public int readDword(int fd) {
        return JNI.readDword(address, fd);
    }
    public float readFloat(int fd) {
        return JNI.readFloat(address, fd);
    }
    public double readDouble(int fd) {
        return JNI.readDouble(address, fd);
    }
}
