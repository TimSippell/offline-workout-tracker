package com.timsippell.swt.ui.screens

import androidx.compose.foundation.ExperimentalFoundationApi
import androidx.compose.foundation.combinedClickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import com.timsippell.swt.bridge.SwtBridge

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun WorkoutScreen(navController: NavController) {
    val context = LocalContext.current
    val weightUnit = remember { AppSettings.getWeightUnit(context) }
    var activeWorkoutId by remember { mutableStateOf<Long?>(null) }
    var sets by remember { mutableStateOf<List<SwtBridge.WorkoutSet>>(emptyList()) }
    var exercises by remember { mutableStateOf(SwtBridge.listExercises()) }
    var showAddSet by remember { mutableStateOf(false) }
    var showTemplatePicker by remember { mutableStateOf(false) }
    var editingSet by remember { mutableStateOf<SwtBridge.WorkoutSet?>(null) }
    var completedSets by remember { mutableStateOf(setOf<Long>()) }

    fun refreshSets() {
        activeWorkoutId?.let { sets = SwtBridge.getSetsForWorkout(it) }
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(if (activeWorkoutId != null) "Active Workout" else "Workout") },
                actions = {
                    if (activeWorkoutId != null) {
                        IconButton(onClick = {
                            SwtBridge.finishWorkout(activeWorkoutId!!)
                            activeWorkoutId = null
                            sets = emptyList()
                        }) {
                            Icon(Icons.Default.Check, contentDescription = "Finish")
                        }
                    }
                }
            )
        },
        floatingActionButton = {
            if (activeWorkoutId != null) {
                FloatingActionButton(onClick = { showAddSet = true }) {
                    Icon(Icons.Default.Add, contentDescription = "Add set")
                }
            }
        }
    ) { padding ->
        Column(
            modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            if (activeWorkoutId == null) {
                Button(
                    onClick = {
                        activeWorkoutId = SwtBridge.startWorkout("")
                        exercises = SwtBridge.listExercises()
                    },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(Icons.Default.PlayArrow, contentDescription = null)
                    Spacer(Modifier.width(8.dp))
                    Text("Start Blank Workout")
                }

                OutlinedButton(
                    onClick = { showTemplatePicker = true },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(Icons.Default.PlayArrow, contentDescription = null)
                    Spacer(Modifier.width(8.dp))
                    Text("Start from Template")
                }

                Spacer(Modifier.height(8.dp))

                TextButton(
                    onClick = { navController.navigate("templates") },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("Manage Templates")
                }
            } else {
                LazyColumn(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                    items(sets) { set ->
                        SetCard(
                            set = set,
                            exercises = exercises,
                            weightUnit = weightUnit,
                            completed = set.id in completedSets,
                            onClick = {
                                completedSets = if (set.id in completedSets)
                                    completedSets - set.id
                                else
                                    completedSets + set.id
                            },
                            onLongClick = { editingSet = set }
                        )
                    }
                    if (sets.isEmpty()) {
                        item {
                            Text(
                                "No sets yet. Tap + to log a set.",
                                style = MaterialTheme.typography.bodyLarge,
                                modifier = Modifier.padding(16.dp)
                            )
                        }
                    }
                }
            }
        }
    }

    if (showAddSet && activeWorkoutId != null) {
        AddSetDialog(
            exercises = exercises,
            onDismiss = { showAddSet = false },
            onConfirm = { exerciseId, reps, weight, rpe ->
                SwtBridge.addSet(activeWorkoutId!!, exerciseId, sets.size + 1, reps, weight, rpe)
                exercises = SwtBridge.listExercises()
                refreshSets()
                showAddSet = false
            }
        )
    }

    if (showTemplatePicker) {
        TemplatePickerDialog(
            onDismiss = { showTemplatePicker = false },
            onSelect = { templateId ->
                activeWorkoutId = SwtBridge.startWorkoutFromTemplate(templateId)
                exercises = SwtBridge.listExercises()
                refreshSets()
                showTemplatePicker = false
            }
        )
    }

    editingSet?.let { set ->
        EditSetDialog(
            set = set,
            exercises = exercises,
            onDismiss = { editingSet = null },
            onConfirm = { reps, weight, rpe ->
                SwtBridge.updateSet(set.id, reps, weight, rpe)
                refreshSets()
                editingSet = null
            }
        )
    }
}

@OptIn(ExperimentalFoundationApi::class)
@Composable
private fun SetCard(
    set: SwtBridge.WorkoutSet,
    exercises: List<SwtBridge.Exercise>,
    weightUnit: String,
    completed: Boolean,
    onClick: () -> Unit,
    onLongClick: () -> Unit
) {
    val exercise = exercises.find { it.id == set.exerciseId }
    val exerciseName = exercise?.name ?: "Unknown"
    val isTimeExercise = exercise?.notes == "time"

    val cardColors = when {
        completed -> CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.primaryContainer
        )
        set.weight == 0.0 -> CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.secondaryContainer
        )
        else -> CardDefaults.cardColors()
    }

    Card(
        modifier = Modifier.fillMaxWidth().combinedClickable(
            onClick = onClick,
            onLongClick = onLongClick
        ),
        colors = cardColors
    ) {
        Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
            Column(modifier = Modifier.weight(1f)) {
                Text(exerciseName, style = MaterialTheme.typography.titleSmall)
                Text(
                    buildString {
                        if (set.reps > 0) append("${set.reps} reps")
                        if (set.weight > 0) {
                            if (isTimeExercise) append(" × ${set.weight.toInt()}s")
                            else append(" × ${set.weight} $weightUnit")
                        } else if (set.reps > 0) append(" — tap to enter weight")
                        if (set.rpe > 0) append(" @RPE ${set.rpe}")
                    },
                    style = MaterialTheme.typography.bodyMedium
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun AddSetDialog(
    exercises: List<SwtBridge.Exercise>,
    onDismiss: () -> Unit,
    onConfirm: (Long, Int, Double, Double) -> Unit
) {
    val context = LocalContext.current
    val weightUnit = remember { AppSettings.getWeightUnit(context) }
    var exerciseName by remember { mutableStateOf("") }
    var reps by remember { mutableStateOf("") }
    var weight by remember { mutableStateOf("") }
    var rpe by remember { mutableStateOf("") }
    var expanded by remember { mutableStateOf(false) }

    val suggestions = remember(exerciseName) {
        if (exerciseName.isBlank()) emptyList()
        else exercises.filter { it.name.contains(exerciseName, ignoreCase = true) }
    }

    val matchedExercise = remember(exerciseName) {
        exercises.find { it.name.equals(exerciseName, ignoreCase = true) }
    }
    val isTimeExercise = matchedExercise?.notes == "time"

    fun resolveExerciseId(): Long {
        if (matchedExercise != null) return matchedExercise.id
        return SwtBridge.addExercise(exerciseName, "", "", "weight")
    }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Log Set") },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                ExposedDropdownMenuBox(
                    expanded = expanded && suggestions.isNotEmpty(),
                    onExpandedChange = { expanded = it }
                ) {
                    OutlinedTextField(
                        value = exerciseName,
                        onValueChange = { exerciseName = it; expanded = true },
                        modifier = Modifier.menuAnchor(),
                        label = { Text("Exercise") },
                        singleLine = true
                    )
                    ExposedDropdownMenu(
                        expanded = expanded && suggestions.isNotEmpty(),
                        onDismissRequest = { expanded = false }
                    ) {
                        suggestions.forEach { ex ->
                            DropdownMenuItem(
                                text = { Text(ex.name) },
                                onClick = {
                                    exerciseName = ex.name
                                    expanded = false
                                }
                            )
                        }
                    }
                }
                OutlinedTextField(value = reps, onValueChange = { reps = it }, label = { Text("Reps") })
                if (isTimeExercise) {
                    OutlinedTextField(value = weight, onValueChange = { weight = it }, label = { Text("Time (seconds)") })
                } else {
                    OutlinedTextField(value = weight, onValueChange = { weight = it }, label = { Text("Weight ($weightUnit)") })
                }
                OutlinedTextField(value = rpe, onValueChange = { rpe = it }, label = { Text("RPE (optional)") })
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    val id = resolveExerciseId()
                    onConfirm(id, reps.toIntOrNull() ?: 0, weight.toDoubleOrNull() ?: 0.0, rpe.toDoubleOrNull() ?: 0.0)
                },
                enabled = exerciseName.isNotBlank() && reps.isNotBlank()
            ) { Text("Add") }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } }
    )
}

@Composable
private fun EditSetDialog(
    set: SwtBridge.WorkoutSet,
    exercises: List<SwtBridge.Exercise>,
    onDismiss: () -> Unit,
    onConfirm: (Int, Double, Double) -> Unit
) {
    val context = LocalContext.current
    val weightUnit = remember { AppSettings.getWeightUnit(context) }
    val exercise = exercises.find { it.id == set.exerciseId }
    val exerciseName = exercise?.name ?: "Unknown"
    val isTimeExercise = exercise?.notes == "time"
    var reps by remember { mutableStateOf(if (set.reps > 0) set.reps.toString() else "") }
    var weight by remember { mutableStateOf(if (set.weight > 0) set.weight.toString() else "") }
    var rpe by remember { mutableStateOf(if (set.rpe > 0) set.rpe.toString() else "") }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(exerciseName) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedTextField(value = reps, onValueChange = { reps = it }, label = { Text("Reps") })
                if (isTimeExercise) {
                    OutlinedTextField(value = weight, onValueChange = { weight = it }, label = { Text("Time (seconds)") })
                } else {
                    OutlinedTextField(value = weight, onValueChange = { weight = it }, label = { Text("Weight ($weightUnit)") })
                }
                OutlinedTextField(value = rpe, onValueChange = { rpe = it }, label = { Text("RPE (optional)") })
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    onConfirm(
                        reps.toIntOrNull() ?: 0,
                        weight.toDoubleOrNull() ?: 0.0,
                        rpe.toDoubleOrNull() ?: 0.0
                    )
                }
            ) { Text("Save") }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } }
    )
}

@Composable
private fun TemplatePickerDialog(
    onDismiss: () -> Unit,
    onSelect: (Long) -> Unit
) {
    val templates = remember { SwtBridge.listTemplates() }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Choose Template") },
        text = {
            if (templates.isEmpty()) {
                Text("No templates yet. Create one first.")
            } else {
                LazyColumn(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    items(templates) { template ->
                        TextButton(
                            onClick = { onSelect(template.id) },
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Column(modifier = Modifier.fillMaxWidth()) {
                                Text(template.name, style = MaterialTheme.typography.titleSmall)
                                Text("${template.setCount} sets", style = MaterialTheme.typography.bodySmall)
                            }
                        }
                    }
                }
            }
        },
        confirmButton = {},
        dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } }
    )
}
