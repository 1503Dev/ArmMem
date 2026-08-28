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
        run(sb, "signature exact match", this::testSignature_exactMatch);
        run(sb, "signature wildcard match", this::testSignature_wildcard);
        run(sb, "signature no match", this::testSignature_noMatch);
        run(sb, "signature empty pattern", this::testSignature_emptyPattern);
        run(sb, "signature refine", this::testSignature_refine);
        run(sb, "signature IDA pattern", this::testSignature_idaPattern);
        run(sb, "search combined single group", this::testSearch_combinedSingle);
        run(sb, "search combined two groups", this::testSearch_combinedTwoGroups);
        run(sb, "search combined with span", this::testSearch_combinedWithSpan);
        run(sb, "search single dword", this::testSearch_singleDword);
        run(sb, "search real dword+float struct", this::testSearch_realDwordFloat);
        run(sb, "search real mixed struct", this::testSearch_realMixed);
        run(sb, "search combined gap", this::testSearch_combinedGap);
        run(sb, "search combined float radius", this::testSearch_combinedFloatRadius);
        run(sb, "search combined hex", this::testSearch_combinedHex);
        run(sb, "search combined invalid expression", this::testSearch_combinedInvalid);
        run(sb, "search combined span reject", this::testSearch_combinedSpanReject);
        run(sb, "search combined refine multi-group", this::testSearch_combinedRefineMultiGroup);

        String result = "\n===== ArmMem Test Results =====\n" +
                "Passed: " + passed + " / " + (passed + failed) + "\n" +
                "Failed: " + failed + "\n" +
                sb.toString();
        Log.i(TAG, "===== SUMMARY =====");
        Log.i(TAG, "Passed: " + passed + " / " + (passed + failed));
        Log.i(TAG, "Failed: " + failed);
        return result;
    }

    private interface TestCase { void run(); }

    private void run(StringBuilder sb, String name, TestCase tc) {
        try {
            tc.run();
            passed++;
            sb.append("  PASS: ").append(name).append("\n");
            Log.d(TAG, "PASS: " + name);
        } catch (Throwable t) {
            failed++;
            sb.append("  FAIL: ").append(name).append(" - ").append(t.getMessage()).append("\n");
            Log.e(TAG, "FAIL: " + name + " - " + t.getMessage(), t);
        }
    }

    private static void check(String msg, boolean condition) {
        if (!condition) {
            Log.e(TAG, "CHECK FAILED: " + msg);
            throw new AssertionError(msg);
        }
    }

    // ========== Tests ==========

    private void testMemoryValue_constructor() {
        Log.d(TAG, "Creating MemoryValue(pid=1234, address=0xDEAD)");
        MemoryValue v = new MemoryValue(1234, 0xDEADL);
        Log.d(TAG, "  pid=" + v.getPid() + ", address=0x" + Long.toHexString(v.getAddress()));
        check("pid", v.getPid() == 1234);
        check("address", v.getAddress() == 0xDEADL);
    }

    private void testMemoryValue_setters() {
        Log.d(TAG, "Creating MemoryValue(0, 0), setting pid=99, address=0xFF");
        MemoryValue v = new MemoryValue(0, 0);
        v.setPid(99).setAddress(0xFF);
        Log.d(TAG, "  pid=" + v.getPid() + ", address=0x" + Long.toHexString(v.getAddress()));
        check("pid", v.getPid() == 99);
        check("address", v.getAddress() == 0xFF);
    }

    private void testEmptySet_getFirstPid() {
        MemoryValueSet set = new MemoryValueSet();
        int pid = set.getFirstPid();
        Log.d(TAG, "Empty set getFirstPid=" + pid);
        check("should return -1", pid == -1);
    }

    private void testEmptySet_getAddresses() {
        MemoryValueSet set = new MemoryValueSet();
        long[] addrs = set.getAddresses();
        Log.d(TAG, "Empty set getAddresses length=" + addrs.length);
        check("length 0", addrs.length == 0);
    }

    private void testEmptySet_get() {
        MemoryValueSet set = new MemoryValueSet();
        MemoryValue v0 = set.get(0);
        MemoryValue v100 = set.get(100);
        Log.d(TAG, "Empty set get(0)=" + v0 + ", get(100)=" + v100);
        check("get(0) null", v0 == null);
        check("get(100) null", v100 == null);
    }

    private void testAddDword_noSideEffect() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS to find safe address");
        MemoryValueSet search = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (search.isEmpty()) { Log.d(TAG, "  no results, skip"); return; }

        MemoryValue v = search.get(0);
        long addr = v.getAddress();
        int before = Memory.readDword(pid, addr);
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", before=" + before);

        MemoryValueSet set = new MemoryValueSet();
        set.addDword(pid, addr, 99999);
        Log.d(TAG, "  addDword(pid=" + pid + ", addr=0x" + Long.toHexString(addr) + ", value=99999)");

        int after = Memory.readDword(pid, addr);
        Log.d(TAG, "  after=" + after);
        check("addDword must not write to memory, before=" + before + " after=" + after, before == after);
    }

    private void testAddDword_addsToSet() {
        MemoryValueSet set = new MemoryValueSet();
        Log.d(TAG, "Adding pid=100, addr=0x1000, value=42");
        boolean added = set.addDword(100, 0x1000L, 42);
        Log.d(TAG, "  added=" + added + ", size=" + set.size());
        check("add returns true", added);
        check("size 1", set.size() == 1);
        MemoryValue v = set.get(0);
        Log.d(TAG, "  v.pid=" + v.getPid() + ", v.address=0x" + Long.toHexString(v.getAddress()));
        check("pid", v.getPid() == 100);
        check("address", v.getAddress() == 0x1000L);
    }

    private void testAddDword_duplicate() {
        MemoryValueSet set = new MemoryValueSet();
        boolean first = set.addDword(100, 0x1000L, 1);
        boolean dup = set.addDword(100, 0x1000L, 2);
        Log.d(TAG, "first add=" + first + ", dup add=" + dup + ", size=" + set.size());
        check("first add", first);
        check("duplicate add", !dup);
        check("size still 1", set.size() == 1);
    }

    private void testGetAddresses_limit() {
        MemoryValueSet set = new MemoryValueSet();
        set.addDword(1, 0x100L, 10);
        set.addDword(1, 0x200L, 20);
        set.addDword(1, 0x300L, 30);

        long[] addrs = set.getAddresses(2);
        Log.d(TAG, "3 entries, getAddresses(2) length=" + addrs.length);
        check("limited to 2", addrs.length == 2);
    }

    private void testContains() {
        MemoryValueSet set = new MemoryValueSet();
        MemoryValue v = new MemoryValue(1, 0x100L);
        set.add(v);
        boolean contains = set.contains(v);
        boolean notContains = set.contains(new MemoryValue(1, 0x200L));
        Log.d(TAG, "contains(0x100)=" + contains + ", contains(0x200)=" + notContains);
        check("contains added", contains);
        check("not contains different", !notContains);
    }

    private void testWriteDword_roundTrip() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        int magic = 0xCAFEBABE;
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", writing 0x" + Integer.toHexString(magic));

        boolean writeOk = Memory.writeDword(pid, addr, magic);
        Log.d(TAG, "  writeDword=" + writeOk);
        check("writeDword", writeOk);

        int readBack = Memory.readDword(pid, addr);
        Log.d(TAG, "  readBack=0x" + Integer.toHexString(readBack));
        check("round-trip: wrote 0x" + Integer.toHexString(magic) + " read 0x" + Integer.toHexString(readBack), readBack == magic);
        Memory.writeDword(pid, addr, 0);
    }

    private void testWriteFloat_roundTrip() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching float=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchFloat(pid, 0f, 0.001f, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        float magic = 3.14159f;
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", writing " + magic);

        boolean writeOk = Memory.writeFloat(pid, addr, magic);
        Log.d(TAG, "  writeFloat=" + writeOk);
        check("writeFloat", writeOk);

        float readBack = Memory.readFloat(pid, addr);
        Log.d(TAG, "  readBack=" + readBack);
        check("round-trip", Math.abs(readBack - magic) < 0.0001f);
        Memory.writeFloat(pid, addr, 0f);
    }

    private void testWriteDouble_roundTrip() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching double=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDouble(pid, 0.0, 0.001, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        double magic = 2.718281828459045;
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", writing " + magic);

        boolean writeOk = Memory.writeDouble(pid, addr, magic);
        Log.d(TAG, "  writeDouble=" + writeOk);
        check("writeDouble", writeOk);

        double readBack = Memory.readDouble(pid, addr);
        Log.d(TAG, "  readBack=" + readBack);
        check("round-trip", Math.abs(readBack - magic) < 0.0000001);
        Memory.writeDouble(pid, addr, 0.0);
    }

    private void testWriteByte_roundTrip() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching byte=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchByte(pid, (byte) 0, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        byte magic = (byte) 0xAB;
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", writing 0x" + Integer.toHexString(magic & 0xFF));

        Memory.writeByte(pid, addr, magic);
        byte readBack = Memory.readByte(pid, addr);
        Log.d(TAG, "  readBack=0x" + Integer.toHexString(readBack & 0xFF));
        check("round-trip", readBack == magic);
        Memory.writeByte(pid, addr, (byte) 0);
    }

    private void testWriteWord_roundTrip() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching word=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchWord(pid, (short) 0, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        short magic = (short) 0xBEEF;
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", writing 0x" + Integer.toHexString(magic & 0xFFFF));

        Memory.writeWord(pid, addr, magic);
        short readBack = Memory.readWord(pid, addr);
        Log.d(TAG, "  readBack=0x" + Integer.toHexString(readBack & 0xFFFF));
        check("round-trip", readBack == magic);
        Memory.writeWord(pid, addr, (short) 0);
    }

    private void testWriteQword_roundTrip() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching qword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchQword(pid, 0L, Memory.RANGE_ANONYMOUS);
        check("need results", !results.isEmpty());

        long addr = results.get(0).getAddress();
        long magic = 0xDEADBEEFCAFEBABEL;
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", writing 0x" + Long.toHexString(magic));

        boolean writeOk = Memory.writeQword(pid, addr, magic);
        Log.d(TAG, "  writeQword=" + writeOk);
        check("writeQword", writeOk);

        long readBack = Memory.readQword(pid, addr);
        Log.d(TAG, "  readBack=0x" + Long.toHexString(readBack));
        check("round-trip", readBack == magic);
        Memory.writeQword(pid, addr, 0L);
    }

    private void testMemoryValue_writeRead() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "  no results, skip"); return; }

        MemoryValue v = results.get(0);
        int magic = 0x12345678;
        Log.d(TAG, "  addr=0x" + Long.toHexString(v.getAddress()) + ", writing 0x" + Integer.toHexString(magic));
        v.writeDword(magic);
        int readBack = v.readDword();
        Log.d(TAG, "  readBack=0x" + Integer.toHexString(readBack));
        check("MemoryValue write/read", readBack == magic);
        v.writeDword(0);
    }

    private void testMemoryValue_readViaFd() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "  no results, skip"); return; }

        MemoryValue v = results.get(0);
        int magic = 0xAABBCCDD;
        Log.d(TAG, "  addr=0x" + Long.toHexString(v.getAddress()) + ", writing 0x" + Integer.toHexString(magic));
        v.writeDword(magic);

        int fd = Memory.openMemFile(pid);
        Log.d(TAG, "  openMemFile fd=" + fd);
        check("fd valid", fd > 0);

        int readBack = v.readDword(fd);
        Log.d(TAG, "  readBack=0x" + Integer.toHexString(readBack));
        Memory.closeMemFile(fd);
        check("read via fd", readBack == magic);
        v.writeDword(0);
    }

    private void testSearchDword_findsValue() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "  no results, skip"); return; }

        long addr = results.get(0).getAddress();
        int magic = 0x78563412;
        Log.d(TAG, "  writing 0x" + Integer.toHexString(magic) + " at 0x" + Long.toHexString(addr));

        Memory.writeDword(pid, addr, magic);

        Log.d(TAG, "  searching for 0x" + Integer.toHexString(magic));
        MemoryValueSet found = Memory.searchDword(pid, magic, Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.size() + " results");
        boolean foundIt = false;
        for (MemoryValue v : found) {
            Log.d(TAG, "    0x" + Long.toHexString(v.getAddress()));
            if (v.getAddress() == addr) { foundIt = true; }
        }
        check("searchDword finds written value", foundIt);
        Memory.writeDword(pid, addr, 0);
    }

    private void testSearchDword_refine() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet init = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (init.size() < 2) { Log.d(TAG, "  need >=2 results, got " + init.size() + ", skip"); return; }

        long addr1 = init.get(0).getAddress();
        int val1 = 0x11111111;
        Log.d(TAG, "  writing 0x" + Integer.toHexString(val1) + " at 0x" + Long.toHexString(addr1));

        Memory.writeDword(pid, addr1, val1);

        Log.d(TAG, "  first search for 0x" + Integer.toHexString(val1));
        MemoryValueSet first = Memory.searchDword(pid, val1, Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  first found " + first.size() + " results");
        boolean found = false;
        for (MemoryValue v : first) {
            Log.d(TAG, "    0x" + Long.toHexString(v.getAddress()));
            if (v.getAddress() == addr1) { found = true; }
        }
        check("first search finds addr1", found);

        Log.d(TAG, "  refined search for 0x" + Integer.toHexString(val1));
        MemoryValueSet refined = first.searchDword(val1);
        Log.d(TAG, "  refined found " + refined.size() + " results");
        found = false;
        for (MemoryValue v : refined) {
            Log.d(TAG, "    0x" + Long.toHexString(v.getAddress()));
            if (v.getAddress() == addr1) { found = true; }
        }
        check("refined search finds addr1", found);
        Memory.writeDword(pid, addr1, 0);
    }

    private void testOpenMemFile() {
        int pid = android.os.Process.myPid();
        int fd = Memory.openMemFile(pid);
        Log.d(TAG, "openMemFile(" + pid + ")=" + fd);
        check("fd > 0", fd > 0);
        Memory.closeMemFile(fd);

        int fd2 = Memory.openMemFile(99999);
        Log.d(TAG, "openMemFile(99999)=" + fd2);
        check("invalid pid returns -1", fd2 == -1);
    }

    private void testGetPid() {
        int pid = dev1503.armmem.memory.JNI.getPid();
        int myPid = android.os.Process.myPid();
        Log.d(TAG, "getPid()=" + pid + ", myPid=" + myPid);
        check("matches current process", pid == myPid);
    }

    private void testWrite_returnsBool() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "  no results, skip"); return; }

        long addr = results.get(0).getAddress();
        boolean valid = Memory.writeDword(pid, addr, 0);
        boolean invalid = Memory.writeDword(0x1L, 42);
        Log.d(TAG, "  valid addr write=" + valid + ", invalid addr write=" + invalid);
        check("valid addr returns true", valid);
        check("invalid addr returns false", !invalid);
    }

    private void testHook_getFunctionAddress_valid() {
        long addr = Hook.getFunctionAddress("libc.so", "open");
        Log.d(TAG, "getFunctionAddress(\"libc.so\", \"open\")=0x" + Long.toHexString(addr));
        check("should find open() address, got 0x" + Long.toHexString(addr), addr != 0);
    }

    private void testHook_getFunctionAddress_invalid() {
        long addr = Hook.getFunctionAddress("libc.so", "nonexistent_function_xyz_abc");
        Log.d(TAG, "getFunctionAddress(\"libc.so\", \"nonexistent\")=0x" + Long.toHexString(addr));
        check("invalid symbol should return 0", addr == 0);
    }

    private void testHook_getFunctionAddress_invalidModule() {
        long addr = Hook.getFunctionAddress("libnonexistent.so", "open");
        Log.d(TAG, "getFunctionAddress(\"libnonexistent.so\", \"open\")=0x" + Long.toHexString(addr));
        check("invalid module should return 0", addr == 0);
    }

    private void testHook_nullTarget() {
        JNI jni = new JNI();
        boolean result = jni.testHookNullTarget();
        Log.d(TAG, "testHookNullTarget=" + result);
        check("hook with null target should fail", !result);
    }

    private void testHook_nullFunc() {
        JNI jni = new JNI();
        boolean result = jni.testHookNullFunc();
        Log.d(TAG, "testHookNullFunc=" + result);
        check("hook with null func should fail", !result);
    }

    private void testHook_unhook_cycle() {
        JNI jni = new JNI();

        String before = jni.stringFromJNI();
        Log.d(TAG, "before hook: \"" + before + "\"");
        check("original stringFromJNI should return non-empty", before != null && !before.isEmpty());

        boolean hooked = jni.testHook();
        Log.d(TAG, "testHook=" + hooked);
        check("hook should succeed", hooked);

        String afterHook = jni.stringFromJNI();
        Log.d(TAG, "after hook: \"" + afterHook + "\"");
        check("after hook should return HOOKED, got: " + afterHook, "HOOKED".equals(afterHook));

        boolean unhooked = jni.testUnhook();
        Log.d(TAG, "testUnhook=" + unhooked);
        check("unhook should succeed", unhooked);

        String afterUnhook = jni.stringFromJNI();
        Log.d(TAG, "after unhook: \"" + afterUnhook + "\"");
        check("after unhook should restore original, got: " + afterUnhook, !afterUnhook.equals("HOOKED"));
    }

    private void testSignature_exactMatch() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "  no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;

        int original = Memory.readDword(pid, addr);
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", original=0x" + Integer.toHexString(original));

        Memory.writeDword(pid, addr, 0xDDCCBBAA);
        int written = Memory.readDword(pid, addr);
        Log.d(TAG, "  wrote 0xDDCCBBAA, read back=0x" + Integer.toHexString(written));

        Log.d(TAG, "  searching pattern \"AA BB CC DD\" in ANONYMOUS");
        long[] found = Memory.searchSignature(pid, "AA BB CC DD", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");
        boolean hit = false;
        for (long a : found) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) { hit = true; }
        }
        check("exact match should find written bytes, found " + found.length + " results", hit);

        Memory.writeDword(pid, addr, original);
    }

    private void testSignature_wildcard() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "  no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;

        int original = Memory.readDword(pid, addr);
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", original=0x" + Integer.toHexString(original));

        Memory.writeDword(pid, addr, 0xDDCCBBAA);
        int written = Memory.readDword(pid, addr);
        Log.d(TAG, "  wrote 0xDDCCBBAA, read back=0x" + Integer.toHexString(written));

        Log.d(TAG, "  searching pattern \"AA BB ?? DD\" in ANONYMOUS");
        long[] found = Memory.searchSignature(pid, "AA BB ?? DD", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");
        boolean hit = false;
        for (long a : found) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) { hit = true; }
        }
        check("wildcard match should find written bytes, found " + found.length + " results", hit);

        Memory.writeDword(pid, addr, original);
    }

    private void testSignature_noMatch() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching pattern \"FF FE FD FC FB FA\" in ANONYMOUS");
        long[] found = Memory.searchSignature(pid, "FF FE FD FC FB FA", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");
        check("no match search should not crash", found != null);
    }

    private void testSignature_emptyPattern() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching empty pattern");
        long[] found = Memory.searchSignature(pid, "", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");
        check("empty pattern should return empty", found.length == 0);

        Log.d(TAG, "Searching whitespace pattern");
        found = Memory.searchSignature(pid, "   ", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");
        check("whitespace pattern should return empty", found.length == 0);
    }

    private void testSignature_refine() {
        int pid = android.os.Process.myPid();
        Log.d(TAG, "Searching dword=0 in ANONYMOUS");
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "  no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;

        int original = Memory.readDword(pid, addr);
        Log.d(TAG, "  addr=0x" + Long.toHexString(addr) + ", original=0x" + Integer.toHexString(original));

        Memory.writeDword(pid, addr, 0xDDCCBBAA);

        Log.d(TAG, "  first search \"AA BB ?? DD\" in ANONYMOUS");
        long[] first = Memory.searchSignature(pid, "AA BB ?? DD", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  first found " + first.length + " results");
        boolean hit = false;
        for (long a : first) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) { hit = true; }
        }
        check("first signature search should find it", hit);

        Log.d(TAG, "  refined search \"AA BB CC DD\" in first results");
        long[] refined = Memory.searchSignature(pid, "AA BB CC DD", first);
        Log.d(TAG, "  refined found " + refined.length + " results");
        hit = false;
        for (long a : refined) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) { hit = true; }
        }
        check("refined signature search should still find it", hit);

        Memory.writeDword(pid, addr, original);
    }

    private void testSignature_idaPattern() {
        JNI jni = new JNI();
        long realAddr = jni.getModi2Address();
        Log.d(TAG, "modi2 address=0x" + Long.toHexString(realAddr));
        check("modi2 address should be non-zero", realAddr != 0);

        // 诊断：读出实际字节
        int pid = android.os.Process.myPid();
        StringBuilder actual = new StringBuilder();
        for (int i = 0; i < 16; i++) {
            int b = Memory.readByte(pid, realAddr + i) & 0xFF;
            actual.append(String.format("%02X ", b));
        }
        Log.d(TAG, "  actual bytes at 0x" + Long.toHexString(realAddr) + ": " + actual);

        // 直接用 IDA 模式搜索
        String idaPattern = "E8 03 00 F9 E0 03 40 F9 1F 20 03 D5 E1 ? ? ?";
        Log.d(TAG, "  searching IDA pattern: " + idaPattern);

        long[] codeFound = Memory.searchSignature(idaPattern, Memory.RANGE_CODE_APP);
        Log.d(TAG, "  CODE_APP found " + codeFound.length);
        for (long a : codeFound) {
            Log.d(TAG, "    CODE_APP 0x" + Long.toHexString(a));
        }

        long[] anonFound = Memory.searchSignature(idaPattern, Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  ANONYMOUS found " + anonFound.length);
        for (long a : anonFound) {
            Log.d(TAG, "    ANON 0x" + Long.toHexString(a));
        }

        // 用实际字节搜 CODE_APP
        String bytesStr = actual.toString().trim();
        Log.d(TAG, "  searching actual bytes: " + bytesStr);
        long[] actualCode = Memory.searchSignature(bytesStr, Memory.RANGE_CODE_APP);
        Log.d(TAG, "  actual CODE_APP found " + actualCode.length);
        for (long a : actualCode) {
            Log.d(TAG, "    actual 0x" + Long.toHexString(a));
        }

        boolean hit = false;
        for (long a : codeFound) { if (a == realAddr) hit = true; }
        for (long a : anonFound) { if (a == realAddr) hit = true; }
        for (long a : actualCode) { if (a == realAddr) hit = true; }
        check("modi2=0x" + Long.toHexString(realAddr)
              + ", IDA_CODE=" + codeFound.length
              + ", IDA_ANON=" + anonFound.length
              + ", actual_CODE=" + actualCode.length
              + "\n  IDA: " + idaPattern
              + "\n  actual: " + actual, hit);
    }

    private void testSearch_combinedSingle() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;
        int original1 = Memory.readDword(pid, addr);
        int original2 = Memory.readDword(pid, addr + 4);

        Memory.writeDword(pid, addr, 111);
        Memory.writeDword(pid, addr + 4, 222);

        Log.d(TAG, "  searching '111;222' at 0x" + Long.toHexString(addr));
        long[] found = Memory.search("111;222", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");

        boolean hit = false;
        for (long a : found) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) hit = true;
        }
        check("single group '111;222' should match at 0x" + Long.toHexString(addr)
              + ", found=" + found.length, hit);

        Memory.writeDword(pid, addr, original1);
        Memory.writeDword(pid, addr + 4, original2);
    }

    private void testSearch_combinedTwoGroups() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;
        int o1 = Memory.readDword(pid, addr);
        int o2 = Memory.readDword(pid, addr + 4);
        int o3 = Memory.readDword(pid, addr + 8);
        int o4 = Memory.readDword(pid, addr + 12);

        Memory.writeDword(pid, addr, 111);
        Memory.writeDword(pid, addr + 4, 222);
        Memory.writeDword(pid, addr + 8, 333);
        Memory.writeDword(pid, addr + 12, 444);

        Log.d(TAG, "  searching '111;222||333;444' at 0x" + Long.toHexString(addr));
        long[] found = Memory.search("111;222||333;444", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");

        boolean hit = false;
        for (long a : found) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) hit = true;
        }
        check("two groups '111;222||333;444' should match at 0x" + Long.toHexString(addr)
              + ", found=" + found.length, hit);

        Memory.writeDword(pid, addr, o1);
        Memory.writeDword(pid, addr + 4, o2);
        Memory.writeDword(pid, addr + 8, o3);
        Memory.writeDword(pid, addr + 12, o4);
    }

    private void testSearch_combinedWithSpan() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;
        int o1 = Memory.readDword(pid, addr);
        int o2 = Memory.readDword(pid, addr + 4);
        int o3 = Memory.readDword(pid, addr + 8);

        Memory.writeDword(pid, addr, 555);
        Memory.writeDword(pid, addr + 4, 666);
        Memory.writeDword(pid, addr + 8, 777);

        Log.d(TAG, "  searching '555;666:20||777' at 0x" + Long.toHexString(addr));
        long[] found = Memory.search("555;666:20||777", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");

        boolean hit = false;
        for (long a : found) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) hit = true;
        }
        check("span '555;666:20::777' should match at 0x" + Long.toHexString(addr)
              + ", found=" + found.length, hit);

        Memory.writeDword(pid, addr, o1);
        Memory.writeDword(pid, addr + 4, o2);
        Memory.writeDword(pid, addr + 8, o3);
    }

    private void testSearch_singleDword() {
        JNI jni = new JNI();
        long base = jni.getDwordFloatAddress();
        int pid = android.os.Process.myPid();

        // 验证单值搜索能找到
        Log.d(TAG, "  searching '111' in ANONYMOUS");
        long[] found = Memory.search("111", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length);
        boolean hit = false;
        for (long a : found) { if (a == base) hit = true; }
        Log.d(TAG, "  hit at base=" + hit);

        // 验证地址正确性
        int readVal = Memory.readDword(pid, base);
        Log.d(TAG, "  readDword(base) = " + readVal);
        check("single dword '111' should match, readDword=" + readVal, readVal == 111);
    }

    private void testSearch_realDwordFloat() {
        JNI jni = new JNI();
        long base = jni.getDwordFloatAddress();
        Log.d(TAG, "DwordFloatGroup base=0x" + Long.toHexString(base));

        int pid = android.os.Process.myPid();

        // 诊断：读出实际值
        int i1 = Memory.readDword(pid, base);
        float f1 = Memory.readFloat(pid, base + 4);
        int i2 = Memory.readDword(pid, base + 8);
        Log.d(TAG, "  i1=" + i1 + " f1=" + f1 + " i2=" + i2);

        // 搜索 i1=111, f1=2.5, i2=222
        Log.d(TAG, "  searching '111;f::2.5;222' in ANONYMOUS");
        long[] found = Memory.search("111;f::2.5;222", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  ANONYMOUS found " + found.length);

        // 也搜 C_BSS
        long[] foundBss = Memory.search("111;f::2.5;222", Memory.RANGE_C_BSS);
        Log.d(TAG, "  C_BSS found " + foundBss.length);

        boolean hit = false;
        for (long a : found) { Log.d(TAG, "    ANON 0x" + Long.toHexString(a)); if (a == base) hit = true; }
        for (long a : foundBss) { Log.d(TAG, "    BSS 0x" + Long.toHexString(a)); if (a == base) hit = true; }
        check("should find dword+float struct at 0x" + Long.toHexString(base)
              + ", ANON=" + found.length + ", BSS=" + foundBss.length
              + ", actual: i1=" + i1 + " f1=" + f1 + " i2=" + i2, hit);
    }

    private void testSearch_realMixed() {
        JNI jni = new JNI();
        long base = jni.getMixedGroupAddress();
        Log.d(TAG, "MixedGroup base=0x" + Long.toHexString(base));

        int pid = android.os.Process.myPid();

        // 诊断：读出实际值
        int i1 = Memory.readDword(pid, base);
        float f1 = Memory.readFloat(pid, base + 4);
        double d1 = Memory.readDouble(pid, base + 8);
        short w1 = Memory.readWord(pid, base + 16);
        Log.d(TAG, "  i1=" + i1 + " f1=" + f1 + " d1=" + d1 + " w1=" + w1);

        Log.d(TAG, "  searching '100;f::1.5;e::2.5;w::300' in ANONYMOUS");
        long[] found = Memory.search("100;f::1.5;e::2.5;w::300", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  ANONYMOUS found " + found.length);

        long[] foundBss = Memory.search("100;f::1.5;e::2.5;w::300", Memory.RANGE_C_BSS);
        Log.d(TAG, "  C_BSS found " + foundBss.length);

        boolean hit = false;
        for (long a : found) { Log.d(TAG, "    ANON 0x" + Long.toHexString(a)); if (a == base) hit = true; }
        for (long a : foundBss) { Log.d(TAG, "    BSS 0x" + Long.toHexString(a)); if (a == base) hit = true; }
        check("should find mixed struct at 0x" + Long.toHexString(base)
              + ", ANON=" + found.length + ", BSS=" + foundBss.length
              + ", actual: i1=" + i1 + " f1=" + f1 + " d1=" + d1 + " w1=" + w1, hit);
    }

    private void testSearch_combinedGap() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;
        int o1 = Memory.readDword(pid, addr);
        int o2 = Memory.readDword(pid, addr + 16);

        // gap 12: 555 结束于 base+4, 666 从 base+16 开始
        Memory.writeDword(pid, addr, 555);
        Memory.writeDword(pid, addr + 16, 666);

        Log.d(TAG, "  searching '555:12:666' at 0x" + Long.toHexString(addr));
        long[] found = Memory.search("555:12:666", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");

        boolean hit = false;
        for (long a : found) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) hit = true;
        }
        check("gap '555:12:666' should match at 0x" + Long.toHexString(addr)
              + ", found=" + found.length, hit);

        Memory.writeDword(pid, addr, o1);
        Memory.writeDword(pid, addr + 16, o2);
    }

    private void testSearch_combinedFloatRadius() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;
        int o1 = Memory.readDword(pid, addr);
        float o2 = Memory.readFloat(pid, addr + 4);
        int o3 = Memory.readDword(pid, addr + 8);

        // 写入与目标值略有偏差的浮点, 验证半径匹配
        Memory.writeDword(pid, addr, 111);
        Memory.writeFloat(pid, addr + 4, 2.4999f);
        Memory.writeDword(pid, addr + 8, 222);

        Log.d(TAG, "  searching '111;f::2.5~0.01;222' at 0x" + Long.toHexString(addr));
        long[] found = Memory.search("111;f::2.5~0.01;222", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");

        boolean hit = false;
        for (long a : found) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) hit = true;
        }
        check("radius '111;f::2.5~0.01;222' should match 2.4999f at 0x" + Long.toHexString(addr)
              + ", found=" + found.length, hit);

        Memory.writeDword(pid, addr, o1);
        Memory.writeFloat(pid, addr + 4, o2);
        Memory.writeDword(pid, addr + 8, o3);
    }

    private void testSearch_combinedHex() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;
        int o1 = Memory.readDword(pid, addr);
        int o2 = Memory.readDword(pid, addr + 4);

        Memory.writeDword(pid, addr, 0x6F); // 111
        Memory.writeDword(pid, addr + 4, 222);

        Log.d(TAG, "  searching '0x6F;222' at 0x" + Long.toHexString(addr));
        long[] found = Memory.search("0x6F;222", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");

        boolean hit = false;
        for (long a : found) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) hit = true;
        }
        check("hex '0x6F;222' should match at 0x" + Long.toHexString(addr)
              + ", found=" + found.length, hit);

        Memory.writeDword(pid, addr, o1);
        Memory.writeDword(pid, addr + 4, o2);
    }

    private void testSearch_combinedInvalid() {
        Log.d(TAG, "  searching invalid expressions, all should return empty without crash");

        long[] a = Memory.search("abc", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  'abc' found " + a.length);
        check("unparseable token 'abc' should return empty, got " + a.length, a != null && a.length == 0);

        long[] b = Memory.search("111;;222", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  '111;;222' found " + b.length);
        check("empty token '111;;222' should return empty, got " + b.length, b != null && b.length == 0);

        long[] c = Memory.search("", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  '' found " + c.length);
        check("empty expression should return empty, got " + c.length, c != null && c.length == 0);

        long[] d = Memory.search("555:20:", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  '555:20:' found " + d.length);
        check("dangling gap '555:20:' should return empty, got " + d.length, d != null && d.length == 0);
    }

    private void testSearch_combinedSpanReject() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;
        int o1 = Memory.readDword(pid, addr);
        int o2 = Memory.readDword(pid, addr + 4);

        Memory.writeDword(pid, addr, 555);
        Memory.writeDword(pid, addr + 4, 666);

        // 组宽度 8 > span 4, 解析期即应拒绝 → 返回空
        Log.d(TAG, "  searching '555;666:4' (width 8 > span 4) at 0x" + Long.toHexString(addr));
        long[] found = Memory.search("555;666:4", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  found " + found.length + " results");
        check("span narrower than group '555;666:4' should return empty, got " + found.length,
              found != null && found.length == 0);

        // 同值但 span 足够 → 应命中
        Log.d(TAG, "  searching '555;666:20' at 0x" + Long.toHexString(addr));
        long[] ok = Memory.search("555;666:20", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  ok found " + ok.length + " results");
        boolean hit = false;
        for (long a : ok) { if (a == addr) hit = true; }
        check("span sufficient '555;666:20' should match at 0x" + Long.toHexString(addr)
              + ", found=" + ok.length, hit);

        Memory.writeDword(pid, addr, o1);
        Memory.writeDword(pid, addr + 4, o2);
    }

    private void testSearch_combinedRefineMultiGroup() {
        int pid = android.os.Process.myPid();
        MemoryValueSet results = Memory.searchDword(pid, 0, Memory.RANGE_ANONYMOUS);
        if (results.isEmpty()) { Log.d(TAG, "no results, skip"); return; }

        long addr = results.get(0).getAddress();
        addr &= ~3L;
        int o1 = Memory.readDword(pid, addr);
        int o2 = Memory.readDword(pid, addr + 4);
        int o3 = Memory.readDword(pid, addr + 8);
        int o4 = Memory.readDword(pid, addr + 12);

        Memory.writeDword(pid, addr, 111);
        Memory.writeDword(pid, addr + 4, 222);
        Memory.writeDword(pid, addr + 8, 333);
        Memory.writeDword(pid, addr + 12, 444);

        Log.d(TAG, "  first search '111;222' at 0x" + Long.toHexString(addr));
        long[] first = Memory.search("111;222", Memory.RANGE_ANONYMOUS);
        Log.d(TAG, "  first found " + first.length + " results");
        check("first search should find results", first != null && first.length > 0);

        Log.d(TAG, "  refine '111;222||333;444' over " + first.length + " addresses");
        long[] refined = Memory.search("111;222||333;444", first);
        Log.d(TAG, "  refined found " + refined.length + " results");
        boolean hit = false;
        for (long a : refined) {
            Log.d(TAG, "    0x" + Long.toHexString(a));
            if (a == addr) hit = true;
        }
        check("multi-group refine should keep 0x" + Long.toHexString(addr)
              + ", refined=" + refined.length, hit);

        Memory.writeDword(pid, addr, o1);
        Memory.writeDword(pid, addr + 4, o2);
        Memory.writeDword(pid, addr + 8, o3);
        Memory.writeDword(pid, addr + 12, o4);
    }
}
