package com.topjohnwu.magisk.ui.log

data class MagiskLogEntry(
    val timestamp: String = "",
    val pid: Int = 0,
    val tid: Int = 0,
    val level: Char = 'I',
    val tag: String = "",
    val message: String = "",
    val isParsed: Boolean = false,
)

object MagiskLogParser {

    // Logcat format: "MM-DD HH:MM:SS.mmm  PID  TID LEVEL TAG     : message"
    private val logcatRegex = Regex(
        """(\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d{3})\s+(\d+)\s+(\d+)\s+([VDIWEF])\s+(.+?)\s*:\s+(.*)"""
    )

    fun parse(raw: String): List<MagiskLogEntry> {
        if (raw.isBlank()) return emptyList()

        val lines = raw.lines()
        val result = mutableListOf<MagiskLogEntry>()

        for (line in lines) {
            if (line.isBlank()) continue

            val match = logcatRegex.find(line)
            if (match != null) {
                result.add(
                    MagiskLogEntry(
                        timestamp = match.groupValues[1],
                        pid = match.groupValues[2].toIntOrNull() ?: 0,
                        tid = match.groupValues[3].toIntOrNull() ?: 0,
                        level = match.groupValues[4].firstOrNull() ?: 'I',
                        tag = match.groupValues[5].trim(),
                        message = match.groupValues[6],
                        isParsed = true,
                    )
                )
            } else if (result.isNotEmpty() && result.last().isParsed) {
                // Continuation line — append to previous entry
                val prev = result.last()
                result[result.lastIndex] = prev.copy(
                    message = prev.message + "\n" + line.trimEnd()
                )
            } else {
                result.add(
                    MagiskLogEntry(message = line.trimEnd())
                )
            }
        }
        return result
    }
}
