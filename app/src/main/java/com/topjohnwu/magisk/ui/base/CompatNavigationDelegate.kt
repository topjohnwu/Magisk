package com.topjohnwu.magisk.ui.base

import android.content.Intent
import android.os.Bundle
import com.ncapdevi.fragnav.FragNavController
import com.ncapdevi.fragnav.FragNavTransactionOptions
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.navigation.MagiskAnimBuilder
import com.topjohnwu.magisk.model.navigation.MagiskNavigationEvent
import com.topjohnwu.magisk.ui.ReselectionTarget
import timber.log.Timber

class CompatNavigationDelegate<out Source>(
    private val source: Source,
    private val listener: FragNavController.TransactionListener? = null
) : FragNavController.RootFragmentListener where Source : BaseUIActivity<*, *> {

    private val controller by lazy {
        check(source.navHost != 0) { "Did you forget to override \"navHostId\"?" }
        FragNavController(source.supportFragmentManager, source.navHost)
    }

    val isRoot get() = controller.isRootFragment


    //region Listener
    override val numberOfRootFragments: Int
        get() = source.baseFragments.size

    override fun getRootFragment(index: Int) =
        source.baseFragments[index].java.newInstance()
    //endregion

    fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        controller.currentFrag?.onActivityResult(requestCode, resultCode, data)
    }

    fun onCreate(savedInstanceState: Bundle?) = controller.run {
        rootFragmentListener = this@CompatNavigationDelegate
        transactionListener = listener
        initialize(0, savedInstanceState)
    }

    fun onSaveInstanceState(outState: Bundle) =
        controller.onSaveInstanceState(outState)

    fun onReselected() {
        (controller.currentFrag as? ReselectionTarget)?.onReselected()
    }

    fun onBackPressed(): Boolean {
        val fragment = controller.currentFrag as? BaseUIFragment<*, *>

        if (fragment?.onBackPressed() == true) {
            return true
        }

        return runCatching { controller.popFragment() }.fold({ true }, { false })
    }

    // ---

    fun navigateTo(event: MagiskNavigationEvent) {
        val directions = event.navDirections

        controller.defaultTransactionOptions = FragNavTransactionOptions.newBuilder()
            .customAnimations(event.animOptions)
            .build()

        controller.currentStack
            ?.indexOfFirst { it.javaClass == event.navOptions.popUpTo }
            ?.takeIf { it != -1 } // invalidate if class is not found
            ?.let { if (event.navOptions.inclusive) it + 1 else it }
            ?.let { controller.popFragments(it) }

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

        Intent(source, destination)
            .putExtras(event.navDirections.args)
            .apply {
                if (options.singleTop) addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP)
                if (options.clearTask) addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK)
            }
            .let { source.startActivity(it) }
    }

    private fun navigateToFragment(event: MagiskNavigationEvent) {
        val destination = event.navDirections.destination ?: let {
            Timber.e("Cannot navigate to null destination")
            return
        }

        source.baseFragments
            .indexOfFirst { it == destination }
            .takeIf { it >= 0 }
            ?.let { controller.switchTab(it) } ?: destination.java.newInstance()
            .also { it.arguments = event.navDirections.args }
            .let { controller.pushFragment(it) }
    }

    private fun FragNavTransactionOptions.Builder.customAnimations(options: MagiskAnimBuilder) =
        apply {
            if (!options.anySet) customAnimations(
                R.anim.fragment_enter,
                R.anim.fragment_exit,
                R.anim.fragment_enter_pop,
                R.anim.fragment_exit_pop
            ) else customAnimations(
                options.enter,
                options.exit,
                options.popEnter,
                options.popExit
            )
        }

}
