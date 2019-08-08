package com.topjohnwu.magisk.ui.home

import android.content.Context
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.databinding.FragmentMagiskBinding
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.magisk.extensions.writeTo
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.ui.base.MagiskFragment
import com.topjohnwu.magisk.utils.DynamicClassLoader
import com.topjohnwu.magisk.utils.SafetyNetHelper
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.magisk.view.dialogs.*
import com.topjohnwu.superuser.Shell
import dalvik.system.DexFile
import io.reactivex.Completable
import org.koin.androidx.viewmodel.ext.android.viewModel
import java.io.File
import java.lang.reflect.InvocationHandler

class HomeFragment : MagiskFragment<HomeViewModel, FragmentMagiskBinding>(),
    SafetyNetHelper.Callback {

    override val layoutRes: Int = R.layout.fragment_magisk
    override val viewModel: HomeViewModel by viewModel()

    private val magiskRepo: MagiskRepository by inject()
    private val EXT_APK by lazy { File("${activity.filesDir.parent}/snet", "snet.jar") }

    override fun onResponse(responseCode: Int) = viewModel.finishSafetyNetCheck(responseCode)

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is OpenLinkEvent -> openLink(event.url)
            is ManagerInstallEvent -> installManager()
            is MagiskInstallEvent -> installMagisk()
            is UninstallEvent -> uninstall()
            is ManagerChangelogEvent -> changelogManager()
            is EnvFixEvent -> fixEnv()
            is UpdateSafetyNetEvent -> updateSafetyNet(false)
        }
    }

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        requireActivity().setTitle(R.string.magisk)
    }

    private fun installMagisk() {
        // Show Manager update first
        if (Info.remote.app.versionCode > BuildConfig.VERSION_CODE) {
            installManager()
            return
        }

        MagiskInstallDialog(requireActivity() as MagiskActivity<*, *>).show()
    }

    private fun installManager() = ManagerInstallDialog(requireActivity()).show()
    private fun uninstall() = UninstallDialog(requireActivity()).show()
    private fun fixEnv() = EnvFixDialog(requireActivity()).show()

    private fun changelogManager() = MarkDownWindow
        .show(requireActivity(), null, resources.openRawResource(R.raw.changelog))

    private fun downloadSafetyNet(requiresUserInput: Boolean = true) {
        fun download() = magiskRepo.fetchSafetynet()
                .map { it.byteStream().writeTo(EXT_APK) }
                .subscribeK { updateSafetyNet(true) }

        if (!requiresUserInput) {
            download()
            return
        }

        CustomAlertDialog(requireActivity())
            .setTitle(R.string.proprietary_title)
            .setMessage(R.string.proprietary_notice)
            .setCancelable(false)
            .setPositiveButton(R.string.yes) { _, _ -> download() }
            .setNegativeButton(R.string.no_thanks) { _, _ -> viewModel.finishSafetyNetCheck(-2) }
            .show()
    }

    private fun updateSafetyNet(dieOnError: Boolean) {
        Completable.fromAction {
            val loader = DynamicClassLoader(EXT_APK)
            val dex = DexFile.loadDex(EXT_APK.path, EXT_APK.parent, 0)

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

            val helper = helperClass.getMethod("get",
                    Class::class.java, Context::class.java, Any::class.java)
                    .invoke(null, SafetyNetHelper::class.java, activity, this) as SafetyNetHelper

            if (helper.version < Const.SNET_EXT_VER)
                throw Exception()

            helper.attest()
        }.subscribeK(onError = {
            if (dieOnError) {
                viewModel.finishSafetyNetCheck(-1)
            } else {
                Shell.sh("rm -rf " + EXT_APK.parent).exec()
                EXT_APK.parentFile?.mkdir()
                downloadSafetyNet(!dieOnError)
            }
        })
    }
}

