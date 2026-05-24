package com.timsippell.swt

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import com.timsippell.swt.bridge.SwtBridge
import com.timsippell.swt.ui.SwtApp

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        SwtBridge.init(applicationContext)
        enableEdgeToEdge()
        setContent {
            SwtApp()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        SwtBridge.close()
    }
}
