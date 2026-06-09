package com.timsippell.owt

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import com.timsippell.owt.bridge.OwtBridge
import com.timsippell.owt.ui.OwtApp

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        OwtBridge.init(applicationContext)
        enableEdgeToEdge()
        setContent {
            OwtApp()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        OwtBridge.close()
    }
}
