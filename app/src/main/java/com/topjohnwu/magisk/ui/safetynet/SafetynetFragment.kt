package com.topjohnwu.magisk.ui.safetynet

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSafetynetMd2Binding
import com.topjohnwu.magisk.ui.base.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class SafetynetFragment : CompatFragment<SafetynetViewModel, FragmentSafetynetMd2Binding>() {

    override val layoutRes = R.layout.fragment_safetynet_md2
    override val viewModel by viewModel<SafetynetViewModel>()

    override fun onStart() {
        super.onStart()
        activity.setTitle(R.string.safetynet)
    }

}
