package com.topjohnwu.magisk.core.model

enum class ColorMode(val value: Int) {
    MONET_SYSTEM(0),
    MONET_LIGHT(1),
    MONET_DARK(2),
    SYSTEM(3),
    LIGHT(4),
    DARK(5);

    val isMonet: Boolean
        get() = value in 0..2

    fun isDark(systemDark: Boolean): Boolean = when (this) {
        MONET_LIGHT, LIGHT -> false
        MONET_DARK, DARK -> true
        MONET_SYSTEM, SYSTEM -> systemDark
    }

    fun toUiIndex(isDynamicColorSupported: Boolean): Int {
        return if (isDynamicColorSupported) {
            value.coerceIn(0, 5)
        } else {
            (value - 3).coerceIn(0, 2)
        }
    }

    companion object {
        fun fromValue(value: Int): ColorMode {
            return entries.firstOrNull { it.value == value } ?: MONET_SYSTEM
        }

        fun fromUiIndex(index: Int, isDynamicColorSupported: Boolean): ColorMode {
            val targetValue = if (isDynamicColorSupported) index else index + 3
            return fromValue(targetValue)
        }
    }
}
