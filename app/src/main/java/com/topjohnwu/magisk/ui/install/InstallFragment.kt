package com.topjohnwu.magisk.ui.install

import android.content.Intent
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIFragment
import com.topjohnwu.magisk.core.download.BaseDownloadService
import com.topjohnwu.magisk.databinding.FragmentInstallMd2Binding
import com.topjohnwu.magisk.events.RequestFileEvent
import com.topjohnwu.magisk.ktx.coroutineScope
import org.koin.androidx.viewmodel.ext.android.viewModel

class InstallFragment : BaseUIFragment<InstallViewModel, FragmentInstallMd2Binding>() {

    override val layoutRes = R.layout.fragment_install_md2
    override val viewModel by viewModel<InstallViewModel>()

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        viewModel.data = RequestFileEvent.resolve(requestCode, resultCode, data)
    }

    override fun onStart() {
        super.onStart()
        requireActivity().setTitle(R.string.install)

        // Allow markwon to run in viewmodel scope
        binding.releaseNotes.coroutineScope = viewModel.viewModelScope
        BaseDownloadService.observeProgress(this, viewModel::onProgressUpdate)
    }

}
