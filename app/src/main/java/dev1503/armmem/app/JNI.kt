package dev1503.armmem.app

class JNI {
    external fun stringFromJNI(): String
    external fun hook()
    external fun unHook()
    external fun geti(): Int
    external fun modi(): Int
    external fun modi2(addr: Long): Int
    external fun unlisWr(): Int
    external fun handleTest()
    external fun lisRd(addr: Long)
    external fun unlisRd()

    external fun testHook(): Boolean
    external fun testCallStringFromJNI(): String
    external fun testUnhook(): Boolean
    external fun testGetFunctionAddress(moduleName: String, symbolName: String): Long
    external fun testHookNullTarget(): Boolean
    external fun testHookNullFunc(): Boolean
    external fun getModi2Address(): Long
    external fun getDwordFloatAddress(): Long
    external fun getMixedGroupAddress(): Long

    companion object {
        init {
            System.loadLibrary("app")
        }
    }
}