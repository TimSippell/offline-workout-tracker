package com.timsippell.owt.ui.screens

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.timsippell.owt.bridge.OwtBridge

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SetupScreen(onFinish: () -> Unit) {
    val context = LocalContext.current
    val weightUnit = remember { AppSettings.getWeightUnit(context) }
    val exercises = remember { OwtBridge.listExercises().filter { it.notes != "time" } }
    val values = remember {
        mutableStateMapOf<Long, String>().apply {
            exercises.forEach { ex ->
                val stored = AppSettings.getOneRepMax(context, ex.id)
                val display = if (stored > 0) AppSettings.toDisplayWeight(stored, context) else 0.0
                this[ex.id] = if (stored > 0) "%.1f".format(display) else ""
            }
        }
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Set Up One Rep Max") },
                navigationIcon = {
                    IconButton(onClick = onFinish) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                    }
                }
            )
        },
        bottomBar = {
            Surface(tonalElevation = 3.dp) {
                Button(
                    onClick = {
                        values.forEach { (exerciseId, value) ->
                            val weight = value.toDoubleOrNull() ?: 0.0
                            AppSettings.setOneRepMax(context, exerciseId, AppSettings.toStorageWeight(weight, context))
                        }
                        AppSettings.setSetupComplete(context, true)
                        onFinish()
                    },
                    modifier = Modifier.fillMaxWidth().padding(16.dp)
                ) {
                    Text("Save")
                }
            }
        }
    ) { padding ->
        if (exercises.isEmpty()) {
            Box(modifier = Modifier.fillMaxSize().padding(padding).padding(32.dp)) {
                Text(
                    "No exercises yet. Add exercises first, then come back to set your one rep max.",
                    style = MaterialTheme.typography.bodyLarge
                )
            }
        } else {
            LazyColumn(
                modifier = Modifier.fillMaxSize().padding(padding),
                contentPadding = PaddingValues(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                item {
                    Text(
                        "Enter your known or estimated one rep max for each exercise. Leave blank to skip.",
                        style = MaterialTheme.typography.bodyMedium,
                        modifier = Modifier.padding(bottom = 8.dp)
                    )
                }
                items(exercises, key = { it.id }) { exercise ->
                    OutlinedTextField(
                        value = values[exercise.id] ?: "",
                        onValueChange = { values[exercise.id] = it },
                        label = { Text(exercise.name) },
                        suffix = { Text(weightUnit) },
                        singleLine = true,
                        modifier = Modifier.fillMaxWidth(),
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Decimal)
                    )
                }
            }
        }
    }
}
