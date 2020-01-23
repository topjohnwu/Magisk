package com.topjohnwu.magisk.ui.settings

import android.os.Bundle
import android.view.View
import androidx.core.graphics.Insets
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSettingsMd2Binding
import com.topjohnwu.magisk.ui.base.CompatFragment
import com.topjohnwu.magisk.utils.PinchZoomTouchListener
import org.koin.androidx.viewmodel.ext.android.viewModel

class SettingsFragment : CompatFragment<SettingsViewModel, FragmentSettingsMd2Binding>() {

    override val layoutRes = R.layout.fragment_settings_md2
    override val viewModel by viewModel<SettingsViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onStart() {
        super.onStart()

        activity.title = resources.getString(R.string.settings)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        PinchZoomTouchListener.attachTo(binding.settingsList)
    }

    override fun onDestroyView() {
        PinchZoomTouchListener.clear(binding.settingsList)
        super.onDestroyView()
    }

    override fun onResume() {
        super.onResume()
        viewModel.items.forEach { it.refresh() }
    }

}
