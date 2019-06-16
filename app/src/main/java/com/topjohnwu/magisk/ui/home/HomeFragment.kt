package com.topjohnwu.magisk.ui.home

import android.app.Activity
import android.os.Bundle
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.databinding.FragmentMagiskBinding
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.ui.base.MagiskFragment
import com.topjohnwu.magisk.utils.ISafetyNetHelper
import com.topjohnwu.magisk.utils.copyTo
import com.topjohnwu.magisk.utils.inject
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.magisk.view.dialogs.*
import com.topjohnwu.superuser.Shell
import dalvik.system.DexClassLoader
import org.koin.androidx.viewmodel.ext.android.viewModel
import java.io.File

class HomeFragment : MagiskFragment<HomeViewModel, FragmentMagiskBinding>(),
    ISafetyNetHelper.Callback {

    override val layoutRes: Int = R.layout.fragment_magisk
    override val viewModel: HomeViewModel by viewModel()
    val magiskRepo: MagiskRepository by inject()
    lateinit var EXT_FILE: File

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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        EXT_FILE = File("${requireActivity().filesDir.parent}/snet", "snet.apk")
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
                .map { it.byteStream().copyTo(EXT_FILE) }
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
        try {
            val loader = DexClassLoader(EXT_APK.path, EXT_APK.parent, null,
                    ISafetyNetHelper::class.java.classLoader)
            val clazz = loader.loadClass("com.topjohnwu.snet.Snet")
            val helper = clazz.getMethod("newHelper",
                    Class::class.java, String::class.java, Activity::class.java, Any::class.java)
                    .invoke(null, ISafetyNetHelper::class.java, EXT_APK.path,
                            requireActivity(), this) as ISafetyNetHelper
            if (helper.version < Const.SNET_EXT_VER)
                throw Exception()
            helper.attest()
        } catch (e: Exception) {
            if (dieOnError) {
                viewModel.finishSafetyNetCheck(-1)
                return
            }
            Shell.sh("rm -rf " + EXT_APK.parent).exec()
            EXT_APK.parentFile?.mkdir()
            downloadSafetyNet(!dieOnError)
        }
    }

    companion object {
        val EXT_APK = File("${App.self.filesDir.parent}/snet", "snet.apk")
    }
}

