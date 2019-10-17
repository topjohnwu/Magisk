package com.topjohnwu.magisk.redesign.superuser

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSuperuserMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class SuperuserFragment : CompatFragment<SuperuserViewModel, FragmentSuperuserMd2Binding>() {

    override val layoutRes = R.layout.fragment_superuser_md2
    override val viewModel by viewModel<SuperuserViewModel>()

}