package com.timsippell.owt

import com.timsippell.owt.bridge.OwtBridge
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

class OwtBridgeDataTest {

    @Test
    fun `exercise data class holds values`() {
        val ex = OwtBridge.Exercise(1, "Bench Press", "compound", "chest", "barbell")
        assertEquals(1L, ex.id)
        assertEquals("Bench Press", ex.name)
        assertEquals("compound", ex.category)
        assertEquals("chest", ex.muscleGroup)
        assertEquals("barbell", ex.notes)
    }

    @Test
    fun `exercise equality by value`() {
        val a = OwtBridge.Exercise(1, "Squat", "compound", "legs", "")
        val b = OwtBridge.Exercise(1, "Squat", "compound", "legs", "")
        assertEquals(a, b)
    }

    @Test
    fun `exercise inequality on different id`() {
        val a = OwtBridge.Exercise(1, "Squat", "compound", "legs", "")
        val b = OwtBridge.Exercise(2, "Squat", "compound", "legs", "")
        assertNotEquals(a, b)
    }

    @Test
    fun `workout set data class holds values`() {
        val set = OwtBridge.WorkoutSet(10, 1, 5, 1, 8, 100.0, 8.0)
        assertEquals(10L, set.id)
        assertEquals(1L, set.workoutId)
        assertEquals(5L, set.exerciseId)
        assertEquals(1, set.order)
        assertEquals(8, set.reps)
        assertEquals(100.0, set.weight, 0.0)
        assertEquals(8.0, set.rpe, 0.0)
    }

    @Test
    fun `exercise stats data class holds values`() {
        val stats = OwtBridge.ExerciseStats(5, 120.0, 100.0, 5000.0, 10)
        assertEquals(5L, stats.exerciseId)
        assertEquals(120.0, stats.estimated1rm, 0.0)
        assertEquals(100.0, stats.bestWeight, 0.0)
        assertEquals(5000.0, stats.totalVolume, 0.0)
        assertEquals(10, stats.sessionCount)
    }

    @Test
    fun `template set data class holds values`() {
        val ts = OwtBridge.TemplateSet(1, 2, 3, 1, 10, 50.0, 7.0)
        assertEquals(1L, ts.id)
        assertEquals(2L, ts.templateId)
        assertEquals(3L, ts.exerciseId)
        assertEquals(1, ts.order)
        assertEquals(10, ts.reps)
        assertEquals(50.0, ts.weight, 0.0)
        assertEquals(7.0, ts.rpe, 0.0)
    }

    @Test
    fun `progression point data class holds values`() {
        val pp = OwtBridge.ProgressionPoint("2024-06-15", 120.0, 100.0, 3000.0)
        assertEquals("2024-06-15", pp.date)
        assertEquals(120.0, pp.estimated1rm, 0.0)
        assertEquals(100.0, pp.bestSetWeight, 0.0)
        assertEquals(3000.0, pp.sessionVolume, 0.0)
    }

    @Test
    fun `workout template data class holds values`() {
        val wt = OwtBridge.WorkoutTemplate(1, "Push Day", "chest and triceps", 6)
        assertEquals(1L, wt.id)
        assertEquals("Push Day", wt.name)
        assertEquals("chest and triceps", wt.notes)
        assertEquals(6, wt.setCount)
    }

    @Test
    fun `workout data class copy works`() {
        val w = OwtBridge.Workout(1, "Morning", "2024-06-15 08:00:00", "", "notes", 5)
        val w2 = w.copy(name = "Evening")
        assertEquals("Evening", w2.name)
        assertEquals(w.id, w2.id)
    }
}
