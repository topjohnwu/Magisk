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
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.extensions.writeTo
import com.topjohnwu.magisk.utils.RxBus
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.superuser.Shell
import dalvik.system.DexFile
import io.reactivex.Completable
import io.reactivex.subjects.PublishSubject
import org.koin.core.KoinComponent
import org.koin.core.inject
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

    private lateinit var EXT_APK: File
    private lateinit var EXT_DEX: File

    override fun invoke(context: Context) {
        val die = ::EXT_APK.isInitialized

        EXT_APK = File("${context.filesDir.parent}/snet", "snet.jar")
        EXT_DEX = File(EXT_APK.parent, "snet.dex")

        Completable.fromAction {
            val loader = DynamicClassLoader(EXT_APK)
            val dex = DexFile.loadDex(EXT_APK.path, EXT_DEX.path, 0)

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

            val helper = helperClass.getMethod(
                "get",
                Class::class.java, Context::class.java, Any::class.java
            )
                .invoke(null, SafetyNetHelper::class.java, context, this) as SafetyNetHelper

            if (helper.version < Const.SNET_EXT_VER)
                throw Exception()

            helper.attest()
        }.subscribeK(onError = {
            if (die) {
                rxBus.post(SafetyNetResult(-1))
            } else {
                Shell.sh("rm -rf " + EXT_APK.parent).exec()
                EXT_APK.parentFile?.mkdir()
                download(context, true)
            }
        })
    }

    @Suppress("SameParameterValue")
    private fun download(context: Context, askUser: Boolean) {
        fun downloadInternal() = magiskRepo.fetchSafetynet()
            .map { it.byteStream().writeTo(EXT_APK) }
            .subscribeK { invoke(context) }

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
                titleRes = android.R.string.no
                onClick { rxBus.post(SafetyNetResult(-2)) }
            }
            .reveal()
    }

    override fun onResponse(responseCode: Int) {
        rxBus.post(SafetyNetResult(responseCode))
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
