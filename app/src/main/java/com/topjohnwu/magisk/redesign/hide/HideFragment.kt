package com.topjohnwu.magisk.redesign.hide

import android.content.Context
import android.graphics.Insets
import android.os.Bundle
import android.view.View
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHideMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class HideFragment : CompatFragment<HideViewModel, FragmentHideMd2Binding>() {

    override val layoutRes = R.layout.fragment_hide_md2
    override val viewModel by viewModel<HideViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onAttach(context: Context) {
        super.onAttach(context)
        activity.setTitle(R.string.magiskhide)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.hideActionScrollUp.setOnClickListener {
            binding.hideScrollContainer.fullScroll(View.FOCUS_UP)
        }
    }

}