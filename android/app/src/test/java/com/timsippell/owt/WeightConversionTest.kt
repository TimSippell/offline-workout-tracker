package com.timsippell.owt

import com.timsippell.owt.ui.screens.AppSettings
import org.junit.Assert.assertEquals
import org.junit.Test

class WeightConversionTest {

    @Test
    fun `kg to display in kg mode is identity`() {
        assertEquals(100.0, AppSettings.toDisplayWeight(100.0, "kg"), 0.001)
    }

    @Test
    fun `kg to display in lbs mode converts correctly`() {
        assertEquals(220.462, AppSettings.toDisplayWeight(100.0, "lbs"), 0.001)
    }

    @Test
    fun `lbs input to storage converts to kg`() {
        assertEquals(100.0, AppSettings.toStorageWeight(220.462, "lbs"), 0.001)
    }

    @Test
    fun `kg input to storage in kg mode is identity`() {
        assertEquals(100.0, AppSettings.toStorageWeight(100.0, "kg"), 0.001)
    }

    @Test
    fun `roundtrip kg to lbs and back preserves value`() {
        val original = 85.5
        val displayed = AppSettings.toDisplayWeight(original, "lbs")
        val stored = AppSettings.toStorageWeight(displayed, "lbs")
        assertEquals(original, stored, 0.0001)
    }

    @Test
    fun `zero weight stays zero in both directions`() {
        assertEquals(0.0, AppSettings.toDisplayWeight(0.0, "lbs"), 0.0)
        assertEquals(0.0, AppSettings.toStorageWeight(0.0, "lbs"), 0.0)
    }

    @Test
    fun `known plate math - 60kg is about 132 lbs`() {
        val lbs = AppSettings.toDisplayWeight(60.0, "lbs")
        assertEquals(132.277, lbs, 0.001)
    }

    @Test
    fun `known plate math - 225 lbs is about 102 kg`() {
        val kg = AppSettings.toStorageWeight(225.0, "lbs")
        assertEquals(102.058, kg, 0.001)
    }
}
