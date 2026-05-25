package com.timsippell.swt.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import com.timsippell.swt.bridge.SwtBridge

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TemplateBuilderScreen(templateId: Long?, navController: NavController) {
    if (templateId == null) return

    var sets by remember { mutableStateOf(SwtBridge.getTemplateSets(templateId)) }
    val exercises = remember { SwtBridge.listExercises() }
    var showAddDialog by remember { mutableStateOf(false) }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Edit Template") },
                navigationIcon = {
                    IconButton(onClick = { navController.popBackStack() }) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                    }
                }
            )
        },
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
            items(sets, key = { it.id }) { set ->
                val exerciseName = exercises.find { it.id == set.exerciseId }?.name ?: "Unknown"
                TemplateSetCard(
                    exerciseName = exerciseName,
                    set = set,
                    onDelete = {
                        SwtBridge.deleteTemplateSet(set.id)
                        sets = SwtBridge.getTemplateSets(templateId)
                    }
                )
            }
            if (sets.isEmpty()) {
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
        AddTemplateSetDialog(
            exercises = exercises,
            onDismiss = { showAddDialog = false },
            onConfirm = { exerciseId, numSets, reps, weight, rpe ->
                for (i in 1..numSets) {
                    SwtBridge.addTemplateSet(
                        templateId, exerciseId,
                        sets.size + i, reps, weight, rpe
                    )
                }
                sets = SwtBridge.getTemplateSets(templateId)
                showAddDialog = false
            }
        )
    }
}

@Composable
private fun TemplateSetCard(
    exerciseName: String,
    set: SwtBridge.TemplateSet,
    onDelete: () -> Unit
) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier.padding(16.dp).fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(exerciseName, style = MaterialTheme.typography.titleSmall)
                Text(
                    buildString {
                        append("Set ${set.order}")
                        if (set.reps > 0) append(" • ${set.reps} reps")
                        if (set.weight > 0) append(" × ${set.weight}")
                        if (set.rpe > 0) append(" @RPE ${set.rpe}")
                    },
                    style = MaterialTheme.typography.bodyMedium
                )
            }
            IconButton(onClick = onDelete) {
                Icon(Icons.Default.Delete, contentDescription = "Remove")
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun AddTemplateSetDialog(
    exercises: List<SwtBridge.Exercise>,
    onDismiss: () -> Unit,
    onConfirm: (Long, Int, Int, Double, Double) -> Unit
) {
    val context = LocalContext.current
    val weightUnit = remember { AppSettings.getWeightUnit(context) }
    var selectedExercise by remember { mutableStateOf(exercises.firstOrNull()) }
    var numSets by remember { mutableStateOf("3") }
    var reps by remember { mutableStateOf("") }
    var weight by remember { mutableStateOf("") }
    var rpe by remember { mutableStateOf("") }
    var expanded by remember { mutableStateOf(false) }

    val isWeightExercise = selectedExercise?.notes != "time"

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Add Exercise") },
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
                OutlinedTextField(
                    value = numSets,
                    onValueChange = { numSets = it },
                    label = { Text("Number of sets") },
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
                )
                OutlinedTextField(
                    value = reps,
                    onValueChange = { reps = it },
                    label = { Text("Reps per set") },
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
                )
                if (isWeightExercise) {
                    OutlinedTextField(
                        value = weight,
                        onValueChange = { weight = it },
                        label = { Text("Weight ($weightUnit) (optional)") },
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Decimal)
                    )
                }
                OutlinedTextField(
                    value = rpe,
                    onValueChange = { rpe = it },
                    label = { Text("Target RPE (optional)") },
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Decimal)
                )
            }
        },
        confirmButton = {
            TextButton(
                onClick = {
                    selectedExercise?.let {
                        onConfirm(
                            it.id,
                            numSets.toIntOrNull() ?: 1,
                            reps.toIntOrNull() ?: 0,
                            weight.toDoubleOrNull() ?: 0.0,
                            rpe.toDoubleOrNull() ?: 0.0
                        )
                    }
                },
                enabled = selectedExercise != null && reps.isNotBlank()
            ) { Text("Add") }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } }
    )
}
