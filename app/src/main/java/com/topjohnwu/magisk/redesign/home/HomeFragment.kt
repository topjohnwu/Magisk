package com.topjohnwu.magisk.redesign.home

import android.graphics.Insets
import androidx.lifecycle.Observer
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHomeMd2Binding
import com.topjohnwu.magisk.model.download.RemoteFileService
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel
import timber.log.Timber
import kotlin.math.roundToInt

class HomeFragment : CompatFragment<HomeViewModel, FragmentHomeMd2Binding>() {

    override val layoutRes = R.layout.fragment_home_md2
    override val viewModel by viewModel<HomeViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onStart() {
        super.onStart()
        RemoteFileService.progressBroadcast.observe(this, Observer {
            when (it.second) {
                is DownloadSubject.Magisk.Download,
                is DownloadSubject.Magisk.Flash -> viewModel.stateMagiskProgress.value =
                    it.first.times(100f).roundToInt().also { Timber.i("Progress: $it") }
                is DownloadSubject.Manager -> viewModel.stateManagerProgress.value =
                    it.first.times(100f).roundToInt().also { Timber.i("Progress: $it") }
            }
        })
        activity.title = resources.getString(R.string.section_home)
    }
}