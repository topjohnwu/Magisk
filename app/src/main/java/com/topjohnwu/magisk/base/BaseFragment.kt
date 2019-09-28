package com.topjohnwu.magisk.base

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.annotation.CallSuper
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.base.viewmodel.BaseViewModel
import com.topjohnwu.magisk.model.events.EventHandler
import com.topjohnwu.magisk.model.events.ViewEvent

abstract class BaseFragment<ViewModel : BaseViewModel, Binding : ViewDataBinding> :
    Fragment(), EventHandler {

    protected val activity get() = requireActivity() as BaseActivity<*, *>
    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int
    protected abstract val viewModel: ViewModel

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
            lifecycleOwner = this@BaseFragment
        }

        return binding.root
    }

    @CallSuper
    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        activity.onEventDispatched(event)
    }

    open fun onBackPressed(): Boolean = false

}
