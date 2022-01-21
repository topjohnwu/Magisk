package com.topjohnwu.magisk.ui.install

import android.net.Uri
import android.text.SpannableStringBuilder
import android.text.Spanned
import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.events.MagiskInstallFileEvent
import com.topjohnwu.magisk.events.dialog.SecondSlotWarningDialog
import com.topjohnwu.magisk.ui.flash.FlashFragment
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import timber.log.Timber
import java.io.File
import java.io.IOException

class InstallViewModel(
    svc: NetworkService
) : BaseViewModel() {

    val isRooted = Shell.rootAccess()
    val hideVbmeta = Info.vbmeta || Info.isSamsung || Info.isAB
    val skipOptions = Info.isEmulator || (Info.isSAR && !Info.isFDE && hideVbmeta && Info.ramdisk)
    val noSecondSlot = !isRooted || !Info.isAB || Info.isEmulator

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
                    MagiskInstallFileEvent { uri -> data = uri }.publish()
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
    var notes: Spanned = SpannableStringBuilder()
        set(value) = set(value, field, { field = it }, BR.notes)

    init {
        viewModelScope.launch(Dispatchers.IO) {
            try {
                val file = File(AppContext.cacheDir, "${BuildConfig.VERSION_CODE}.md")
                val text = when {
                    file.exists() -> file.readText()
                    Const.Url.CHANGELOG_URL.isEmpty() -> ""
                    else -> {
                        val str = svc.fetchString(Const.Url.CHANGELOG_URL)
                        file.writeText(str)
                        str
                    }
                }
                notes = ServiceLocator.markwon.toMarkdown(text)
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
            R.id.method_patch -> FlashFragment.patch(data!!).navigate(true)
            R.id.method_direct -> FlashFragment.flash(false).navigate(true)
            R.id.method_inactive_slot -> FlashFragment.flash(true).navigate(true)
            else -> error("Unknown value")
        }
        state = State.LOADING
    }
}
