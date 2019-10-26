package com.topjohnwu.magisk.redesign.home

import android.graphics.Insets
import android.os.Bundle
import android.view.View
import androidx.recyclerview.widget.LinearSnapHelper
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHomeMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class HomeFragment : CompatFragment<HomeViewModel, FragmentHomeMd2Binding>() {

    override val layoutRes = R.layout.fragment_home_md2
    override val viewModel by viewModel<HomeViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onStart() {
        super.onStart()
        activity.title = resources.getString(R.string.section_home)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        LinearSnapHelper().attachToRecyclerView(binding.homeSupportList)
    }
}