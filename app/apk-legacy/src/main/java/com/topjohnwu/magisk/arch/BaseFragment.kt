package com.topjohnwu.magisk.arch

import android.os.Bundle
import android.view.KeyEvent
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.view.MenuProvider
import androidx.databinding.DataBindingUtil
import androidx.databinding.OnRebindCallback
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import androidx.lifecycle.Lifecycle
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.BR

abstract class BaseFragment<Binding : ViewDataBinding> : Fragment(), ViewModelHolder {

    val activity get() = getActivity() as? NavigationActivity<*>
    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int

    private val navigation get() = activity?.navigation
    open val snackbarView: View? get() = null
    open val snackbarAnchorView: View? get() = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        startObserveLiveData()
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = DataBindingUtil.inflate<Binding>(inflater, layoutRes, container, false).also {
            it.setVariable(BR.viewModel, viewModel)
            it.lifecycleOwner = viewLifecycleOwner
        }
        if (this is MenuProvider) {
            activity?.addMenuProvider(this, viewLifecycleOwner, Lifecycle.State.STARTED)
        }
        savedInstanceState?.let { viewModel.onRestoreState(it) }
        return binding.root
    }

    override fun onSaveInstanceState(outState: Bundle) {
        viewModel.onSaveState(outState)
    }

    override fun onStart() {
        super.onStart()
        activity?.supportActionBar?.subtitle = null
    }

    override fun onEventDispatched(event: ViewEvent) = when(event) {
        is ContextExecutor -> event(requireContext())
        is ActivityExecutor -> activity?.let { event(it) } ?: Unit
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
                this@BaseFragment.onPreBind(binding)
                return true
            }
        })
    }

    override fun onResume() {
        super.onResume()
        viewModel.let {
            if (it is AsyncLoadViewModel)
                it.startLoading()
        }
    }

    protected open fun onPreBind(binding: Binding) {
        (binding.root as? ViewGroup)?.startAnimations()
    }

    fun NavDirections.navigate() {
        navigation?.currentDestination?.getAction(actionId)?.let { navigation!!.navigate(this) }
    }
}
