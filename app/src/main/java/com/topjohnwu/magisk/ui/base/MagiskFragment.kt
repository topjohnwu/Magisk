package com.topjohnwu.magisk.ui.base

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.annotation.CallSuper
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import androidx.navigation.findNavController
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.extensions.snackbar
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.model.navigation.MagiskNavigationEvent
import com.topjohnwu.magisk.model.navigation.Navigator
import com.topjohnwu.magisk.model.permissions.PermissionRequestBuilder
import kotlin.reflect.KClass

abstract class MagiskFragment<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
    Fragment(), Navigator, EventHandler {

    protected val activity get() = requireActivity() as MagiskActivity<*, *>
    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int
    protected abstract val viewModel: ViewModel
    protected open val snackbarView get() = binding.root
    protected val navController get() = binding.root.findNavController()
    private val viewEventObserver = ViewEventObserver {
        onEventDispatched(it)
        if (it is SimpleViewEvent) {
            onSimpleEventDispatched(it.event)
        }
    }

    // We don't need nested fragments
    override val baseFragments: List<KClass<Fragment>> = listOf()

    override fun navigateTo(event: MagiskNavigationEvent) = activity.navigateTo(event)

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
            lifecycleOwner = this@MagiskFragment
        }

        return binding.root
    }

    @CallSuper
    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is SnackbarEvent -> snackbar(snackbarView, event.message(requireContext()), event.length, event.f)
            is BackPressEvent -> activity.onBackPressed()
            is MagiskNavigationEvent -> navigateTo(event)
            is ViewActionEvent -> event.action(requireActivity())
            is PermissionEvent -> activity.withPermissions(*event.permissions.toTypedArray()) {
                onSuccess { event.callback.onNext(true) }
                onFailure {
                    event.callback.onNext(false)
                    event.callback.onError(SecurityException("User refused permissions"))
                }
            }
        }
    }

    fun withPermissions(vararg permissions: String, builder: PermissionRequestBuilder.() -> Unit) {
        activity.withPermissions(*permissions, builder = builder)
    }

    fun openLink(url: String) = activity.openUrl(url)

    open fun onBackPressed(): Boolean = false

}
