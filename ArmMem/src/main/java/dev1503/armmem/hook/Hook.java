package dev1503.armmem.hook;

public class Hook {
//    public static void hook(long address, HookOnInvokeListener listener) {
//        JNI.hook(address, listener);
//    }
    public static long getFunctionAddress(String moduleName, String functionName) {
        return JNI.getFunctionAddress(moduleName, functionName);
    }
}
