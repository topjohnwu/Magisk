package com.topjohnwu.magisk.ui.terminal

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import androidx.compose.ui.unit.dp

private enum class LogSeverity { ERROR, WARN, INFO }
private val ERROR_WORDS = Regex("\\b(error|failed|failure|fatal|exception|denied|abort|invalid)\\b", RegexOption.IGNORE_CASE)
private val WARN_WORDS = Regex("\\b(warn|warning|deprecated|timeout|retry)\\b", RegexOption.IGNORE_CASE)
private val ANSI_CODES = Regex("\u001B\\[[0-9;]*m")

private fun detectSeverity(line: String): LogSeverity {
    val t = line.replace(ANSI_CODES, "")
    return when {
        t.startsWith("!") || ERROR_WORDS.containsMatchIn(t) -> LogSeverity.ERROR
        WARN_WORDS.containsMatchIn(t) -> LogSeverity.WARN
        else -> LogSeverity.INFO
    }
}

@Composable
fun StyledLogLine(
    line: String,
    colors: ColorScheme,
    modifier: Modifier = Modifier,
) {
    val normalized = line.replace("\u0000", "").trimEnd()
    val severity = detectSeverity(normalized)
    val (tag, container, content) = when (severity) {
        LogSeverity.ERROR -> Triple("ERR", colors.errorContainer, colors.onErrorContainer)
        LogSeverity.WARN -> Triple("WRN", colors.tertiaryContainer, colors.onTertiaryContainer)
        LogSeverity.INFO -> {
            when {
                normalized.startsWith("-") -> Triple("STEP", colors.primaryContainer.copy(alpha = 0.45f), colors.onPrimaryContainer)
                normalized.startsWith("*") -> Triple("INFO", colors.secondaryContainer.copy(alpha = 0.5f), colors.onSecondaryContainer)
                else -> Triple("LOG", colors.surfaceContainerHighest, colors.onSurfaceVariant)
            }
        }
    }

    Row(
        modifier = modifier,
        horizontalArrangement = Arrangement.Start
    ) {
        Surface(
            color = container,
            contentColor = content,
            shape = RoundedCornerShape(6.dp)
        ) {
            Text(
                text = tag,
                modifier = Modifier.width(30.dp),
                fontFamily = FontFamily.Monospace,
                fontWeight = FontWeight.Bold,
                style = MaterialTheme.typography.labelSmall
            )
        }
        Spacer(Modifier.width(8.dp))
        Text(
            text = ansiLogText(normalized, colors),
            fontFamily = FontFamily.Monospace,
            style = MaterialTheme.typography.bodySmall.copy(lineHeight = 18.sp),
            softWrap = true
        )
    }
}
