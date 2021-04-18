package com.topjohnwu.magisk.ui.install

import android.app.Activity
import android.net.Uri
import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.events.MagiskInstallFileEvent
import com.topjohnwu.magisk.events.dialog.SecondSlotWarningDialog
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.launch
import timber.log.Timber
import java.io.File
import java.io.IOException

class InstallViewModel(
    svc: NetworkService
) : BaseViewModel() {

    val isRooted = Shell.rootAccess()
    val skipOptions = Info.isEmulator || (Info.ramdisk && !Info.isFDE && Info.isSAR)
    val noSecondSlot = !isRooted || Info.isPixel || Info.isVirtualAB || !Info.isAB || Info.isEmulator

    @get:Bindable
    var step = if (skipOptions) 1 else 0
        set(value) = set(value, field, { field = it }, BR.step)

    var _method = -1

    @get:Bindable
    var method
        get() = _method
        set(value) = set(value, _method, { _method = it }, BR.method) {
            when (it) {
                R.id.method_patch -> {
                    MagiskInstallFileEvent { code, intent ->
                        if (code == Activity.RESULT_OK)
                            data = intent?.data
                    }.publish()
                }
                R.id.method_inactive_slot -> {
                    SecondSlotWarningDialog().publish()
                }
            }
        }

    @get:Bindable
    var data: Uri? = null
        set(value) = set(value, field, { field = it }, BR.data)

    @get:Bindable
    var notes = ""
        set(value) = set(value, field, { field = it }, BR.notes)

    init {
        viewModelScope.launch {
            try {
                File(AppContext.cacheDir, "${BuildConfig.VERSION_CODE}.md").run {
                    notes = when {
                        exists() -> readText()
                        Const.Url.CHANGELOG_URL.isEmpty() -> ""
                        else -> {
                            val text = svc.fetchString(Const.Url.CHANGELOG_URL)
                            writeText(text)
                            text
                        }
                    }
                }
            } catch (e: IOException) {
                Timber.e(e)
            }
        }
    }

    fun step(nextStep: Int) {
        step = nextStep
    }

    fun install() {
        when (method) {
            R.id.method_patch -> FlashFragment.patch(data!!).navigate()
            R.id.method_direct -> FlashFragment.flash(false).navigate()
            R.id.method_inactive_slot -> FlashFragment.flash(true).navigate()
            else -> error("Unknown value")
        }
        state = State.LOADING
    }
}
