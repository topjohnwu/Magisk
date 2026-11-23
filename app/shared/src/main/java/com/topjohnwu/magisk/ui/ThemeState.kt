package com.topjohnwu.magisk.ui

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.setValue
import com.topjohnwu.magisk.core.Config

object ThemeState {
    var colorMode by mutableIntStateOf(Config.colorMode)
}
