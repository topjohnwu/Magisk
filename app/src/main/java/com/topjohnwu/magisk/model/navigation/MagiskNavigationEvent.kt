package com.topjohnwu.magisk.model.navigation

import android.os.Bundle
import androidx.annotation.AnimRes
import androidx.annotation.AnimatorRes
import androidx.fragment.app.Fragment
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.ui.base.BaseUIActivity
import kotlin.reflect.KClass

@DslMarker
annotation class NavigationDslMarker

class MagiskNavigationEvent(
    val navDirections: MagiskNavDirectionsBuilder,
    val navOptions: MagiskNavOptions,
    val animOptions: MagiskAnimBuilder
) : ViewEvent(), ActivityExecutor {

    companion object {
        operator fun invoke(builder: Builder.() -> Unit) = Builder().apply(builder).build()
    }

    override fun invoke(activity: BaseActivity) {
        if (activity !is BaseUIActivity<*, *>) return
        activity.navigation?.navigateTo(this)
    }

    @NavigationDslMarker
    class Builder {

        private var animOptions: MagiskAnimBuilder = MagiskAnimBuilder()
        private var navOptions: MagiskNavOptions = MagiskNavOptions()
        private val directionsBuilder = MagiskNavDirectionsBuilder()

        fun args(builder: Bundle.() -> Unit) = directionsBuilder.args(builder)

        fun navAnim(builder: MagiskAnimBuilder.() -> Unit) {
            animOptions = MagiskAnimBuilder().apply(builder)
        }

        fun navOptions(builder: MagiskNavOptions.() -> Unit) {
            navOptions = MagiskNavOptions().apply(builder)
        }

        fun navDirections(builder: MagiskNavDirectionsBuilder.() -> Unit) {
            directionsBuilder.apply(builder)
        }

        internal fun build() = MagiskNavigationEvent(directionsBuilder, navOptions, animOptions)
    }
}

@NavigationDslMarker
class MagiskNavDirectionsBuilder {

    var destination: KClass<out Fragment>? = null
    var isActivity: Boolean = false
    val args: Bundle = Bundle()

    fun args(builder: Bundle.() -> Unit) = args.apply(builder)

}

@NavigationDslMarker
class MagiskNavOptions {
    var popUpTo: KClass<*>? = null
    var inclusive: Boolean = false
    var clearTask: Boolean = false
    var singleTop: Boolean = false
}

@NavigationDslMarker
class MagiskAnimBuilder {
    @AnimRes
    @AnimatorRes
    var enter = 0

    @AnimRes
    @AnimatorRes
    var exit = 0

    @AnimRes
    @AnimatorRes
    var popEnter = 0

    @AnimRes
    @AnimatorRes
    var popExit = 0

    val anySet: Boolean get() = enter != 0 || exit != 0 || popEnter != 0 || popExit != 0
}
