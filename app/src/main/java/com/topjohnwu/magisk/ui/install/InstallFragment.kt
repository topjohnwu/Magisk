package com.topjohnwu.magisk.ui.install

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentInstallMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class InstallFragment : CompatFragment<InstallViewModel, FragmentInstallMd2Binding>() {

    override val layoutRes = R.layout.fragment_install_md2
    override val viewModel by viewModel<InstallViewModel>()

}