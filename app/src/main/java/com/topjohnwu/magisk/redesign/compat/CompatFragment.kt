package com.topjohnwu.magisk.redesign.compat

import android.os.Bundle
import android.view.View
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.ui.base.MagiskFragment

abstract class CompatFragment<ViewModel : CompatViewModel, Binding : ViewDataBinding>
    : MagiskFragment<ViewModel, Binding>(), CompatView<ViewModel> {

    override val viewRoot: View get() = binding.root

    private val delegate by lazy { CompatDelegate(this) }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        delegate.ensureInsets()
    }

}