package com.topjohnwu.magisk.ui.flash

import org.junit.Assert.assertEquals
import org.junit.Test

class DisplayNameTest {

    @Test
    fun dropsControlChars() {
        assertEquals("ab.zip", "a\u0000b\n.zip\u007f".sanitizeDisplayName())
    }

    @Test
    fun dropsBidiOverride() {
        assertEquals("evilpiz.exe", "evil‮piz.exe".sanitizeDisplayName())
    }

    @Test
    fun dropsSupplementaryFormatChars() {
        val tag = String(Character.toChars(0xE0041))
        assertEquals("module.zip", "module${tag}.zip".sanitizeDisplayName())
    }

    @Test
    fun keepsOtherSupplementaryChars() {
        val emoji = String(Character.toChars(0x1F600))
        assertEquals("$emoji.zip", "$emoji.zip".sanitizeDisplayName())
    }

    @Test
    fun capsLengthInCodePoints() {
        val emoji = String(Character.toChars(0x1F600))
        assertEquals(emoji.repeat(100), emoji.repeat(150).sanitizeDisplayName())
        assertEquals("a".repeat(100), "a".repeat(150).sanitizeDisplayName())
    }

    @Test
    fun trims() {
        assertEquals("module.zip", "  module.zip \t".sanitizeDisplayName())
    }
}
