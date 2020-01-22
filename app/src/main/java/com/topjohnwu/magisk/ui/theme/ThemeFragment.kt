package com.topjohnwu.magisk.ui.theme

import androidx.core.graphics.Insets
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentThemeMd2Binding
import com.topjohnwu.magisk.ui.base.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class ThemeFragment : CompatFragment<ThemeViewModel, FragmentThemeMd2Binding>() {

    override val layoutRes = R.layout.fragment_theme_md2
    override val viewModel by viewModel<ThemeViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onStart() {
        super.onStart()

        activity.title = getString(R.string.section_theme)
    }

}
