package com.timsippell.swt.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.timsippell.swt.bridge.SwtBridge

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ProgressScreen() {
    var exercises by remember { mutableStateOf(SwtBridge.listExercises()) }
    var selectedExercise by remember { mutableStateOf<SwtBridge.Exercise?>(null) }
    var stats by remember { mutableStateOf<SwtBridge.ExerciseStats?>(null) }
    var progression by remember { mutableStateOf<List<SwtBridge.ProgressionPoint>>(emptyList()) }

    LaunchedEffect(selectedExercise) {
        selectedExercise?.let {
            stats = SwtBridge.getStats(it.id)
            progression = SwtBridge.getProgression(it.id)
        }
    }

    Scaffold(
        topBar = { TopAppBar(title = { Text("Progress") }) }
    ) { padding ->
        Column(
            modifier = Modifier.fillMaxSize().padding(padding).padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            var expanded by remember { mutableStateOf(false) }
            ExposedDropdownMenuBox(expanded = expanded, onExpandedChange = { expanded = it }) {
                OutlinedTextField(
                    value = selectedExercise?.name ?: "Select exercise",
                    onValueChange = {},
                    readOnly = true,
                    modifier = Modifier.fillMaxWidth().menuAnchor(),
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

            stats?.let { s ->
                Card(modifier = Modifier.fillMaxWidth()) {
                    Column(modifier = Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        Text("Stats", style = MaterialTheme.typography.titleMedium)
                        Text("Est. 1RM: %.1f kg".format(s.estimated1rm))
                        Text("Best Weight: %.1f kg".format(s.bestWeight))
                        Text("Total Volume: %.0f kg".format(s.totalVolume))
                        Text("Sessions: ${s.sessionCount}")
                    }
                }
            }

            if (progression.isNotEmpty()) {
                Text("Progression", style = MaterialTheme.typography.titleMedium)
                LazyColumn(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    items(progression) { point ->
                        Row(
                            modifier = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
                            horizontalArrangement = Arrangement.SpaceBetween
                        ) {
                            Text(point.date, style = MaterialTheme.typography.bodySmall)
                            Text("1RM: %.1f".format(point.estimated1rm), style = MaterialTheme.typography.bodySmall)
                            Text("Vol: %.0f".format(point.sessionVolume), style = MaterialTheme.typography.bodySmall)
                        }
                    }
                }
            }

            if (selectedExercise == null) {
                Text(
                    "Select an exercise to view progress.",
                    style = MaterialTheme.typography.bodyLarge,
                    modifier = Modifier.padding(16.dp)
                )
            }
        }
    }
}
