package com.topjohnwu.magisk.redesign.log

import android.graphics.Insets
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentLogMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class LogFragment : CompatFragment<LogViewModel, FragmentLogMd2Binding>() {

    override val layoutRes = R.layout.fragment_log_md2
    override val viewModel by viewModel<LogViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onStart() {
        super.onStart()

        activity.title = resources.getString(R.string.section_log)
    }

    override fun onPreBind(binding: FragmentLogMd2Binding) = Unit

}