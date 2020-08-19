package com.topjohnwu.magisk.events

import android.app.Activity
import android.content.Context
import android.content.Intent
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.ContextExecutor
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.arch.ViewEventWithScope
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.view.MarkDownWindow
import kotlinx.coroutines.launch

class ViewActionEvent(val action: BaseActivity.() -> Unit) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: BaseActivity) = action(activity)
}

class OpenChangelogEvent(val item: Repo) : ViewEventWithScope(), ContextExecutor {
    override fun invoke(context: Context) {
        scope.launch {
            MarkDownWindow.show(context, null, item::readme)
        }
    }
}

class PermissionEvent(
    private val permissions: List<String>,
    private val callback: (Boolean) -> Unit
) : ViewEvent(), ActivityExecutor {

    override fun invoke(activity: BaseActivity) =
        activity.withPermissions(*permissions.toTypedArray()) {
            onSuccess {
                callback(true)
            }
            onFailure {
                callback(false)
            }
        }
}

class BackPressEvent : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: BaseActivity) {
        activity.onBackPressed()
    }
}

class DieEvent : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: BaseActivity) {
        activity.finish()
    }
}

class RecreateEvent : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: BaseActivity) {
        activity.recreate()
    }
}

class RequestFileEvent : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: BaseActivity) {
        Intent(Intent.ACTION_GET_CONTENT)
            .setType("*/*")
            .addCategory(Intent.CATEGORY_OPENABLE)
            .also { activity.startActivityForResult(it, REQUEST_CODE) }
    }

    companion object {
        private const val REQUEST_CODE = 10
        fun resolve(requestCode: Int, resultCode: Int, data: Intent?) = data
            ?.takeIf { resultCode == Activity.RESULT_OK }
            ?.takeIf { requestCode == REQUEST_CODE }
            ?.data
    }
}
