package com.topjohnwu.magisk.ui.install

import android.net.Uri
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.download.RemoteFileService
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.extensions.addOnPropertyChangedCallback
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.events.RequestFileEvent
import com.topjohnwu.magisk.model.events.dialog.SecondSlotWarningDialog
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.superuser.Shell
import org.koin.core.get
import kotlin.math.roundToInt

class InstallViewModel : BaseViewModel(State.LOADED) {

    val isRooted get() = Shell.rootAccess()
    val isAB get() = Info.isAB

    val step = KObservableField(0)
    val method = KObservableField(-1)

    val progress = KObservableField(0)

    var data = KObservableField<Uri?>(null)

    init {
        RemoteFileService.reset()
        RemoteFileService.progressBroadcast.observeForever {
            val (progress, subject) = it ?: return@observeForever
            if (subject !is DownloadSubject.Magisk) {
                return@observeForever
            }
            this.progress.value = progress.times(100).roundToInt()
            if (this.progress.value >= 100) {
                state = State.LOADED
            }
        }
        method.addOnPropertyChangedCallback {
            when (method.value) {
                R.id.method_patch -> {
                    Utils.toast(R.string.patch_file_msg, Toast.LENGTH_LONG)
                    RequestFileEvent().publish()
                }
                R.id.method_inactive_slot -> {
                    SecondSlotWarningDialog().publish()
                }
            }
        }
    }

    fun step(nextStep: Int) {
        step.value = nextStep
    }

    fun install() = DownloadService(get()) {
        subject = DownloadSubject.Magisk(resolveConfiguration())
    }.also { state = State.LOADING }

    // ---

    private fun resolveConfiguration() = when (method.value) {
        R.id.method_download -> Configuration.Download
        R.id.method_patch -> Configuration.Patch(data.value!!)
        R.id.method_direct -> Configuration.Flash.Primary
        R.id.method_inactive_slot -> Configuration.Flash.Secondary
        else -> throw IllegalArgumentException("Unknown value")
    }
}
