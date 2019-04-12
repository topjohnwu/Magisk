package com.topjohnwu.magisk.ui.base

import androidx.databinding.ViewDataBinding
import com.skoumal.teanity.view.TeanityFragment


abstract class MagiskFragment<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
    TeanityFragment<ViewModel, Binding>() {

    protected val magiskActivity get() = activity as MagiskActivity<*, *>

    fun openLink(url: String) {
        magiskActivity.openUrl(url)
    }

}
