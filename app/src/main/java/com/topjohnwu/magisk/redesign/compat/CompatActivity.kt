package com.topjohnwu.magisk.redesign.compat

import android.os.Bundle
import android.view.View
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.ui.base.MagiskActivity

abstract class CompatActivity<ViewModel : CompatViewModel, Binding : ViewDataBinding> :
    MagiskActivity<ViewModel, Binding>(), CompatView<ViewModel> {

    override val viewRoot: View get() = binding.root

    private val delegate by lazy { CompatDelegate(this) }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        delegate.ensureInsets()
    }

    override fun onResume() {
        super.onResume()

        delegate.onResume()
    }

}