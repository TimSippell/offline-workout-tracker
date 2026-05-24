package com.swt.ui.screens

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
import androidx.compose.ui.unit.dp
import com.swt.bridge.SwtBridge

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun WorkoutScreen() {
    var activeWorkoutId by remember { mutableStateOf<Long?>(null) }
    var sets by remember { mutableStateOf<List<SwtBridge.WorkoutSet>>(emptyList()) }
    var exercises by remember { mutableStateOf(SwtBridge.listExercises()) }
    var showAddSet by remember { mutableStateOf(false) }

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
                    Text("Start Workout")
                }
            } else {
                LazyColumn(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                    items(sets) { set ->
                        SetCard(set, exercises)
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
                refreshSets()
                showAddSet = false
            }
        )
    }
}

@Composable
private fun SetCard(set: SwtBridge.WorkoutSet, exercises: List<SwtBridge.Exercise>) {
    val exerciseName = exercises.find { it.id == set.exerciseId }?.name ?: "Unknown"
    Card(modifier = Modifier.fillMaxWidth()) {
        Row(modifier = Modifier.padding(16.dp), verticalAlignment = Alignment.CenterVertically) {
            Column(modifier = Modifier.weight(1f)) {
                Text(exerciseName, style = MaterialTheme.typography.titleSmall)
                Text(
                    buildString {
                        if (set.reps > 0) append("${set.reps} reps")
                        if (set.weight > 0) append(" × ${set.weight} kg")
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
    var selectedExercise by remember { mutableStateOf(exercises.firstOrNull()) }
    var reps by remember { mutableStateOf("") }
    var weight by remember { mutableStateOf("") }
    var rpe by remember { mutableStateOf("") }
    var expanded by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Log Set") },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                ExposedDropdownMenuBox(expanded = expanded, onExpandedChange = { expanded = it }) {
                    OutlinedTextField(
                        value = selectedExercise?.name ?: "Select exercise",
                        onValueChange = {},
                        readOnly = true,
                        modifier = Modifier.menuAnchor(),
                        label = { Text("Exercise") }
                    )
                    ExposedDropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }) {
                        exercises.forEach { ex ->
                            DropdownMenuItem(
                                text = { Text(ex.name) },
                                onClick = {
                                    selectedExercise = ex
                                    expanded = false
                                }
                            )
                        }
                    }
                }
                OutlinedTextField(value = reps, onValueChange = { reps = it }, label = { Text("Reps") })
                OutlinedTextField(value = weight, onValueChange = { weight = it }, label = { Text("Weight (kg)") })
                OutlinedTextField(value = rpe, onValueChange = { rpe = it }, label = { Text("RPE (optional)") })
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    selectedExercise?.let {
                        onConfirm(it.id, reps.toIntOrNull() ?: 0, weight.toDoubleOrNull() ?: 0.0, rpe.toDoubleOrNull() ?: 0.0)
                    }
                },
                enabled = selectedExercise != null && reps.isNotBlank()
            ) { Text("Add") }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } }
    )
}
