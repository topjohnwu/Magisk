package com.topjohnwu.magisk.ui.install

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseFragment
import com.topjohnwu.magisk.databinding.FragmentInstallMd2Binding
import com.topjohnwu.magisk.di.viewModel

class InstallFragment : BaseFragment<FragmentInstallMd2Binding>() {

    override val layoutRes = R.layout.fragment_install_md2
    override val viewModel by viewModel<InstallViewModel>()

    override fun onStart() {
        super.onStart()
        requireActivity().setTitle(R.string.install)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        viewModel._method = savedInstanceState?.getInt(KEY_CURRENT_METHOD, -1) ?: -1
        return super.onCreateView(inflater, container, savedInstanceState)
    }

    override fun onSaveInstanceState(outState: Bundle) {
        outState.putInt(KEY_CURRENT_METHOD, viewModel.method)
    }

    companion object {
        private const val KEY_CURRENT_METHOD = "current_method"
    }
}
