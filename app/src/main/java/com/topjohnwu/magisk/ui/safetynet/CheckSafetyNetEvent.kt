package com.topjohnwu.magisk.ui.safetynet

import android.content.Context
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.ContextExecutor
import com.topjohnwu.magisk.arch.ViewEventWithScope
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.data.network.GithubRawServices
import com.topjohnwu.magisk.ktx.DynamicClassLoader
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.superuser.Shell
import dalvik.system.DexFile
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.json.JSONObject
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import java.io.File
import java.io.IOException
import java.lang.reflect.InvocationHandler

@Suppress("DEPRECATION")
class CheckSafetyNetEvent(
    private val callback: (SafetyNetResult) -> Unit = {}
) : ViewEventWithScope(), ContextExecutor, KoinComponent, SafetyNetHelper.Callback {

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
                    .invoke(
                        null, SafetyNetHelper::class.java,
                        context, this@CheckSafetyNetEvent
                    ) as SafetyNetHelper

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
            .onDismiss {
                callback(SafetyNetResult(dismiss = true))
            }
            .reveal()
    }

    override fun onResponse(response: JSONObject?) {
        callback(SafetyNetResult(response))
    }
}
