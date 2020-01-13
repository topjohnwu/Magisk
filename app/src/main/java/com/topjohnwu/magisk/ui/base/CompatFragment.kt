package com.topjohnwu.magisk.ui.base

import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import androidx.databinding.OnRebindCallback
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import com.topjohnwu.magisk.extensions.startAnimations
import com.topjohnwu.magisk.model.events.ViewEvent

// TODO (diareuse): Merge into BaseUIFragment after all legacy UI is migrated

abstract class CompatFragment<ViewModel : BaseViewModel, Binding : ViewDataBinding>
    : BaseUIFragment<ViewModel, Binding>(), CompatView<ViewModel> {

    override val viewRoot: View get() = binding.root
    override val navigation by lazy { compatActivity.navigation }

    private val delegate by lazy { CompatDelegate(this) }

    protected val compatActivity get() = requireActivity() as CompatActivity<*, *>

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.addOnRebindCallback(object : OnRebindCallback<Binding>() {
            override fun onPreBind(binding: Binding): Boolean {
                this@CompatFragment.onPreBind(binding)
                return true
            }
        })

        delegate.onCreate()
    }

    override fun onResume() {
        super.onResume()

        delegate.onResume()
    }

    override fun onEventDispatched(event: ViewEvent) {
        delegate.onEventExecute(event, this)
    }

    protected open fun onPreBind(binding: Binding) {
        (binding.root as? ViewGroup)?.startAnimations()
    }

    protected fun ViewEvent.dispatchOnSelf() = delegate.onEventExecute(this, this@CompatFragment)

}

fun Fragment.hideKeyboard() {
    activity?.hideKeyboard()
}
