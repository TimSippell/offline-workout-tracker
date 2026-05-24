package com.swt.ui

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.FitnessCenter
import androidx.compose.material.icons.filled.History
import androidx.compose.material.icons.filled.List
import androidx.compose.material.icons.filled.TrendingUp
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.swt.ui.screens.*

enum class Screen(val route: String, val label: String, val icon: ImageVector) {
    Exercises("exercises", "Exercises", Icons.Default.List),
    Workout("workout", "Workout", Icons.Default.FitnessCenter),
    History("history", "History", Icons.Default.History),
    Progress("progress", "Progress", Icons.Default.TrendingUp)
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SwtApp() {
    val navController = rememberNavController()
    val currentEntry by navController.currentBackStackEntryAsState()
    val currentRoute = currentEntry?.destination?.route

    MaterialTheme(colorScheme = dynamicColorScheme()) {
        Scaffold(
            bottomBar = {
                NavigationBar {
                    Screen.entries.forEach { screen ->
                        NavigationBarItem(
                            selected = currentRoute == screen.route,
                            onClick = {
                                navController.navigate(screen.route) {
                                    popUpTo(navController.graph.startDestinationId) { saveState = true }
                                    launchSingleTop = true
                                    restoreState = true
                                }
                            },
                            icon = { Icon(screen.icon, contentDescription = screen.label) },
                            label = { Text(screen.label) }
                        )
                    }
                }
            }
        ) { padding ->
            NavHost(
                navController = navController,
                startDestination = Screen.Workout.route,
                modifier = Modifier.padding(padding)
            ) {
                composable(Screen.Exercises.route) { ExercisesScreen() }
                composable(Screen.Workout.route) { WorkoutScreen() }
                composable(Screen.History.route) { HistoryScreen() }
                composable(Screen.Progress.route) { ProgressScreen() }
            }
        }
    }
}

@Composable
private fun dynamicColorScheme(): ColorScheme {
    return MaterialTheme.colorScheme
}
