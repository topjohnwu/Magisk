package com.topjohnwu.magisk.events

import android.content.Context
import android.view.View
import androidx.annotation.StringRes
import androidx.navigation.NavDirections
import com.google.android.material.snackbar.Snackbar
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.ContextExecutor
import com.topjohnwu.magisk.arch.NavigationActivity
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.core.base.ContentResultCallback
import com.topjohnwu.magisk.core.base.relaunch
import com.topjohnwu.magisk.utils.TextHolder
import com.topjohnwu.magisk.utils.asText
import com.topjohnwu.magisk.view.MagiskDialog
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
        activity.relaunch()
    }
}

class AuthEvent(
    private val callback: () -> Unit
) : ViewEvent(), ActivityExecutor {

    override fun invoke(activity: UIActivity<*>) {
        activity.withAuthentication { if (it) callback() }
    }
}

class GetContentEvent(
    private val type: String,
    private val callback: ContentResultCallback
) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        activity.getContent(type, callback)
    }
}

class NavigationEvent(
    private val directions: NavDirections,
    private val pop: Boolean
) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        (activity as? NavigationActivity<*>)?.apply {
            if (pop) navigation.popBackStack()
            directions.navigate()
        }
    }
}

class AddHomeIconEvent : ViewEvent(), ContextExecutor {
    override fun invoke(context: Context) {
        Shortcuts.addHomeIcon(context)
    }
}

class SnackbarEvent(
    private val msg: TextHolder,
    private val length: Int = Snackbar.LENGTH_SHORT,
    private val builder: Snackbar.() -> Unit = {}
) : ViewEvent(), ActivityExecutor {

    constructor(
        @StringRes res: Int,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) : this(res.asText(), length, builder)

    constructor(
        msg: String,
        length: Int = Snackbar.LENGTH_SHORT,
        builder: Snackbar.() -> Unit = {}
    ) : this(msg.asText(), length, builder)

    override fun invoke(activity: UIActivity<*>) {
        activity.showSnackbar(msg.getText(activity.resources), length, builder)
    }
}

class DialogEvent(
    private val builder: DialogBuilder
) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: UIActivity<*>) {
        MagiskDialog(activity).apply(builder::build).show()
    }
}

interface DialogBuilder {
    fun build(dialog: MagiskDialog)
}
