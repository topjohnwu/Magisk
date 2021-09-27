package com.topjohnwu.magisk.arch

import android.os.Bundle
import android.view.KeyEvent
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.databinding.DataBindingUtil
import androidx.databinding.OnRebindCallback
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.ktx.startAnimations

abstract class BaseUIFragment<VM : BaseViewModel, Binding : ViewDataBinding> :
    Fragment(), BaseUIComponent<VM> {

    val activity get() = requireActivity() as BaseUIActivity<*, *>
    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int

    override val viewRoot: View get() = binding.root
    private val navigation get() = activity.navigation

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

    override fun onStart() {
        super.onStart()
        activity.supportActionBar?.subtitle = null
    }

    override fun onEventDispatched(event: ViewEvent) = when(event) {
        is ContextExecutor -> event(requireContext())
        is ActivityExecutor -> event(activity)
        is FragmentExecutor -> event(this)
        else -> Unit
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
