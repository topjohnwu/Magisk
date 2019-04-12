package com.topjohnwu.magisk.ui.base

import androidx.core.net.toUri
import androidx.databinding.ViewDataBinding
import com.skoumal.teanity.view.TeanityActivity
import com.topjohnwu.magisk.utils.Utils


abstract class MagiskActivity<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
    TeanityActivity<ViewModel, Binding>() {

    fun openUrl(url: String) = Utils.openLink(this, url.toUri())

}
