package com.topjohnwu.magisk.ui.base

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.graphics.Insets
import androidx.databinding.DataBindingUtil
import androidx.databinding.OnRebindCallback
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.extensions.startAnimations
import com.topjohnwu.magisk.model.events.EventHandler
import com.topjohnwu.magisk.model.events.ViewEvent

abstract class BaseUIFragment<ViewModel : BaseViewModel, Binding : ViewDataBinding> :
    Fragment(), CompatView<ViewModel>, EventHandler {

    protected val activity get() = requireActivity() as BaseUIActivity<*, *>
    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int

    override val viewRoot: View get() = binding.root
    override val navigation get() = activity.navigation
    private val delegate by lazy { CompatDelegate(this) }

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewModel.viewEvents.observe(this, viewEventObserver)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = DataBindingUtil.inflate<Binding>(inflater, layoutRes, container, false).apply {
            setVariable(BR.viewModel, viewModel)
            lifecycleOwner = this@BaseUIFragment
        }

        return binding.root
    }

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        delegate.onEventExecute(event, this)
    }

    open fun onBackPressed(): Boolean = false


    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.addOnRebindCallback(object : OnRebindCallback<Binding>() {
            override fun onPreBind(binding: Binding): Boolean {
                this@BaseUIFragment.onPreBind(binding)
                return true
            }
        })

        delegate.onCreate()
    }

    override fun onResume() {
        super.onResume()

        delegate.onResume()
    }

    protected open fun onPreBind(binding: Binding) {
        (binding.root as? ViewGroup)?.startAnimations()
    }

    protected fun ViewEvent.dispatchOnSelf() = delegate.onEventExecute(this, this@BaseUIFragment)

    fun NavDirections.navigate() {
        navigation?.navigate(this)
    }

}
