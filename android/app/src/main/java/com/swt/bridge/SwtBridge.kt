package com.swt.bridge

import android.content.Context

object SwtBridge {
    init {
        System.loadLibrary("swt-jni")
    }

    data class Exercise(
        val id: Long,
        val name: String,
        val category: String,
        val muscleGroup: String,
        val notes: String
    )

    data class Workout(
        val id: Long,
        val name: String,
        val startedAt: String,
        val finishedAt: String,
        val notes: String,
        val setCount: Int
    )

    data class WorkoutSet(
        val id: Long,
        val workoutId: Long,
        val exerciseId: Long,
        val order: Int,
        val reps: Int,
        val weight: Double,
        val rpe: Double
    )

    data class ExerciseStats(
        val exerciseId: Long,
        val estimated1rm: Double,
        val bestWeight: Double,
        val totalVolume: Double,
        val sessionCount: Int
    )

    data class ProgressionPoint(
        val date: String,
        val estimated1rm: Double,
        val bestSetWeight: Double,
        val sessionVolume: Double
    )

    fun init(context: Context) {
        val dbPath = context.getDatabasePath("swt.db").absolutePath
        context.getDatabasePath("swt.db").parentFile?.mkdirs()
        nativeInit(dbPath)
    }

    fun close() = nativeClose()

    fun addExercise(name: String, category: String, muscleGroup: String, notes: String): Long =
        nativeAddExercise(name, category, muscleGroup, notes)

    fun listExercises(filter: String = ""): List<Exercise> =
        nativeListExercises(filter)?.toList() ?: emptyList()

    fun deleteExercise(id: Long) = nativeDeleteExercise(id)

    fun startWorkout(name: String = ""): Long = nativeStartWorkout(name)
    fun finishWorkout(id: Long) = nativeFinishWorkout(id)
    fun listWorkouts(limit: Int = 20, offset: Int = 0): List<Workout> =
        nativeListWorkouts(limit, offset)?.toList() ?: emptyList()
    fun deleteWorkout(id: Long) = nativeDeleteWorkout(id)

    fun addSet(workoutId: Long, exerciseId: Long, order: Int, reps: Int, weight: Double, rpe: Double): Long =
        nativeAddSet(workoutId, exerciseId, order, reps, weight, rpe)
    fun getSetsForWorkout(workoutId: Long): List<WorkoutSet> =
        nativeGetSetsForWorkout(workoutId)?.toList() ?: emptyList()
    fun deleteSet(id: Long) = nativeDeleteSet(id)

    fun getStats(exerciseId: Long): ExerciseStats? = nativeGetStats(exerciseId)
    fun getProgression(exerciseId: Long, sessions: Int = 20): List<ProgressionPoint> =
        nativeGetProgression(exerciseId, sessions)?.toList() ?: emptyList()

    private external fun nativeInit(dbPath: String)
    private external fun nativeClose()
    private external fun nativeAddExercise(name: String, category: String, muscleGroup: String, notes: String): Long
    private external fun nativeListExercises(filter: String): Array<Exercise>?
    private external fun nativeDeleteExercise(id: Long)
    private external fun nativeStartWorkout(name: String): Long
    private external fun nativeFinishWorkout(id: Long)
    private external fun nativeListWorkouts(limit: Int, offset: Int): Array<Workout>?
    private external fun nativeDeleteWorkout(id: Long)
    private external fun nativeAddSet(workoutId: Long, exerciseId: Long, order: Int, reps: Int, weight: Double, rpe: Double): Long
    private external fun nativeGetSetsForWorkout(workoutId: Long): Array<WorkoutSet>?
    private external fun nativeDeleteSet(id: Long)
    private external fun nativeGetStats(exerciseId: Long): ExerciseStats?
    private external fun nativeGetProgression(exerciseId: Long, sessions: Int): Array<ProgressionPoint>?
}
