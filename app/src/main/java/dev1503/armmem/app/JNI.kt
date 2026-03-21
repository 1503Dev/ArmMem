package dev1503.armmem.app

class JNI {
    external fun stringFromJNI(): String
    external fun hook()
    external fun unHook()
    external fun geti(): Int
    external fun modi(): Int
    companion object {
        init {
            System.loadLibrary("app")
        }
    }
}