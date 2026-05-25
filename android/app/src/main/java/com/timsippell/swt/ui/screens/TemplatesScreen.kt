package com.timsippell.swt.ui.screens

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import com.timsippell.swt.bridge.SwtBridge

internal data class DefaultTemplateSet(val exerciseName: String, val sets: Int, val reps: Int)
internal data class DefaultTemplate(val name: String, val exercises: List<DefaultTemplateSet>)

internal val defaultTemplates = listOf(
    DefaultTemplate("Push Day", listOf(
        DefaultTemplateSet("Bench Press", 4, 8),
        DefaultTemplateSet("Overhead Press", 3, 10),
        DefaultTemplateSet("Incline Bench Press", 3, 10),
        DefaultTemplateSet("Lateral Raise", 3, 15),
        DefaultTemplateSet("Tricep Pushdown", 3, 12),
    )),
    DefaultTemplate("Pull Day", listOf(
        DefaultTemplateSet("Deadlift", 3, 5),
        DefaultTemplateSet("Barbell Row", 4, 8),
        DefaultTemplateSet("Pull Up", 3, 8),
        DefaultTemplateSet("Face Pull", 3, 15),
        DefaultTemplateSet("Bicep Curl", 3, 12),
    )),
    DefaultTemplate("Leg Day", listOf(
        DefaultTemplateSet("Squat", 4, 6),
        DefaultTemplateSet("Romanian Deadlift", 3, 10),
        DefaultTemplateSet("Leg Press", 3, 12),
        DefaultTemplateSet("Leg Curl", 3, 12),
        DefaultTemplateSet("Calf Raise", 4, 15),
    )),
    DefaultTemplate("Upper Body", listOf(
        DefaultTemplateSet("Bench Press", 4, 8),
        DefaultTemplateSet("Barbell Row", 4, 8),
        DefaultTemplateSet("Overhead Press", 3, 10),
        DefaultTemplateSet("Lat Pulldown", 3, 10),
        DefaultTemplateSet("Bicep Curl", 2, 12),
        DefaultTemplateSet("Tricep Pushdown", 2, 12),
    )),
    DefaultTemplate("Lower Body", listOf(
        DefaultTemplateSet("Squat", 4, 6),
        DefaultTemplateSet("Romanian Deadlift", 3, 10),
        DefaultTemplateSet("Leg Press", 3, 12),
        DefaultTemplateSet("Leg Extension", 3, 12),
        DefaultTemplateSet("Leg Curl", 3, 12),
        DefaultTemplateSet("Calf Raise", 4, 15),
    )),
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TemplatesScreen(navController: NavController) {
    var templates by remember { mutableStateOf(SwtBridge.listTemplates()) }
    var showCreateDialog by remember { mutableStateOf(false) }
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Workout Templates") },
                navigationIcon = {
                    IconButton(onClick = { navController.popBackStack() }) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                    }
                }
            )
        },
        floatingActionButton = {
            FloatingActionButton(onClick = { showCreateDialog = true }) {
                Icon(Icons.Default.Add, contentDescription = "New template")
            }
        }
    ) { padding ->
        LazyColumn(
            modifier = Modifier.fillMaxSize().padding(padding),
            contentPadding = PaddingValues(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            items(templates, key = { it.id }) { template ->
                TemplateCard(
                    template = template,
                    onClick = {
                        navController.navigate("template_builder/${template.id}")
                    },
                    onDelete = {
                        SwtBridge.deleteTemplate(template.id)
                        templates = SwtBridge.listTemplates()
                    }
                )
            }
            if (templates.isEmpty()) {
                item {
                    Text(
                        "No templates yet. Tap + to create one.",
                        modifier = Modifier.padding(32.dp),
                        style = MaterialTheme.typography.bodyLarge
                    )
                }
            }
        }
    }

    if (showCreateDialog) {
        CreateTemplateDialog(
            onDismiss = { showCreateDialog = false },
            onConfirm = { name ->
                val id = SwtBridge.createTemplate(name)
                showCreateDialog = false
                navController.navigate("template_builder/$id")
            }
        )
    }

}

internal fun seedDefaultTemplates() {
    val exercises = SwtBridge.listExercises()
    val exerciseMap = exercises.associateBy { it.name }

    for (tmpl in defaultTemplates) {
        val templateId = SwtBridge.createTemplate(tmpl.name)
        var order = 1
        for (ex in tmpl.exercises) {
            val exerciseId = exerciseMap[ex.exerciseName]?.id
                ?: SwtBridge.addExercise(ex.exerciseName, "", "", "weight")
            for (s in 1..ex.sets) {
                SwtBridge.addTemplateSet(templateId, exerciseId, order, ex.reps, 0.0, 0.0)
                order++
            }
        }
    }
}

@Composable
private fun TemplateCard(
    template: SwtBridge.WorkoutTemplate,
    onClick: () -> Unit,
    onDelete: () -> Unit
) {
    Card(modifier = Modifier.fillMaxWidth().clickable(onClick = onClick)) {
        Row(
            modifier = Modifier.padding(16.dp).fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(template.name, style = MaterialTheme.typography.titleMedium)
                Text(
                    "${template.setCount} sets",
                    style = MaterialTheme.typography.bodySmall
                )
            }
            IconButton(onClick = onDelete) {
                Icon(Icons.Default.Delete, contentDescription = "Delete")
            }
        }
    }
}

@Composable
private fun CreateTemplateDialog(onDismiss: () -> Unit, onConfirm: (String) -> Unit) {
    var name by remember { mutableStateOf("") }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("New Template") },
        text = {
            OutlinedTextField(
                value = name,
                onValueChange = { name = it },
                label = { Text("Template name") },
                singleLine = true
            )
        },
        confirmButton = {
            TextButton(onClick = { onConfirm(name) }, enabled = name.isNotBlank()) {
                Text("Create")
            }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text("Cancel") } }
    )
}
