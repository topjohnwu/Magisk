package com.topjohnwu.magisk.ui.animation

import androidx.compose.animation.core.Spring

object MotionTokens {
    const val DurationQuick = 120
    const val DurationMedium = 180
    const val DurationStandard = 220
    const val DurationExpand = 260
    const val DurationCollapse = 210
    const val DurationEmphasized = 280

    const val DurationTiny = 100
    const val DelayXs = 40
    const val DelaySm = 50
    const val Stagger1 = 60
    const val Stagger2 = 110
    const val Stagger3 = 150

    val DampingNoBounce = Spring.DampingRatioNoBouncy
    val DampingLowBouncy = Spring.DampingRatioLowBouncy
    val StiffnessLow = Spring.StiffnessLow
    val StiffnessMediumLow = Spring.StiffnessMediumLow
}
