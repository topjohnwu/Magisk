package com.topjohnwu.magisk.ui.base

import android.content.Intent
import android.os.Bundle
import androidx.annotation.CallSuper
import androidx.core.net.toUri
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import com.ncapdevi.fragnav.FragNavController
import com.ncapdevi.fragnav.FragNavTransactionOptions
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.model.navigation.MagiskAnimBuilder
import com.topjohnwu.magisk.model.navigation.MagiskNavigationEvent
import com.topjohnwu.magisk.model.navigation.Navigator
import com.topjohnwu.magisk.utils.Utils
import timber.log.Timber
import kotlin.reflect.KClass


abstract class MagiskActivity<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
    MagiskLeanbackActivity<ViewModel, Binding>(), FragNavController.RootFragmentListener,
    Navigator {

    override val numberOfRootFragments: Int get() = baseFragments.size
    override val baseFragments: List<KClass<out Fragment>> = listOf()

    protected open val defaultPosition: Int = 0

    protected val navigationController by lazy {
        if (navHostId == 0) throw IllegalStateException("Did you forget to override \"navHostId\"?")
        FragNavController(supportFragmentManager, navHostId)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        navigationController.apply {
            rootFragmentListener = this@MagiskActivity
            initialize(defaultPosition, savedInstanceState)
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        navigationController.onSaveInstanceState(outState)
    }

    @CallSuper
    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is MagiskNavigationEvent -> navigateTo(event)
        }
    }

    override fun getRootFragment(index: Int) = baseFragments[index].java.newInstance()

    override fun navigateTo(event: MagiskNavigationEvent) {
        val directions = event.navDirections

        navigationController.defaultTransactionOptions = FragNavTransactionOptions.newBuilder()
            .customAnimations(event.animOptions)
            .build()

        navigationController.currentStack
            ?.indexOfFirst { it.javaClass == event.navOptions.popUpTo }
            ?.let { if (it == -1) null else it } // invalidate if class is not found
            ?.let { if (event.navOptions.inclusive) it + 1 else it }
            ?.let { navigationController.popFragments(it) }

        when (directions.isActivity) {
            true -> navigateToActivity(event)
            else -> navigateToFragment(event)
        }
    }

    private fun navigateToActivity(event: MagiskNavigationEvent) {
        val destination = event.navDirections.destination?.java ?: let {
            Timber.e("Cannot navigate to null destination")
            return
        }
        val options = event.navOptions

        Intent(this, destination)
            .putExtras(event.navDirections.args)
            .apply {
                if (options.singleTop) addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP)
                if (options.clearTask) addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK)
            }
            .let { startActivity(it) }
    }

    private fun navigateToFragment(event: MagiskNavigationEvent) {
        val destination = event.navDirections.destination?.java ?: let {
            Timber.e("Cannot navigate to null destination")
            return
        }

        when (val index = baseFragments.indexOfFirst { it.java.name == destination.name }) {
            -1 -> destination.newInstance()
                .apply { arguments = event.navDirections.args }
                .let { navigationController.pushFragment(it) }
            // When it's desired that fragments of same class are put on top of one another edit this
            else -> navigationController.switchTab(index)
        }
    }

    override fun onBackPressed() {
        val fragment = navigationController.currentFrag as? MagiskFragment<*, *>

        if (fragment?.onBackPressed() == true) {
            return
        }

        try {
            navigationController.popFragment()
        } catch (e: UnsupportedOperationException) {
            super.onBackPressed()
        }
    }

    fun openUrl(url: String) = Utils.openLink(this, url.toUri())

    private fun FragNavTransactionOptions.Builder.customAnimations(options: MagiskAnimBuilder) =
        customAnimations(options.enter, options.exit, options.popEnter, options.popExit)

}
