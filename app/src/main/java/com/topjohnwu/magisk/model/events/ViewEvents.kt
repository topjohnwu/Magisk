package com.topjohnwu.magisk.model.events

import android.app.Activity
import android.content.Context
import android.content.Intent
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.model.module.Repo
import com.topjohnwu.magisk.core.utils.SafetyNetHelper
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.DynamicClassLoader
import com.topjohnwu.magisk.extensions.OnErrorListener
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.extensions.writeTo
import com.topjohnwu.magisk.utils.RxBus
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.superuser.Shell
import dalvik.system.DexFile
import io.reactivex.Completable
import io.reactivex.subjects.PublishSubject
import org.json.JSONObject
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import java.io.File
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

class UpdateSafetyNetEvent : ViewEvent(), ContextExecutor, KoinComponent, SafetyNetHelper.Callback {

    private val magiskRepo by inject<MagiskRepository>()
    private val rxBus by inject<RxBus>()

    private lateinit var apk: File
    private lateinit var dex: File

    override fun invoke(context: Context) {
        apk = File("${context.filesDir.parent}/snet", "snet.jar")
        dex = File(apk.parent, "snet.dex")

        attest(context) {
            // Download and retry
            Shell.sh("rm -rf " + apk.parent).exec()
            apk.parentFile?.mkdir()
            download(context, true)
        }
    }

    private fun attest(context: Context, onError: OnErrorListener) {
        Completable.fromAction {
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
                .invoke(null, SafetyNetHelper::class.java, context, this) as SafetyNetHelper

            if (helper.version < Const.SNET_EXT_VER)
                throw Exception()

            helper.attest()
        }.subscribeK(onError = onError)
    }

    @Suppress("SameParameterValue")
    private fun download(context: Context, askUser: Boolean) {
        fun downloadInternal() = magiskRepo.fetchSafetynet()
            .map { it.byteStream().writeTo(apk) }
            .subscribeK { attest(context) {
                Timber.e(it)
                rxBus.post(SafetyNetResult())
            } }

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
                onClick { rxBus.post(SafetyNetResult(dismiss = true)) }
            }
            .reveal()
    }

    override fun onResponse(response: JSONObject?) {
        rxBus.post(SafetyNetResult(response))
    }
}

class ViewActionEvent(val action: BaseActivity.() -> Unit) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: BaseActivity) = activity.run(action)
}

class OpenChangelogEvent(val item: Repo) : ViewEvent(), ContextExecutor {
    override fun invoke(context: Context) {
        MarkDownWindow.show(context, null, item.readme)
    }
}

class PermissionEvent(
    val permissions: List<String>,
    val callback: PublishSubject<Boolean>
) : ViewEvent(), ActivityExecutor {

    override fun invoke(activity: BaseActivity) =
        activity.withPermissions(*permissions.toTypedArray()) {
            onSuccess {
                callback.onNext(true)
            }
            onFailure {
                callback.onNext(false)
                callback.onError(SecurityException("User refused permissions"))
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
