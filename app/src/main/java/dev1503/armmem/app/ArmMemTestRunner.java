package dev1503.armmem.app;

import android.util.Log;

import dev1503.armmem.ArmMem;
import dev1503.armmem.hook.Hook;
import dev1503.armmem.memory.Memory;
import dev1503.armmem.memory.MemoryValue;
import dev1503.armmem.memory.MemoryValueSet;

public class ArmMemTestRunner {
    private static final String TAG = "ArmMemTest";
    private int passed = 0;
    private int failed = 0;

    public String runAll() {
        passed = 0;
        failed = 0;
        StringBuilder sb = new StringBuilder();

        run(sb, "MemoryValue constructor", this::testMemoryValue_constructor);
        run(sb, "MemoryValue setters", this::testMemoryValue_setters);
        run(sb, "emptySet getFirstPid", this::testEmptySet_getFirstPid);
        run(sb, "emptySet getAddresses", this::testEmptySet_getAddresses);
        run(sb, "emptySet get", this::testEmptySet_get);
        run(sb, "addDword no side effect", this::testAddDword_noSideEffect);
        run(sb, "addDword adds to set", this::testAddDword_addsToSet);
        run(sb, "addDword duplicate", this::testAddDword_duplicate);
        run(sb, "getAddresses limit", this::testGetAddresses_limit);
        run(sb, "contains", this::testContains);
        run(sb, "writeDword round-trip", this::testWriteDword_roundTrip);
        run(sb, "writeFloat round-trip", this::testWriteFloat_roundTrip);
        run(sb, "writeDouble round-trip", this::testWriteDouble_roundTrip);
        run(sb, "writeByte round-trip", this::testWriteByte_roundTrip);
        run(sb, "writeWord round-trip", this::testWriteWord_roundTrip);
        run(sb, "writeQword round-trip", this::testWriteQword_roundTrip);
        run(sb, "MemoryValue write/read", this::testMemoryValue_writeRead);
        run(sb, "MemoryValue read via fd", this::testMemoryValue_readViaFd);
        run(sb, "searchDword finds value", this::testSearchDword_findsValue);
        run(sb, "searchDword refine", this::testSearchDword_refine);
        run(sb, "openMemFile", this::testOpenMemFile);
        run(sb, "getPid", this::testGetPid);
        run(sb, "write returns bool", this::testWrite_returnsBool);
        run(sb, "hook getFunctionAddress valid", this::testHook_getFunctionAddress_valid);
        run(sb, "hook getFunctionAddress invalid", this::testHook_getFunctionAddress_invalid);
        run(sb, "hook null target fails", this::testHook_nullTarget);
        run(sb, "hook null func fails", this::testHook_nullFunc);
        run(sb, "hook and unhook cycle", this::testHook_unhook_cycle);

        String result = "\n===== ArmMem Test Results =====\n" +
                "Passed: " + passed + " / " + (passed + failed) + "\n" +
                "Failed: " + failed + "\n" +
                sb.toString();
        Log.i(TAG, result);
        return result;
    }

    private interface TestCase { void run(); }

    private void run(StringBuilder sb, String name, TestCase tc) {
        try {
            tc.run();
            passed++;
            sb.append("  PASS: ").append(name).append("\n");
        } catch (Throwable t) {
            failed++;
            sb.append("  FAIL: ").append(name).append(" - ").append(t.getMessage()).append("\n");
            Log.e(TAG, "FAIL: " + name, t);
        }
    }

    private static void check(String msg, boolean condition) {
        if (!condition) throw new AssertionError(msg);
    }

    // ========== Tests ==========

    private void testMemoryValue_constructor() {
        MemoryValue v = new MemoryValue(1234, 0xDEADL);
        check("pid", v.getPid() == 1234);
        check("address", v.getAddress() == 0xDEADL);
    }

    private void testMemoryValue_setters() {
        MemoryValue v = new MemoryValue(0, 0);
        v.setPid(99).setAddress(0xFF);
        check("pid", v.getPid() == 99);
        check("address", v.getAddress() == 0xFF);
    }

    private void testEmptySet_getFirstPid() {
        MemoryValueSet set = new MemoryValueSet();
        check("should return -1", set.getFirstPid() == -1);
    }

    private void testEmptySet_getAddresses() {
        MemoryValueSet set = new MemoryValueSet();
        long[] addrs = set.getAddresses();
        check("length 0", addrs.length == 0);
    }

    private void testEmptySet_get() {
        MemoryValueSet set = new MemoryValueSet();
        check("get(0) null", set.get(0) == null);
        check("get(100) null", set.get(100) == null);
    }

    private void testAddDword_noSideEffect() {
        int pid = android.os.Process.myPid();
        MemoryValueSet search = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (search.isEmpty()) return;

        MemoryValue v = search.get(0);
        long addr = v.getAddress();
        int before = Memory.readDword(pid, addr);

        MemoryValueSet set = new MemoryValueSet();
        set.addDword(pid, addr, 99999);

        int after = Memory.readDword(pid, addr);
        check("addDword must not write to memory, before=" + before + " after=" + after, before == after);
    }

    private void testAddDword_addsToSet() {
        MemoryValueSet set = new MemoryValueSet();
        check("add returns true", set.addDword(100, 0x1000L, 42));
        check("size 1", set.size() == 1);
        MemoryValue v = set.get(0);
        check("pid", v.getPid() == 100);
        check("address", v.getAddress() == 0x1000L);
    }

    private void testAddDword_duplicate() {
        MemoryValueSet set = new MemoryValueSet();
        check("first add", set.addDword(100, 0x1000L, 1));
        check("duplicate add", !set.addDword(100, 0x1000L, 2));
        check("size still 1", set.size() == 1);
    }

    private void testGetAddresses_limit() {
        MemoryValueSet set = new MemoryValueSet();
        set.addDword(1, 0x100L, 10);
        set.addDword(1, 0x200L, 20);
        set.addDword(1, 0x300L, 30);

        long[] addrs = set.getAddresses(2);
        check("limited to 2", addrs.length == 2);
    }

    private void testContains() {
        MemoryValueSet set = new MemoryValueSet();
        MemoryValue v = new MemoryValue(1, 0x100L);
        set.add(v);
        check("contains added", set.contains(v));
        check("not contains different", !set.contains(new MemoryValue(1, 0x200L)));
    }

    private void testWriteDword_roundTrip() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        int magic = 0xCAFEBABE;

        check("writeDword", Memory.writeDword(pid, addr, magic));
        int readBack = Memory.readDword(pid, addr);
        check("round-trip: wrote 0x" + Integer.toHexString(magic) + " read 0x" + Integer.toHexString(readBack), readBack == magic);
        Memory.writeDword(pid, addr, 0);
    }

    private void testWriteFloat_roundTrip() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchFloat(pid, 0f, 0.001f, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        float magic = 3.14159f;

        check("writeFloat", Memory.writeFloat(pid, addr, magic));
        float readBack = Memory.readFloat(pid, addr);
        check("round-trip", Math.abs(readBack - magic) < 0.0001f);
        Memory.writeFloat(pid, addr, 0f);
    }

    private void testWriteDouble_roundTrip() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDouble(pid, 0.0, 0.001, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        double magic = 2.718281828459045;

        check("writeDouble", Memory.writeDouble(pid, addr, magic));
        double readBack = Memory.readDouble(pid, addr);
        check("round-trip", Math.abs(readBack - magic) < 0.0000001);
        Memory.writeDouble(pid, addr, 0.0);
    }

    private void testWriteByte_roundTrip() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchByte(pid, (byte) 0, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        byte magic = (byte) 0xAB;

        Memory.writeByte(pid, addr, magic);
        byte readBack = Memory.readByte(pid, addr);
        check("round-trip", readBack == magic);
        Memory.writeByte(pid, addr, (byte) 0);
    }

    private void testWriteWord_roundTrip() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchWord(pid, (short) 0, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        short magic = (short) 0xBEEF;

        Memory.writeWord(pid, addr, magic);
        short readBack = Memory.readWord(pid, addr);
        check("round-trip", readBack == magic);
        Memory.writeWord(pid, addr, (short) 0);
    }

    private void testWriteQword_roundTrip() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchQword(pid, 0L, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        long magic = 0xDEADBEEFCAFEBABEL;

        check("writeQword", Memory.writeQword(pid, addr, magic));
        long readBack = Memory.readQword(pid, addr);
        check("round-trip", readBack == magic);
        Memory.writeQword(pid, addr, 0L);
    }

    private void testMemoryValue_writeRead() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) return;

        MemoryValue v = results.get(0);
        int magic = 0x12345678;
        v.writeDword(magic);
        check("MemoryValue write/read", v.readDword() == magic);
        v.writeDword(0);
    }

    private void testMemoryValue_readViaFd() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) return;

        MemoryValue v = results.get(0);
        int magic = 0xAABBCCDD;
        v.writeDword(magic);

        int fd = Memory.openMemFile(pid);
        check("fd valid", fd > 0);
        int readBack = v.readDword(fd);
        Memory.closeMemFile(fd);
        check("read via fd", readBack == magic);
        v.writeDword(0);
    }

    private void testSearchDword_findsValue() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) return;

        long addr = results.get(0).getAddress();
        int magic = 0x78563412;

        Memory.writeDword(pid, addr, magic);

        MemoryValueSet found = Memory.searchDword(pid, magic, Memory.RANGE_ANONYMOUS);
        boolean foundIt = false;
        for (MemoryValue v : found) {
            if (v.getAddress() == addr) { foundIt = true; break; }
        }
        check("searchDword finds written value", foundIt);
        Memory.writeDword(pid, addr, 0);
    }

    private void testSearchDword_refine() {
        int pid = android.os.Process.myPid();
        MemoryValueSet init = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (init.size() < 2) return;

        long addr1 = init.get(0).getAddress();
        int val1 = 0x11111111;

        Memory.writeDword(pid, addr1, val1);

        MemoryValueSet first = Memory.searchDword(pid, val1, Memory.RANGE_ANONYMOUS);
        boolean found = false;
        for (MemoryValue v : first) {
            if (v.getAddress() == addr1) { found = true; break; }
        }
        check("first search finds addr1", found);

        MemoryValueSet refined = first.searchDword(val1);
        found = false;
        for (MemoryValue v : refined) {
            if (v.getAddress() == addr1) { found = true; break; }
        }
        check("refined search finds addr1", found);
        Memory.writeDword(pid, addr1, 0);
    }

    private void testOpenMemFile() {
        int pid = android.os.Process.myPid();
        int fd = Memory.openMemFile(pid);
        check("fd > 0", fd > 0);
        Memory.closeMemFile(fd);

        int fd2 = Memory.openMemFile(99999);
        check("invalid pid returns -1", fd2 == -1);
    }

    private void testGetPid() {
        int pid = dev1503.armmem.memory.JNI.getPid();
        check("matches current process", pid == android.os.Process.myPid());
    }

    private void testWrite_returnsBool() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) return;

        long addr = results.get(0).getAddress();
        check("valid addr returns true", Memory.writeDword(pid, addr, 0));
        check("invalid addr returns false", !Memory.writeDword(0x1L, 42));
    }

    private void testHook_getFunctionAddress_valid() {
        long addr = Hook.getFunctionAddress("libc.so", "open");
        check("should find open() address, got 0x" + Long.toHexString(addr), addr != 0);
    }

    private void testHook_getFunctionAddress_invalid() {
        long addr = Hook.getFunctionAddress("libc.so", "nonexistent_function_xyz_abc");
        check("invalid symbol should return 0", addr == 0);
    }

    private void testHook_getFunctionAddress_invalidModule() {
        long addr = Hook.getFunctionAddress("libnonexistent.so", "open");
        check("invalid module should return 0", addr == 0);
    }

    private void testHook_nullTarget() {
        JNI jni = new JNI();
        boolean result = jni.testHookNullTarget();
        check("hook with null target should fail", !result);
    }

    private void testHook_nullFunc() {
        JNI jni = new JNI();
        boolean result = jni.testHookNullFunc();
        check("hook with null func should fail", !result);
    }

    private void testHook_unhook_cycle() {
        JNI jni = new JNI();

        String before = jni.stringFromJNI();
        check("original stringFromJNI should return non-empty", before != null && !before.isEmpty());

        boolean hooked = jni.testHook();
        check("hook should succeed", hooked);

        String afterHook = jni.stringFromJNI();
        check("after hook should return HOOKED, got: " + afterHook, "HOOKED".equals(afterHook));

        // unhook
        boolean unhooked = jni.testUnhook();
        check("unhook should succeed", unhooked);

        String afterUnhook = jni.stringFromJNI();
        check("after unhook should restore original, got: " + afterUnhook, !afterUnhook.equals("HOOKED"));
    }
}
