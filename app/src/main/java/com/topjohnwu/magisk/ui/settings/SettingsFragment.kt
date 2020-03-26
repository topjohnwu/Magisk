package com.topjohnwu.magisk.ui.settings

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSettingsMd2Binding
import com.topjohnwu.magisk.ui.base.BaseUIFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class SettingsFragment : BaseUIFragment<SettingsViewModel, FragmentSettingsMd2Binding>() {

    override val layoutRes = R.layout.fragment_settings_md2
    override val viewModel by viewModel<SettingsViewModel>()

    override fun onStart() {
        super.onStart()

        activity.title = resources.getString(R.string.settings)
    }

    override fun onResume() {
        super.onResume()
        viewModel.items.forEach { it.refresh() }
    }

}
