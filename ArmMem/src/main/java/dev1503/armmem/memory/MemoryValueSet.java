package dev1503.armmem.memory;

import java.util.HashSet;
import java.util.Iterator;

public class MemoryValueSet extends HashSet<MemoryValue> {
    public boolean add(MemoryValue memoryValue) {
        return super.add(memoryValue);
    }

    public boolean addDword(int pid, long address, int value) {
        return add(new MemoryValue(pid, address).writeDword(value));
    }

    public boolean addFloat(int pid, long address, float value) {
        return add(new MemoryValue(pid, address).writeFloat(value));
    }

    public boolean addDouble(int pid, long address, double value) {
        return add(new MemoryValue(pid, address).writeDouble(value));
    }

    public boolean contains(MemoryValue memoryValue) {
        return super.contains(memoryValue);
    }

    public int getFirstPid() {
        return iterator().next().getPid();
    }

    public long[] getAddresses(int count) {
        int size = Math.min(size(), count);
        long[] addresses = new long[size];
        Iterator<MemoryValue> iterator = iterator();
        for (int i = 0; i < size; i++) {
            addresses[i] = iterator.next().getAddress();
        }
        return addresses;
    }

    public long[] getAddresses() {
        return getAddresses(size());
    }

    public int[] getDwordValues(int count) {
        int size = Math.min(size(), count);
        int fd = Memory.openMemFile(getFirstPid());
        int[] dwordValues = new int[size];
        Iterator<MemoryValue> iterator = iterator();
        for (int i = 0; i < size; i++) {
            dwordValues[i] = iterator.next().readDword(fd);
        }
        Memory.closeMemFile(fd);
        return dwordValues;
    }

    public int[] getDwordValues() {
        return getDwordValues(size());
    }

    public float[] getFloatValues(int count) {
        int size = Math.min(size(), count);
        int fd = Memory.openMemFile(getFirstPid());
        float[] floatValues = new float[size];
        Iterator<MemoryValue> iterator = iterator();
        for (int i = 0; i < size; i++) {
            floatValues[i] = iterator.next().readFloat(fd);
        }
        Memory.closeMemFile(fd);
        return floatValues;
    }

    public float[] getFloatValues() {
        return getFloatValues(size());
    }

    public double[] getDoubleValues(int count) {
        int size = Math.min(size(), count);
        int fd = Memory.openMemFile(getFirstPid());
        double[] doubleValues = new double[size];
        Iterator<MemoryValue> iterator = iterator();
        for (int i = 0; i < size; i++) {
            doubleValues[i] = iterator.next().readDouble(fd);
        }
        Memory.closeMemFile(fd);
        return doubleValues;
    }

    public double[] getDoubleValues() {
        return getDoubleValues(size());
    }

    public MemoryValue get(int index) {
        Iterator<MemoryValue> iterator = iterator();
        if (index >= size()) {
            return null;
        }
        for (int i = 0; i < index; i++) {
            iterator.next();
        }
        return iterator.next();
    }

    public MemoryValueSet searchDword(int value) {
        return Memory.searchDword(getFirstPid(), value, getAddresses());
    }

    public MemoryValueSet searchFloat(float value, float radius) {
        return Memory.searchFloat(getFirstPid(), value, radius, getAddresses());
    }

    public MemoryValueSet searchDouble(double value, double radius) {
        return Memory.searchDouble(getFirstPid(), value, radius, getAddresses());
    }
}
