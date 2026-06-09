package com.timsippell.owt.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.platform.LocalContext
import com.timsippell.owt.bridge.OwtBridge

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ProgressScreen() {
    val context = LocalContext.current
    val weightUnit = remember { AppSettings.getWeightUnit(context) }
    var exercises by remember { mutableStateOf(OwtBridge.listExercises()) }
    var selectedExercise by remember { mutableStateOf<OwtBridge.Exercise?>(null) }
    var stats by remember { mutableStateOf<OwtBridge.ExerciseStats?>(null) }
    var progression by remember { mutableStateOf<List<OwtBridge.ProgressionPoint>>(emptyList()) }

    LaunchedEffect(selectedExercise) {
        selectedExercise?.let {
            stats = OwtBridge.getStats(it.id)
            progression = OwtBridge.getProgression(it.id)
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
                        Text("Est. 1RM: %.1f %s".format(AppSettings.toDisplayWeight(s.estimated1rm, context), weightUnit))
                        Text("Best Weight: %.1f %s".format(AppSettings.toDisplayWeight(s.bestWeight, context), weightUnit))
                        Text("Total Volume: %.0f %s".format(AppSettings.toDisplayWeight(s.totalVolume, context), weightUnit))
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
                            Text("1RM: %.1f".format(AppSettings.toDisplayWeight(point.estimated1rm, context)), style = MaterialTheme.typography.bodySmall)
                            Text("Vol: %.0f".format(AppSettings.toDisplayWeight(point.sessionVolume, context)), style = MaterialTheme.typography.bodySmall)
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
