package com.topjohnwu.magisk.ui.base

import androidx.databinding.ViewDataBinding
import com.skoumal.teanity.view.TeanityActivity


abstract class MagiskActivity<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
    TeanityActivity<ViewModel, Binding>()
