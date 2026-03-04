package com.topjohnwu.magisk.ui.theme

import android.os.Build
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config

enum class Theme(
    private val baseThemeName: String,
    val themeRes: Int
) {
    Piplup(
        baseThemeName = "Piplup",
        themeRes = R.style.ThemeFoundationMD2_Piplup
    ),
    PiplupAmoled(
        baseThemeName = "AMOLED",
        themeRes = R.style.ThemeFoundationMD2_Amoled
    ),
    Rayquaza(
        baseThemeName = "Rayquaza",
        themeRes = R.style.ThemeFoundationMD2_Rayquaza
    ),
    Zapdos(
        baseThemeName = "Zapdos",
        themeRes = R.style.ThemeFoundationMD2_Zapdos
    ),
    Charmeleon(
        baseThemeName = "Charmeleon",
        themeRes = R.style.ThemeFoundationMD2_Charmeleon
    ),
    Mew(
        baseThemeName = "Mew",
        themeRes = R.style.ThemeFoundationMD2_Mew
    ),
    Salamence(
        baseThemeName = "Salamence",
        themeRes = R.style.ThemeFoundationMD2_Salamence
    ),
    Fraxure(
        baseThemeName = "Fraxure (Legacy)",
        themeRes = R.style.ThemeFoundationMD2_Fraxure
    ),
    Default(
        baseThemeName = "Default (Dynamic)",
        themeRes = R.style.ThemeFoundationMD2_Default
    ),
    Custom(
        baseThemeName = "Custom",
        // Keep legacy views safe: Compose supplies the actual custom color scheme.
        themeRes = R.style.ThemeFoundationMD2_Default
    );

    val themeName: String
        get() = when {
            this == Default && !supportsMonet -> "Default (Automatic)"
            else -> baseThemeName
        }

    val isSelected get() = selected == this

    companion object {
        val supportsMonet: Boolean
            get() = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S

        val selected
            get() = when (val stored = values().getOrNull(Config.themeOrdinal) ?: Default) {
                // AMOLED is now a dark mode option; keep old ordinals compatible by mapping it.
                PiplupAmoled -> Piplup
                else -> stored
            }

        val shouldUseDynamicColor: Boolean
            get() = supportsMonet && selected == Default

        val displayOrder: List<Theme>
            get() = buildList {
                add(Default)
                add(Custom)
                addAll(values().filterNot { it == Default || it == Custom || it == PiplupAmoled })
            }
    }

}
