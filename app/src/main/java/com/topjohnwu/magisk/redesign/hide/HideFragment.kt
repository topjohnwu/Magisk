package com.topjohnwu.magisk.redesign.hide

import android.content.Context
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHideMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class HideFragment : CompatFragment<HideViewModel, FragmentHideMd2Binding>() {

    override val layoutRes = R.layout.fragment_hide_md2
    override val viewModel by viewModel<HideViewModel>()

    override fun onAttach(context: Context) {
        super.onAttach(context)

        activity.setTitle(R.string.magiskhide)
    }

}