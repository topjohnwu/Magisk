package com.topjohnwu.magisk.ui.base

import android.os.Bundle
import android.view.KeyEvent
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
import com.topjohnwu.magisk.ktx.startAnimations
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.model.events.ContextExecutor
import com.topjohnwu.magisk.model.events.FragmentExecutor
import com.topjohnwu.magisk.model.events.ViewEvent

abstract class BaseUIFragment<VM : BaseViewModel, Binding : ViewDataBinding> :
    Fragment(), BaseUIComponent<VM> {

    protected val activity get() = requireActivity() as BaseUIActivity<*, *>
    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int

    override val viewRoot: View get() = binding.root
    private val navigation get() = activity.navigation

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        startObserveEvents()
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = DataBindingUtil.inflate<Binding>(inflater, layoutRes, container, false).also {
            it.setVariable(BR.viewModel, viewModel)
            it.lifecycleOwner = this
        }
        return binding.root
    }

    override fun onEventDispatched(event: ViewEvent) {
        (event as? ContextExecutor)?.invoke(requireContext())
        (event as? FragmentExecutor)?.invoke(this)
        (event as? ActivityExecutor)?.invoke(activity)
    }

    open fun onKeyEvent(event: KeyEvent): Boolean {
        return false
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
        ensureInsets()
    }

    override fun onResume() {
        super.onResume()
        viewModel.requestRefresh()
    }

    protected open fun onPreBind(binding: Binding) {
        (binding.root as? ViewGroup)?.startAnimations()
    }

    fun NavDirections.navigate() {
        navigation?.navigate(this)
    }

}

interface ReselectionTarget {

    fun onReselected()

}
