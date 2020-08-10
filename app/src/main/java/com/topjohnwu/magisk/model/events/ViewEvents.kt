package com.topjohnwu.magisk.model.events

import android.app.Activity
import android.content.Context
import android.content.Intent
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.core.utils.SafetyNetHelper
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.ktx.DynamicClassLoader
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.magisk.ui.safetynet.SafetyNetResult
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.superuser.Shell
import dalvik.system.DexFile
import kotlinx.coroutines.*
import org.json.JSONObject
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import java.io.File
import java.io.IOException
import java.lang.reflect.InvocationHandler

/**
 * Class for passing events from ViewModels to Activities/Fragments
 * Variable [handled] used so each event is handled only once
 * (see https://medium.com/google-developers/livedata-with-snackbar-navigation-and-other-events-the-singleliveevent-case-ac2622673150)
 * Use [ViewEventObserver] for observing these events
 */
abstract class ViewEvent {

    var handled = false
}

abstract class ViewEventsWithScope: ViewEvent() {
    lateinit var scope: CoroutineScope
}

class CheckSafetyNetEvent(
    private val callback: (SafetyNetResult) -> Unit = {}
) : ViewEventsWithScope(), ContextExecutor, KoinComponent, SafetyNetHelper.Callback {

    private val svc by inject<GithubRawServices>()

    private lateinit var apk: File
    private lateinit var dex: File

    override fun invoke(context: Context) {
        apk = File("${context.filesDir.parent}/snet", "snet.jar")
        dex = File(apk.parent, "snet.dex")

        scope.launch {
            attest(context) {
                // Download and retry
                withContext(Dispatchers.IO) {
                    Shell.sh("rm -rf " + apk.parent).exec()
                    apk.parentFile?.mkdir()
                }
                download(context, true)
            }
        }
    }

    private suspend fun attest(context: Context, onError: suspend (Exception) -> Unit) {
        try {
            val helper = withContext(Dispatchers.IO) {
                val loader = DynamicClassLoader(apk)
                val dex = DexFile.loadDex(apk.path, dex.path, 0)

                // Scan through the dex and find our helper class
                var helperClass: Class<*>? = null
                for (className in dex.entries()) {
                    if (className.startsWith("x.")) {
                        val cls = loader.loadClass(className)
                        if (InvocationHandler::class.java.isAssignableFrom(cls)) {
                            helperClass = cls
                            break
                        }
                    }
                }
                helperClass ?: throw Exception()

                val helper = helperClass
                    .getMethod("get", Class::class.java, Context::class.java, Any::class.java)
                    .invoke(null, SafetyNetHelper::class.java,
                        context, this@CheckSafetyNetEvent) as SafetyNetHelper

                if (helper.version < Const.SNET_EXT_VER)
                    throw Exception()
                helper
            }
            helper.attest()
        } catch (e: Exception) {
            if (e is CancellationException)
                throw e
            onError(e)
        }
    }

    @Suppress("SameParameterValue")
    private fun download(context: Context, askUser: Boolean) {
        fun downloadInternal() = scope.launch {
            val abort: suspend (Exception) -> Unit = {
                Timber.e(it)
                callback(SafetyNetResult())
            }
            try {
                withContext(Dispatchers.IO) {
                    svc.fetchSafetynet().byteStream().writeTo(apk)
                }
                attest(context, abort)
            } catch (e: IOException) {
                if (e is CancellationException)
                    throw e
                abort(e)
            }
        }

        if (!askUser) {
            downloadInternal()
            return
        }

        MagiskDialog(context)
            .applyTitle(R.string.proprietary_title)
            .applyMessage(R.string.proprietary_notice)
            .cancellable(false)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.yes
                onClick { downloadInternal() }
            }
            .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = android.R.string.cancel
                onClick { callback(SafetyNetResult(dismiss = true)) }
            }
            .reveal()
    }

    override fun onResponse(response: JSONObject?) {
        callback(SafetyNetResult(response))
    }
}

class ViewActionEvent(val action: BaseActivity.() -> Unit) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: BaseActivity) = activity.run(action)
}

class OpenChangelogEvent(val item: Repo) : ViewEventsWithScope(), ContextExecutor {
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
