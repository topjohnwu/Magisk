package com.topjohnwu.magisk.model.navigation

import androidx.navigation.NavDirections
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.model.events.ActivityExecutor
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.ui.base.BaseUIActivity

class NavigationWrapper(
    private val directions: NavDirections
) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: BaseActivity) {
        if (activity !is BaseUIActivity<*, *>) return
        activity.apply {
            directions.navigate()
        }
    }
}