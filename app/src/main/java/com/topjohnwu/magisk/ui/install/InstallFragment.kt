package com.topjohnwu.magisk.ui.install

import android.content.Intent
import android.os.Bundle
import android.view.View
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIFragment
import com.topjohnwu.magisk.core.download.BaseDownloader
import com.topjohnwu.magisk.databinding.FragmentInstallMd2Binding
import com.topjohnwu.magisk.events.RequestFileEvent
import com.topjohnwu.magisk.ktx.coroutineScope
import org.koin.androidx.viewmodel.ext.android.viewModel

class InstallFragment : BaseUIFragment<InstallViewModel, FragmentInstallMd2Binding>() {

    override val layoutRes = R.layout.fragment_install_md2
    override val viewModel by viewModel<InstallViewModel>()

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        savedInstanceState?.getInt(KEY_CURRENT_METHOD, -1)?.let { viewModel.method = it }
        binding.methodContainer.setOnCheckedChangeListener { group, checkedId ->
            // If the UI is restored, this also triggers, but we only want user actions
            if (group.findViewById<View>(checkedId)?.isPressed == true) {
                viewModel.onMethodChanged(checkedId)
            }
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        viewModel.data = RequestFileEvent.resolve(requestCode, resultCode, data)
    }

    override fun onStart() {
        super.onStart()
        requireActivity().setTitle(R.string.install)

        // Allow markwon to run in viewmodel scope
        binding.releaseNotes.coroutineScope = viewModel.viewModelScope
        BaseDownloader.observeProgress(this, viewModel::onProgressUpdate)
    }

    override fun onSaveInstanceState(outState: Bundle) {
        outState.putInt(KEY_CURRENT_METHOD, viewModel.method)
        super.onSaveInstanceState(outState)
    }

    companion object {
        private const val KEY_CURRENT_METHOD = "current_method"
    }
}
