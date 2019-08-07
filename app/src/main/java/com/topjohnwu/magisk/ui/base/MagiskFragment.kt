package com.topjohnwu.magisk.ui.base

import androidx.annotation.CallSuper
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import com.skoumal.teanity.view.TeanityFragment
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.model.events.BackPressEvent
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.ViewActionEvent
import com.topjohnwu.magisk.model.navigation.MagiskNavigationEvent
import com.topjohnwu.magisk.model.navigation.Navigator
import com.topjohnwu.magisk.model.permissions.PermissionRequestBuilder
import kotlin.reflect.KClass


abstract class MagiskFragment<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
    TeanityFragment<ViewModel, Binding>(), Navigator {

    protected val activity get() = requireActivity() as MagiskActivity<*, *>

    // We don't need nested fragments
    override val baseFragments: List<KClass<Fragment>> = listOf()

    override fun navigateTo(event: MagiskNavigationEvent) = activity.navigateTo(event)

    @CallSuper
    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
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
