package dev1503.armmem.app

import android.os.Bundle
import android.os.Process
import android.util.Log
import android.view.View
import android.widget.TextView
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import dev1503.armmem.ArmMem
import dev1503.armmem.hook.Hook
import dev1503.armmem.memory.Memory
import dev1503.armmem.memory.MemoryValueSet

class MainActivity : AppCompatActivity() {
    lateinit var tv: TextView
    lateinit var tv2: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }
        tv = findViewById(R.id.text)
        tv2 = findViewById(R.id.i)
        findViewById<TextView>(R.id.abi).text = if (Process.is64Bit()) "arm64-v8a" else "armeabi-v7a"
    }

    fun hook(v: View) {
        JNI().hook()
    }
    fun unhook(v: View) {
        JNI().unHook()
    }
    fun getValue(v: View) {
        tv.text = JNI().stringFromJNI()
    }

    fun geti(v: View) {
        tv2.text = JNI().geti().toString()
    }
    var memoryValueSet: MemoryValueSet? = null
    var addr: Long = 0
    fun modi(v: View) {
        var rez = Memory.searchDword(1145141919, Memory.RANGE_C_DATA)
        memoryValueSet = rez
        if (rez.isNotEmpty()) {
            addr = rez.get(0)?.address ?: 0
            Log.i("ArmMem", "modi: $addr")

        }
        JNI().modi()
    }
    fun modi2(v: View) {
        JNI().modi2(addr)
        Log.i("ArmMem", "modi2: $addr")
    }
    fun search(v: View) {
//        if (memoryValueSet == null) {
//            return
//        }
        val rez1 = Memory.searchDword(21, Memory.RANGE_C_DATA)
        for (i in rez1 ?: emptyList()) {
            Log.i("ArmMem", "searchDword: ${i.address}, ${i.readDword()}")
        }
        val rez = Memory.searchByte(21, Memory.RANGE_C_DATA)
        for (i in rez ?: emptyList()) {
            Log.i("ArmMem", "searchByte: ${i.address}, ${i.readByte()}")
        }
    }
}