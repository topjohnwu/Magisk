package com.topjohnwu.magisk.redesign.compat

import android.os.Bundle
import android.view.View
import androidx.databinding.ViewDataBinding
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.ui.base.MagiskActivity

abstract class CompatActivity<ViewModel : CompatViewModel, Binding : ViewDataBinding> :
    MagiskActivity<ViewModel, Binding>(), CompatView<ViewModel> {

    override val viewRoot: View get() = binding.root
    override val navigation: CompatNavigationDelegate<CompatActivity<ViewModel, Binding>>? by lazy {
        CompatNavigationDelegate(this)
    }

    private val delegate by lazy { CompatDelegate(this) }

    internal abstract val navHost: Int

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        delegate.onCreate()
        navigation?.onCreate(savedInstanceState)
    }

    override fun onResume() {
        super.onResume()

        delegate.onResume()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        navigation?.onSaveInstanceState(outState)
    }

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)

        delegate.onEventExecute(event, this)
    }

    override fun onBackPressed() {
        if (navigation?.onBackPressed()?.not() == true) {
            super.onBackPressed()
        }
    }

    protected fun ViewEvent.dispatchOnSelf() = onEventDispatched(this)

}