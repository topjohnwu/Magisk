package com.topjohnwu.magisk.redesign.home

import android.graphics.Insets
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHomeMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class HomeFragment : CompatFragment<HomeViewModel, FragmentHomeMd2Binding>() {

    override val layoutRes = R.layout.fragment_home_md2
    override val viewModel by viewModel<HomeViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

}