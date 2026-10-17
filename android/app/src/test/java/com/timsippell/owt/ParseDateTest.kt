package com.timsippell.owt

import com.timsippell.owt.ui.screens.parseDate
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test
import java.time.LocalDate
import java.time.ZoneId
import java.time.ZoneOffset

class ParseDateTest {

    private val utc = ZoneOffset.UTC

    @Test
    fun `parses full datetime format`() {
        val result = parseDate("2024-06-15 14:30:00", utc)
        assertEquals(LocalDate.of(2024, 6, 15), result)
    }

    @Test
    fun `parses date-only format`() {
        val result = parseDate("2024-06-15", utc)
        assertEquals(LocalDate.of(2024, 6, 15), result)
    }

    @Test
    fun `parses datetime with extra content after date`() {
        val result = parseDate("2024-01-01 00:00:00.000", utc)
        assertEquals(LocalDate.of(2024, 1, 1), result)
    }

    @Test
    fun `returns null for empty string`() {
        assertNull(parseDate("", utc))
    }

    @Test
    fun `returns null for garbage input`() {
        assertNull(parseDate("not-a-date", utc))
    }

    @Test
    fun `returns null for partial date`() {
        assertNull(parseDate("2024-06", utc))
    }

    @Test
    fun `handles midnight correctly`() {
        val result = parseDate("2024-12-31 00:00:00", utc)
        assertEquals(LocalDate.of(2024, 12, 31), result)
    }

    @Test
    fun `handles end of day correctly`() {
        val result = parseDate("2024-12-31 23:59:59", utc)
        assertEquals(LocalDate.of(2024, 12, 31), result)
    }

    @Test
    fun `converts UTC timestamp to earlier local day`() {
        val result = parseDate("2024-06-15 02:30:00", ZoneId.of("America/Los_Angeles"))
        assertEquals(LocalDate.of(2024, 6, 14), result)
    }

    @Test
    fun `converts UTC timestamp to later local day`() {
        val result = parseDate("2024-06-15 22:30:00", ZoneId.of("Asia/Tokyo"))
        assertEquals(LocalDate.of(2024, 6, 16), result)
    }

    @Test
    fun `date-only value ignores zone`() {
        val result = parseDate("2024-06-15", ZoneId.of("America/Los_Angeles"))
        assertEquals(LocalDate.of(2024, 6, 15), result)
    }
}
