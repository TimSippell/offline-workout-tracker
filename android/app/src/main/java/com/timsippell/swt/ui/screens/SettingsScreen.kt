package com.timsippell.swt.ui.screens

import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.timsippell.swt.bridge.SwtBridge
import org.json.JSONArray
import org.json.JSONObject

object AppSettings {
    private const val PREFS_NAME = "swt_settings"
    private const val KEY_WEIGHT_UNIT = "weight_unit"
    private const val KEY_SETUP_COMPLETE = "setup_complete"
    private const val KEY_1RM_PREFIX = "1rm_"

    fun getPrefs(context: Context): SharedPreferences =
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    fun getWeightUnit(context: Context): String =
        getPrefs(context).getString(KEY_WEIGHT_UNIT, "kg") ?: "kg"

    fun setWeightUnit(context: Context, unit: String) {
        getPrefs(context).edit().putString(KEY_WEIGHT_UNIT, unit).apply()
    }

    fun isSetupComplete(context: Context): Boolean =
        getPrefs(context).getBoolean(KEY_SETUP_COMPLETE, false)

    fun setSetupComplete(context: Context, complete: Boolean) {
        getPrefs(context).edit().putBoolean(KEY_SETUP_COMPLETE, complete).apply()
    }

    fun getOneRepMax(context: Context, exerciseId: Long): Double =
        getPrefs(context).getFloat("$KEY_1RM_PREFIX$exerciseId", 0f).toDouble()

    fun setOneRepMax(context: Context, exerciseId: Long, weight: Double) {
        getPrefs(context).edit().putFloat("$KEY_1RM_PREFIX$exerciseId", weight.toFloat()).apply()
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(onNavigateToSetup: () -> Unit = {}) {
    val context = LocalContext.current
    var weightUnit by remember { mutableStateOf(AppSettings.getWeightUnit(context)) }
    var showResetDialog by remember { mutableStateOf(false) }
    var exportMessage by remember { mutableStateOf<String?>(null) }

    val exportLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.CreateDocument("application/json")
    ) { uri ->
        if (uri != null) {
            try {
                val json = exportDataToJson()
                context.contentResolver.openOutputStream(uri)?.use { stream ->
                    stream.write(json.toByteArray())
                }
                exportMessage = "Data exported successfully"
            } catch (e: Exception) {
                exportMessage = "Export failed: ${e.message}"
            }
        }
    }

    Scaffold(
        topBar = { TopAppBar(title = { Text("Settings") }) }
    ) { padding ->
        Column(
            modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(24.dp)
        ) {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
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

            HorizontalDivider()

            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text("Setup", style = MaterialTheme.typography.titleMedium)
                OutlinedButton(
                    onClick = onNavigateToSetup,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("Set Up One Rep Max")
                }
            }

            HorizontalDivider()

            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text("Data", style = MaterialTheme.typography.titleMedium)
                OutlinedButton(
                    onClick = { exportLauncher.launch("swt-export.json") },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("Export Data")
                }
                exportMessage?.let {
                    Text(it, style = MaterialTheme.typography.bodySmall)
                }
            }

            HorizontalDivider()

            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text("Danger Zone", style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.error)
                OutlinedButton(
                    onClick = { showResetDialog = true },
                    modifier = Modifier.fillMaxWidth(),
                    colors = ButtonDefaults.outlinedButtonColors(
                        contentColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Text("Reset All Data")
                }
            }
        }
    }

    if (showResetDialog) {
        AlertDialog(
            onDismissRequest = { showResetDialog = false },
            title = { Text("Reset All Data") },
            text = { Text("This will permanently delete all exercises, workouts, templates, and history. This cannot be undone.") },
            confirmButton = {
                TextButton(
                    onClick = {
                        val dbFile = context.getDatabasePath("swt.db")
                        SwtBridge.close()
                        dbFile.delete()
                        SwtBridge.init(context)
                        showResetDialog = false
                    },
                    colors = ButtonDefaults.textButtonColors(
                        contentColor = MaterialTheme.colorScheme.error
                    )
                ) { Text("Delete Everything") }
            },
            dismissButton = {
                TextButton(onClick = { showResetDialog = false }) { Text("Cancel") }
            }
        )
    }
}

private fun exportDataToJson(): String {
    val root = JSONObject()

    val exercisesArr = JSONArray()
    SwtBridge.listExercises().forEach { ex ->
        exercisesArr.put(JSONObject().apply {
            put("id", ex.id)
            put("name", ex.name)
            put("category", ex.category)
            put("muscleGroup", ex.muscleGroup)
            put("type", ex.notes)
        })
    }
    root.put("exercises", exercisesArr)

    val workoutsArr = JSONArray()
    SwtBridge.listWorkouts(limit = 10000).forEach { workout ->
        val wo = JSONObject().apply {
            put("id", workout.id)
            put("name", workout.name)
            put("startedAt", workout.startedAt)
            put("finishedAt", workout.finishedAt)
        }
        val setsArr = JSONArray()
        SwtBridge.getSetsForWorkout(workout.id).forEach { set ->
            setsArr.put(JSONObject().apply {
                put("exerciseId", set.exerciseId)
                put("order", set.order)
                put("reps", set.reps)
                put("weight", set.weight)
                put("rpe", set.rpe)
            })
        }
        wo.put("sets", setsArr)
        workoutsArr.put(wo)
    }
    root.put("workouts", workoutsArr)

    val templatesArr = JSONArray()
    SwtBridge.listTemplates().forEach { tmpl ->
        val t = JSONObject().apply {
            put("id", tmpl.id)
            put("name", tmpl.name)
        }
        val setsArr = JSONArray()
        SwtBridge.getTemplateSets(tmpl.id).forEach { set ->
            setsArr.put(JSONObject().apply {
                put("exerciseId", set.exerciseId)
                put("order", set.order)
                put("reps", set.reps)
                put("weight", set.weight)
                put("rpe", set.rpe)
            })
        }
        t.put("sets", setsArr)
        templatesArr.put(t)
    }
    root.put("templates", templatesArr)

    return root.toString(2)
}
