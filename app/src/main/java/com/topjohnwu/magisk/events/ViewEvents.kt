package com.topjohnwu.magisk.events

import android.content.ActivityNotFoundException
import android.content.Context
import android.net.Uri
import android.view.View
import android.widget.Toast
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.MainDirections
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.*
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.Shortcuts

class PermissionEvent(
    private val permission: String,
    private val callback: (Boolean) -> Unit
) : ViewEvent(), ActivityExecutor {

    override fun invoke(activity: UIActivity<*>) =
        activity.withPermission(permission, callback)
}

class BackPressEvent : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        activity.onBackPressed()
    }
}

class DieEvent : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        activity.finish()
    }
}

class ShowUIEvent(private val delegate: View.AccessibilityDelegate?)
    : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        activity.setContentView()
        activity.setAccessibilityDelegate(delegate)
    }
}

class RecreateEvent : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        activity.recreate()
    }
}

class MagiskInstallFileEvent(
    private val callback: (Uri) -> Unit
) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        try {
            activity.getContent("*/*", callback)
            Utils.toast(R.string.patch_file_msg, Toast.LENGTH_LONG)
        } catch (e: ActivityNotFoundException) {
            Utils.toast(R.string.app_not_found, Toast.LENGTH_SHORT)
        }
    }
}

class NavigationEvent(
    private val directions: NavDirections,
    private val pop: Boolean
) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        (activity as? NavigationActivity<*>)?.apply {
            if (pop) navigation?.popBackStack()
            directions.navigate()
        }
    }
}

class AddHomeIconEvent : ViewEvent(), ContextExecutor {
    override fun invoke(context: Context) {
        Shortcuts.addHomeIcon(context)
    }
}

class SelectModuleEvent : ViewEvent(), FragmentExecutor {
    override fun invoke(fragment: BaseFragment<*>) {
        try {
            fragment.apply {
                activity?.getContent("application/zip") {
                    MainDirections.actionFlashFragment(Const.Value.FLASH_ZIP, it).navigate()
                }
            }
        } catch (e: ActivityNotFoundException) {
            Utils.toast(R.string.app_not_found, Toast.LENGTH_SHORT)
        }
    }
}
