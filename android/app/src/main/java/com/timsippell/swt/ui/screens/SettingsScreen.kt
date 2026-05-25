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
    private const val KG_TO_LBS = 2.20462

    fun getPrefs(context: Context): SharedPreferences =
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    fun getWeightUnit(context: Context): String =
        getPrefs(context).getString(KEY_WEIGHT_UNIT, "kg") ?: "kg"

    fun setWeightUnit(context: Context, unit: String) {
        getPrefs(context).edit().putString(KEY_WEIGHT_UNIT, unit).apply()
    }

    fun toDisplayWeight(storedKg: Double, context: Context): Double =
        if (getWeightUnit(context) == "lbs") storedKg * KG_TO_LBS else storedKg

    fun toStorageWeight(displayValue: Double, context: Context): Double =
        if (getWeightUnit(context) == "lbs") displayValue / KG_TO_LBS else displayValue

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
    var importMessage by remember { mutableStateOf<String?>(null) }
    var pendingImportJson by remember { mutableStateOf<String?>(null) }
    var importSummary by remember { mutableStateOf<ImportSummary?>(null) }

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

    val importLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.OpenDocument()
    ) { uri ->
        if (uri != null) {
            try {
                val json = context.contentResolver.openInputStream(uri)?.use { stream ->
                    stream.bufferedReader().readText()
                } ?: throw Exception("Could not read file")
                val summary = previewImport(json)
                pendingImportJson = json
                importSummary = summary
            } catch (e: Exception) {
                importMessage = "Import failed: ${e.message}"
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
                OutlinedButton(
                    onClick = { importLauncher.launch(arrayOf("application/json")) },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("Import Data")
                }
                importMessage?.let {
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

    importSummary?.let { summary ->
        AlertDialog(
            onDismissRequest = {
                importSummary = null
                pendingImportJson = null
            },
            title = { Text("Import Data") },
            text = {
                Text(buildString {
                    append("This will import:\n")
                    append("• ${summary.newExercises} new exercises")
                    if (summary.existingExercises > 0) append(" (${summary.existingExercises} already exist)")
                    append("\n• ${summary.workouts} workouts with ${summary.workoutSets} sets")
                    append("\n• ${summary.templates} templates with ${summary.templateSets} sets")
                    append("\n\nExisting data will not be modified.")
                })
            },
            confirmButton = {
                TextButton(onClick = {
                    try {
                        val result = importDataFromJson(pendingImportJson!!)
                        importMessage = result
                    } catch (e: Exception) {
                        importMessage = "Import failed: ${e.message}"
                    }
                    importSummary = null
                    pendingImportJson = null
                }) { Text("Import") }
            },
            dismissButton = {
                TextButton(onClick = {
                    importSummary = null
                    pendingImportJson = null
                }) { Text("Cancel") }
            }
        )
    }
}

private data class ImportSummary(
    val newExercises: Int,
    val existingExercises: Int,
    val workouts: Int,
    val workoutSets: Int,
    val templates: Int,
    val templateSets: Int
)

private fun previewImport(json: String): ImportSummary {
    val root = JSONObject(json)
    val existingExercises = SwtBridge.listExercises().associateBy { it.name }

    val exercises = root.optJSONArray("exercises") ?: JSONArray()
    var newCount = 0
    var existingCount = 0
    for (i in 0 until exercises.length()) {
        val name = exercises.getJSONObject(i).getString("name")
        if (existingExercises.containsKey(name)) existingCount++ else newCount++
    }

    val workouts = root.optJSONArray("workouts") ?: JSONArray()
    var workoutSets = 0
    for (i in 0 until workouts.length()) {
        workoutSets += workouts.getJSONObject(i).optJSONArray("sets")?.length() ?: 0
    }

    val templates = root.optJSONArray("templates") ?: JSONArray()
    var templateSets = 0
    for (i in 0 until templates.length()) {
        templateSets += templates.getJSONObject(i).optJSONArray("sets")?.length() ?: 0
    }

    return ImportSummary(newCount, existingCount, workouts.length(), workoutSets, templates.length(), templateSets)
}

private fun importDataFromJson(json: String): String {
    val root = JSONObject(json)
    val existingExercises = SwtBridge.listExercises().associateBy { it.name }.toMutableMap()

    val exerciseIdMap = mutableMapOf<Long, Long>()
    val exercises = root.optJSONArray("exercises") ?: JSONArray()
    for (i in 0 until exercises.length()) {
        val obj = exercises.getJSONObject(i)
        val oldId = obj.getLong("id")
        val name = obj.getString("name")
        val existing = existingExercises[name]
        if (existing != null) {
            exerciseIdMap[oldId] = existing.id
        } else {
            val newId = SwtBridge.addExercise(
                name,
                obj.optString("category", ""),
                obj.optString("muscleGroup", ""),
                obj.optString("type", "weight")
            )
            exerciseIdMap[oldId] = newId
        }
    }

    var workoutCount = 0
    var setCount = 0
    val workouts = root.optJSONArray("workouts") ?: JSONArray()
    for (i in 0 until workouts.length()) {
        val wo = workouts.getJSONObject(i)
        val workoutId = SwtBridge.startWorkout(wo.optString("name", ""))
        SwtBridge.finishWorkout(workoutId)
        val sets = wo.optJSONArray("sets") ?: JSONArray()
        for (j in 0 until sets.length()) {
            val s = sets.getJSONObject(j)
            val exerciseId = exerciseIdMap[s.getLong("exerciseId")] ?: continue
            SwtBridge.addSet(
                workoutId, exerciseId,
                s.optInt("order", j + 1),
                s.optInt("reps", 0),
                s.optDouble("weight", 0.0),
                s.optDouble("rpe", 0.0)
            )
            setCount++
        }
        workoutCount++
    }

    var templateCount = 0
    var templateSetCount = 0
    val templates = root.optJSONArray("templates") ?: JSONArray()
    for (i in 0 until templates.length()) {
        val t = templates.getJSONObject(i)
        val templateId = SwtBridge.createTemplate(t.optString("name", ""))
        val sets = t.optJSONArray("sets") ?: JSONArray()
        for (j in 0 until sets.length()) {
            val s = sets.getJSONObject(j)
            val exerciseId = exerciseIdMap[s.getLong("exerciseId")] ?: continue
            SwtBridge.addTemplateSet(
                templateId, exerciseId,
                s.optInt("order", j + 1),
                s.optInt("reps", 0),
                s.optDouble("weight", 0.0),
                s.optDouble("rpe", 0.0)
            )
            templateSetCount++
        }
        templateCount++
    }

    return "Imported $workoutCount workouts, $templateCount templates, $setCount sets"
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
