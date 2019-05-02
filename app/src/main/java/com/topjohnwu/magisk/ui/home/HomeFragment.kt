package com.topjohnwu.magisk.ui.home

import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentMagiskBinding
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.ui.base.MagiskFragment
import com.topjohnwu.magisk.utils.ISafetyNetHelper
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.magisk.view.SafetyNet
import com.topjohnwu.magisk.view.SafetyNet.EXT_APK
import com.topjohnwu.magisk.view.dialogs.*
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell
import org.koin.androidx.viewmodel.ext.android.viewModel

class HomeFragment : MagiskFragment<HomeViewModel, FragmentMagiskBinding>(),
    ISafetyNetHelper.Callback {

    override val layoutRes: Int = R.layout.fragment_magisk
    override val viewModel: HomeViewModel by viewModel()

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
        if (Config.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
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
        fun download() = Networking
            .get(Const.Url.SNET_URL)
            .getAsFile(EXT_APK) { updateSafetyNet(true) }

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
            SafetyNet.dyRun(requireActivity(), this)
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
}

