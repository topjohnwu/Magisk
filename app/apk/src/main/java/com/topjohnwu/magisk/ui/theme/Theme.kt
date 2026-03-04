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
        themeRes = R.style.ThemeFoundationMD2_Piplup
    ),
    Custom(
        baseThemeName = "Custom",
        themeRes = R.style.ThemeFoundationMD2_Piplup
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
