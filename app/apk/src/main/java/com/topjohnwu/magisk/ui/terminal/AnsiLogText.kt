package com.topjohnwu.magisk.ui.terminal

import androidx.compose.material3.ColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontWeight

private val ANSI_REGEX = Regex("\u001B\\[[0-9;]*m")
private val ERROR_REGEX = Regex("(?i)\\b(error|failed|failure|fatal|exception|denied|abort|invalid)\\b")

@Composable
fun ansiLogText(line: String, colors: ColorScheme): AnnotatedString {
    val parsed = parseAnsiSegments(line, colors)
    val plain = parsed.joinToString(separator = "") { it.text }
    val hasError = line.startsWith("!") || ERROR_REGEX.containsMatchIn(plain)

    return buildAnnotatedString {
        parsed.forEach { seg ->
            pushStyle(SpanStyle(color = seg.color))
            append(seg.text)
            pop()
        }
        if (hasError && plain.isNotEmpty()) {
            addStyle(
                style = SpanStyle(
                    color = colors.error,
                    fontWeight = FontWeight.SemiBold
                ),
                start = 0,
                end = plain.length
            )
        }
    }
}

private data class Segment(val text: String, val color: Color)

private fun parseAnsiSegments(line: String, colors: ColorScheme): List<Segment> {
    if (!line.contains('\u001B')) return listOf(Segment(line, colors.onSurfaceVariant))

    val segments = mutableListOf<Segment>()
    var cursor = 0
    var currentColor: Color = colors.onSurfaceVariant

    ANSI_REGEX.findAll(line).forEach { match ->
        if (match.range.first > cursor) {
            segments += Segment(line.substring(cursor, match.range.first), currentColor)
        }
        currentColor = applySgr(currentColor, match.value, colors)
        cursor = match.range.last + 1
    }

    if (cursor < line.length) {
        segments += Segment(line.substring(cursor), currentColor)
    }
    return segments
}

private fun applySgr(current: Color, sequence: String, colors: ColorScheme): Color {
    val payload = sequence.removePrefix("\u001B[").removeSuffix("m")
    if (payload.isBlank()) return colors.onSurfaceVariant
    val codes = payload.split(';').mapNotNull { it.toIntOrNull() }
    var color = current
    for (code in codes) {
        color = when (code) {
            0 -> colors.onSurfaceVariant
            30 -> Color(0xFF222222)
            31 -> colors.error
            32 -> colors.tertiary
            33 -> colors.primary
            34 -> Color(0xFF4FC3F7)
            35 -> Color(0xFFBA68C8)
            36 -> Color(0xFF4DD0E1)
            37 -> colors.onSurface
            90 -> colors.outline
            91 -> colors.error
            92 -> colors.tertiary
            93 -> colors.primary
            94 -> Color(0xFF81D4FA)
            95 -> Color(0xFFCE93D8)
            96 -> Color(0xFF80DEEA)
            97 -> colors.onSurface
            else -> color
        }
    }
    return color
}
