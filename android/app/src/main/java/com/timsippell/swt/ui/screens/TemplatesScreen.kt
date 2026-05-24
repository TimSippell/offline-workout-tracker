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
