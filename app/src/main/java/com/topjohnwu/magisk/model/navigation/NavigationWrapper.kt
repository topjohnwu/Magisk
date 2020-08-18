package com.topjohnwu.magisk.model.navigation

import androidx.navigation.NavDirections
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.core.base.BaseActivity

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
