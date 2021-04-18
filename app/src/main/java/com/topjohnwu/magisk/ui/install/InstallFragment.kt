package com.topjohnwu.magisk.ui.install

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIFragment
import com.topjohnwu.magisk.databinding.FragmentInstallMd2Binding
import com.topjohnwu.magisk.di.viewModel
import com.topjohnwu.magisk.ktx.coroutineScope

class InstallFragment : BaseUIFragment<InstallViewModel, FragmentInstallMd2Binding>() {

    override val layoutRes = R.layout.fragment_install_md2
    override val viewModel by viewModel<InstallViewModel>()

    override fun onStart() {
        super.onStart()
        requireActivity().setTitle(R.string.install)

        // Allow markwon to run in viewmodel scope
        binding.releaseNotes.coroutineScope = viewModel.viewModelScope
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
