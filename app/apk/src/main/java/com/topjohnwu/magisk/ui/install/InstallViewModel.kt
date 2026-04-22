package com.topjohnwu.magisk.ui.install

import android.net.Uri
import android.os.Bundle
import android.os.Parcelable
import android.text.Spanned
import android.text.SpannedString
import android.widget.Toast
import androidx.databinding.Bindable
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.Observer
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig.APP_VERSION_CODE
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.ContentResultCallback
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.dialog.DownloadDialog
import com.topjohnwu.magisk.dialog.SecondSlotWarningDialog
import com.topjohnwu.magisk.events.GetContentEvent
import com.topjohnwu.magisk.ui.flash.FlashFragment
import io.noties.markwon.Markwon
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlinx.parcelize.Parcelize
import timber.log.Timber
import java.io.File
import java.io.IOException
import com.topjohnwu.magisk.core.R as CoreR

class InstallViewModel(svc: NetworkService, markwon: Markwon) : BaseViewModel() {

    val isRooted get() = Info.isRooted
    val skipOptions = Info.isEmulator || (Info.isSAR && !Info.isFDE && Info.ramdisk)
    val noSecondSlot = !isRooted || !Info.isAB || Info.isEmulator

    @get:Bindable
    var step = if (skipOptions) 1 else 0
        set(value) = set(value, field, { field = it }, BR.step)

    private var methodId = -1
    // RadioGroup fires its OnCheckedChangeListener with the previously-checked id
    // right before firing it with -1 when we clear the selection programmatically.
    // Track the id we expect to see in that spurious callback so the setter's
    // side-effect lambda can ignore it.
    private var spuriousMethodId = -1

    @get:Bindable
    var method
        get() = methodId
        set(value) = set(value, methodId, { methodId = it }, BR.method) {
            if (it == spuriousMethodId) {
                spuriousMethodId = -1
                return@set
            }
            when (it) {
                R.id.method_patch -> {
                    GetContentEvent("*/*", UriCallback()).publish()
                }
                R.id.method_download -> {
                    DownloadDialog(
                        callback = { url -> _uri.value = url },
                        onCancel = { resetMethod() },
                    ).show()
                }
                R.id.method_inactive_slot -> {
                    SecondSlotWarningDialog().show()
                }
            }
        }

    private fun resetMethod() {
        spuriousMethodId = methodId
        method = -1
    }

    private val _uri = MutableLiveData<Uri?>()
    val data: LiveData<Uri?> get() = _uri

    // UriCallback is @Parcelize'd and may outlive any single ViewModel instance
    // (it's restored from saved state after process death), so we route its
    // results through the static `patchSource` and observe it here. This keeps
    // the cross-instance coupling behind a single LiveData instead of a raw
    // companion-object lambda.
    private val sourceObserver = Observer<PatchSource?> { source ->
        when (source) {
            is PatchSource.File -> _uri.value = source.uri
            PatchSource.Cancelled -> resetMethod()
            null -> return@Observer
        }
        // Consume the result so a later VM instance doesn't replay a stale
        // selection/cancel from a previous session.
        patchSource.value = null
    }

    @get:Bindable
    var notes: Spanned = SpannedString("")
        set(value) = set(value, field, { field = it }, BR.notes)

    init {
        patchSource.observeForever(sourceObserver)
        viewModelScope.launch(Dispatchers.IO) {
            try {
                val noteFile = File(AppContext.cacheDir, "${APP_VERSION_CODE}.md")
                val noteText = when {
                    noteFile.exists() -> noteFile.readText()
                    else -> {
                        val note = svc.fetchUpdate(APP_VERSION_CODE)?.note.orEmpty()
                        if (note.isEmpty()) return@launch
                        noteFile.writeText(note)
                        note
                    }
                }
                val spanned = markwon.toMarkdown(noteText)
                withContext(Dispatchers.Main) {
                    notes = spanned
                }
            } catch (e: IOException) {
                Timber.e(e)
            }
        }
    }

    fun install() {
        when (method) {
            R.id.method_patch -> FlashFragment.patch(data.value!!).navigate(true)
            R.id.method_download -> FlashFragment.download(data.value!!).navigate(true)
            R.id.method_direct -> FlashFragment.flash(false).navigate(true)
            R.id.method_inactive_slot -> FlashFragment.flash(true).navigate(true)
            else -> error("Unknown value")
        }
    }

    override fun onSaveState(state: Bundle) {
        state.putParcelable(
            INSTALL_STATE_KEY, InstallState(
                methodId,
                step,
                Config.keepVerity,
                Config.keepEnc,
                Config.recovery
            )
        )
    }

    override fun onRestoreState(state: Bundle) {
        state.getParcelable<InstallState>(INSTALL_STATE_KEY)?.let {
            methodId = it.method
            step = it.step
            Config.keepVerity = it.keepVerity
            Config.keepEnc = it.keepEnc
            Config.recovery = it.recovery
        }
    }

    override fun onCleared() {
        patchSource.removeObserver(sourceObserver)
        super.onCleared()
    }

    private sealed interface PatchSource {
        data class File(val uri: Uri) : PatchSource
        data object Cancelled : PatchSource
    }

    @Parcelize
    class UriCallback : ContentResultCallback {
        override fun onActivityLaunch() {
            AppContext.toast(CoreR.string.patch_file_msg, Toast.LENGTH_LONG)
        }

        override fun onActivityResult(result: Uri) {
            patchSource.value = PatchSource.File(result)
        }

        override fun onActivityCancel() {
            patchSource.value = PatchSource.Cancelled
        }
    }

    @Parcelize
    class InstallState(
        val method: Int,
        val step: Int,
        val keepVerity: Boolean,
        val keepEnc: Boolean,
        val recovery: Boolean,
    ) : Parcelable

    companion object {
        private const val INSTALL_STATE_KEY = "install_state"
        private val patchSource = MutableLiveData<PatchSource?>()
    }
}
