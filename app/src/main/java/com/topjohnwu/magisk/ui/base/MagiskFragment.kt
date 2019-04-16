package com.topjohnwu.magisk.ui.base

import androidx.annotation.CallSuper
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import com.skoumal.teanity.view.TeanityFragment
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.model.navigation.MagiskNavigationEvent
import com.topjohnwu.magisk.model.navigation.Navigator
import kotlin.reflect.KClass


abstract class MagiskFragment<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
    TeanityFragment<ViewModel, Binding>(), Navigator {

    protected val magiskActivity get() = activity as MagiskActivity<*, *>

    // We don't need nested fragments
    override val baseFragments: List<KClass<Fragment>> = listOf()

    override fun navigateTo(event: MagiskNavigationEvent) = magiskActivity.navigateTo(event)

    @CallSuper
    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is MagiskNavigationEvent -> navigateTo(event)
        }
    }

    fun openLink(url: String) = magiskActivity.openUrl(url)

    open fun onBackPressed(): Boolean = false

}
