package com.timsippell.swt.ui.screens

import android.content.Context
import android.content.SharedPreferences
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp

object AppSettings {
    private const val PREFS_NAME = "swt_settings"
    private const val KEY_WEIGHT_UNIT = "weight_unit"

    fun getPrefs(context: Context): SharedPreferences =
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    fun getWeightUnit(context: Context): String =
        getPrefs(context).getString(KEY_WEIGHT_UNIT, "kg") ?: "kg"

    fun setWeightUnit(context: Context, unit: String) {
        getPrefs(context).edit().putString(KEY_WEIGHT_UNIT, unit).apply()
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen() {
    val context = LocalContext.current
    var weightUnit by remember { mutableStateOf(AppSettings.getWeightUnit(context)) }

    Scaffold(
        topBar = { TopAppBar(title = { Text("Settings") }) }
    ) { padding ->
        Column(
            modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text("Weight Unit", style = MaterialTheme.typography.titleMedium)
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                FilterChip(
                    selected = weightUnit == "kg",
                    onClick = {
                        weightUnit = "kg"
                        AppSettings.setWeightUnit(context, "kg")
                    },
                    label = { Text("kg") }
                )
                FilterChip(
                    selected = weightUnit == "lbs",
                    onClick = {
                        weightUnit = "lbs"
                        AppSettings.setWeightUnit(context, "lbs")
                    },
                    label = { Text("lbs") }
                )
            }
        }
    }
}
