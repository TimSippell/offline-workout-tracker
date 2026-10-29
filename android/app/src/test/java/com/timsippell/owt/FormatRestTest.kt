package com.timsippell.owt

import com.timsippell.owt.ui.screens.formatRest
import org.junit.Assert.assertEquals
import org.junit.Test

class FormatRestTest {

    @Test
    fun `formats whole minutes`() {
        assertEquals("1:30", formatRest(90_000))
    }

    @Test
    fun `rounds partial seconds up`() {
        assertEquals("1:30", formatRest(89_001))
        assertEquals("0:01", formatRest(1))
    }

    @Test
    fun `clamps negative and zero to 0_00`() {
        assertEquals("0:00", formatRest(0))
        assertEquals("0:00", formatRest(-500))
    }

    @Test
    fun `pads seconds`() {
        assertEquals("2:05", formatRest(125_000))
    }
}
