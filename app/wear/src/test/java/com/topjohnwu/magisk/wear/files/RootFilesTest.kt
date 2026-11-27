package com.topjohnwu.magisk.wear.files

import com.topjohnwu.magisk.terminal.shellQuoted
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Assume.assumeTrue
import org.junit.Test
import java.io.File

class RootFilesTest {

    @Test
    fun parsesZips() {
        val files = RootFiles.parseListing(
            listOf("a.zip", "b.img", "Zażółć gęślą.zip", "x.ZIP", ""),
            setOf("zip")
        )
        assertEquals(listOf("a.zip", "x.ZIP", "Zażółć gęślą.zip"), files)
    }

    @Test
    fun parsesMultiPartExtensions() {
        val files = RootFiles.parseListing(
            listOf("AP_R870.tar.md5", "boot.img", "notes.md5", "odin.TAR", "tar.md5"),
            setOf("img", "tar", "tar.md5")
        )
        assertEquals(listOf("AP_R870.tar.md5", "boot.img", "odin.TAR"), files)
    }

    @Test
    fun emptyDir() {
        assertTrue(RootFiles.parseListing(emptyList(), setOf("zip")).isEmpty())
    }

    @Test
    fun quotesNames() {
        // Runs the quoted name through a real shell, which only exists on Unix hosts
        assumeTrue(File("/bin/sh").canExecute())
        val process = ProcessBuilder("sh", "-c", "printf %s ${"it's a.zip".shellQuoted()}")
            .redirectErrorStream(true)
            .start()
        val output = process.inputStream.bufferedReader().use { it.readText() }
        assertEquals(0, process.waitFor())
        assertEquals("it's a.zip", output)
    }
}
