package com.timsippell.owt

import com.timsippell.owt.ui.screens.parseDate
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test
import java.time.LocalDate

class ParseDateTest {

    @Test
    fun `parses full datetime format`() {
        val result = parseDate("2024-06-15 14:30:00")
        assertEquals(LocalDate.of(2024, 6, 15), result)
    }

    @Test
    fun `parses date-only format`() {
        val result = parseDate("2024-06-15")
        assertEquals(LocalDate.of(2024, 6, 15), result)
    }

    @Test
    fun `parses datetime with extra content after date`() {
        val result = parseDate("2024-01-01 00:00:00.000")
        assertEquals(LocalDate.of(2024, 1, 1), result)
    }

    @Test
    fun `returns null for empty string`() {
        assertNull(parseDate(""))
    }

    @Test
    fun `returns null for garbage input`() {
        assertNull(parseDate("not-a-date"))
    }

    @Test
    fun `returns null for partial date`() {
        assertNull(parseDate("2024-06"))
    }

    @Test
    fun `handles midnight correctly`() {
        val result = parseDate("2024-12-31 00:00:00")
        assertEquals(LocalDate.of(2024, 12, 31), result)
    }

    @Test
    fun `handles end of day correctly`() {
        val result = parseDate("2024-12-31 23:59:59")
        assertEquals(LocalDate.of(2024, 12, 31), result)
    }
}
