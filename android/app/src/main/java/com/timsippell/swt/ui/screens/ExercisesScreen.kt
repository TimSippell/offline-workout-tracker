package com.timsippell.swt.ui.screens

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.timsippell.swt.bridge.SwtBridge

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ExercisesScreen() {
    var exercises by remember { mutableStateOf(SwtBridge.listExercises()) }
    var showAddDialog by remember { mutableStateOf(false) }
    var editingExercise by remember { mutableStateOf<SwtBridge.Exercise?>(null) }

    Scaffold(
        topBar = { TopAppBar(title = { Text("Exercises") }) },
        floatingActionButton = {
            FloatingActionButton(onClick = { showAddDialog = true }) {
                Icon(Icons.Default.Add, contentDescription = "Add exercise")
            }
        }
    ) { padding ->
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            items(exercises, key = { it.id }) { exercise ->
                ExerciseCard(
                    exercise = exercise,
                    onClick = { editingExercise = exercise },
                    onDelete = {
                        SwtBridge.deleteExercise(exercise.id)
                        exercises = SwtBridge.listExercises()
                    }
                )
            }
            if (exercises.isEmpty()) {
                item {
                    Text(
                        "No exercises yet. Tap + to add one.",
                        modifier = Modifier.padding(32.dp),
                        style = MaterialTheme.typography.bodyLarge
                    )
                }
            }
        }
    }

    if (showAddDialog) {
        ExerciseDialog(
            title = "Add Exercise",
            onDismiss = { showAddDialog = false },
            onConfirm = { name, category, muscle, trackingType ->
                SwtBridge.addExercise(name, category, muscle, trackingType)
                exercises = SwtBridge.listExercises()
                showAddDialog = false
            }
        )
    }

    editingExercise?.let { exercise ->
        ExerciseDialog(
            title = "Edit Exercise",
            initialName = exercise.name,
            initialCategory = exercise.category,
            initialMuscle = exercise.muscleGroup,
            initialTrackingType = if (exercise.notes == "time") "time" else "weight",
            onDismiss = { editingExercise = null },
            onConfirm = { name, category, muscle, trackingType ->
                SwtBridge.updateExercise(exercise.id, name, category, muscle, trackingType)
                exercises = SwtBridge.listExercises()
                editingExercise = null
            }
        )
    }
}

@Composable
private fun ExerciseCard(
    exercise: SwtBridge.Exercise,
    onClick: () -> Unit,
    onDelete: () -> Unit
) {
    Card(modifier = Modifier.fillMaxWidth().clickable(onClick = onClick)) {
        Row(
            modifier = Modifier.padding(16.dp).fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(exercise.name, style = MaterialTheme.typography.titleMedium)
                val subtitle = buildString {
                    if (exercise.category.isNotEmpty()) append("${exercise.category} • ${exercise.muscleGroup}")
                    val type = if (exercise.notes == "time") "Time" else "Weight"
                    if (isNotEmpty()) append(" • ")
                    append(type)
                }
                Text(subtitle, style = MaterialTheme.typography.bodySmall)
            }
            IconButton(onClick = onDelete) {
                Icon(Icons.Default.Delete, contentDescription = "Delete")
            }
        }
    }
}

@Composable
private fun ExerciseDialog(
    title: String,
    initialName: String = "",
    initialCategory: String = "",
    initialMuscle: String = "",
    initialTrackingType: String = "weight",
    onDismiss: () -> Unit,
    onConfirm: (String, String, String, String) -> Unit
) {
    var name by remember { mutableStateOf(initialName) }
    var category by remember { mutableStateOf(initialCategory) }
    var muscle by remember { mutableStateOf(initialMuscle) }
    var trackingType by remember { mutableStateOf(initialTrackingType) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(title) },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedTextField(value = name, onValueChange = { name = it }, label = { Text("Name") })
                OutlinedTextField(value = category, onValueChange = { category = it }, label = { Text("Category") })
                OutlinedTextField(value = muscle, onValueChange = { muscle = it }, label = { Text("Muscle Group") })
                Text("Tracking type", style = MaterialTheme.typography.labelMedium)
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    FilterChip(
                        selected = trackingType == "weight",
                        onClick = { trackingType = "weight" },
                        label = { Text("Weight") }
                    )
                    FilterChip(
                        selected = trackingType == "time",
                        onClick = { trackingType = "time" },
                        label = { Text("Time") }
                    )
                }
            }
        },
        confirmButton = {
            TextButton(onClick = { onConfirm(name, category, muscle, trackingType) }, enabled = name.isNotBlank()) {
                Text("Save")
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) { Text("Cancel") }
        }
    )
}
