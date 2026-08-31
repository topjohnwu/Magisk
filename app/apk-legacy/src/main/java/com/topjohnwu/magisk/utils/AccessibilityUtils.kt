package com.topjohnwu.magisk.utils

import android.content.ContentResolver
import android.provider.Settings

class AccessibilityUtils {
    companion object {
        fun isAnimationEnabled(cr: ContentResolver): Boolean {
            return !(Settings.Global.getFloat(cr, Settings.Global.ANIMATOR_DURATION_SCALE, 1.0f) == 0.0f
                && Settings.Global.getFloat(cr, Settings.Global.TRANSITION_ANIMATION_SCALE, 1.0f) == 0.0f
                && Settings.Global.getFloat(cr, Settings.Global.WINDOW_ANIMATION_SCALE, 1.0f) == 0.0f)
        }
    }
}
