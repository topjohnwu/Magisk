package com.topjohnwu.magisk.redesign.compat

import android.content.Intent
import android.os.Bundle
import androidx.fragment.app.FragmentTransaction
import com.ncapdevi.fragnav.FragNavController
import com.ncapdevi.fragnav.FragNavTransactionOptions
import com.topjohnwu.magisk.model.navigation.MagiskAnimBuilder
import com.topjohnwu.magisk.model.navigation.MagiskNavigationEvent
import com.topjohnwu.magisk.model.navigation.Navigator
import timber.log.Timber

class CompatNavigationDelegate<Source>(
    private val source: Source,
    private val listener: FragNavController.TransactionListener? = null
) : FragNavController.RootFragmentListener where Source : CompatActivity<*, *>, Source : Navigator {

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


    fun onCreate(savedInstanceState: Bundle?) = controller.run {
        rootFragmentListener = source
        transactionListener = listener
        initialize(0, savedInstanceState)
    }

    fun onSaveInstanceState(outState: Bundle) =
        controller.onSaveInstanceState(outState)

    fun onBackPressed(): Boolean {
        val fragment = controller.currentFrag as? CompatFragment<*, *>

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
        val destination = event.navDirections.destination?.java ?: let {
            Timber.e("Cannot navigate to null destination")
            return
        }

        source.baseFragments
            .indexOfFirst { it.java.name == destination.name }
            .takeIf { it > 0 }
            ?.let { controller.switchTab(it) } ?: destination.newInstance()
            .also { it.arguments = event.navDirections.args }
            .let { controller.pushFragment(it) }
    }

    private fun FragNavTransactionOptions.Builder.customAnimations(options: MagiskAnimBuilder) =
        customAnimations(options.enter, options.exit, options.popEnter, options.popExit).apply {
            if (!options.anySet) {
                transition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
            }
        }

}