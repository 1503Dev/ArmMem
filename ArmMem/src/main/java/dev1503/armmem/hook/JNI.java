package dev1503.armmem.hook;

public class JNI {
//    public static native void hook(long address, HookOnInvokeListener listener);
    public static native long getFunctionAddress(String moduleName, String functionName);
}
