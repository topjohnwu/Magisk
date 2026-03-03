package com.topjohnwu.magisk.ui.install

import android.net.Uri
import android.widget.Toast
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig.APP_VERSION_CODE
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.ContentResultCallback
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.dialog.SecondSlotWarningDialog
import com.topjohnwu.magisk.events.GetContentEvent
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.ui.navigation.Route
import io.noties.markwon.Markwon
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlinx.parcelize.Parcelize
import timber.log.Timber
import java.io.File
import java.io.IOException
import com.topjohnwu.magisk.core.R as CoreR

class InstallViewModel(svc: NetworkService, markwon: Markwon) : BaseViewModel() {

    enum class Method { NONE, PATCH, DIRECT, INACTIVE_SLOT }

    data class UiState(
        val step: Int = 0,
        val method: Method = Method.NONE,
        val notes: String = "",
        val patchUri: Uri? = null,
    )

    val isRooted get() = Info.isRooted
    val skipOptions = Info.isEmulator || (Info.isSAR && !Info.isFDE && Info.ramdisk)
    val noSecondSlot = !isRooted || !Info.isAB || Info.isEmulator

    private val _uiState = MutableStateFlow(UiState(step = if (skipOptions) 1 else 0))
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    val data: LiveData<Uri?> get() = uri

    init {
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
                    _uiState.update { it.copy(notes = spanned.toString()) }
                }
            } catch (e: IOException) {
                Timber.e(e)
            }
        }

        uri.observeForever { newUri ->
            _uiState.update { it.copy(patchUri = newUri) }
        }
    }

    fun nextStep() {
        _uiState.update { it.copy(step = 1) }
    }

    fun selectMethod(method: Method) {
        _uiState.update { it.copy(method = method) }
        when (method) {
            Method.PATCH -> {
                GetContentEvent("*/*", UriCallback()).publish()
            }
            Method.INACTIVE_SLOT -> {
                SecondSlotWarningDialog().show()
            }
            else -> {}
        }
    }

    fun install() {
        when (_uiState.value.method) {
            Method.PATCH -> navigateTo(Route.Flash(
                action = Const.Value.PATCH_FILE,
                additionalData = data.value!!.toString()
            ))
            Method.DIRECT -> navigateTo(Route.Flash(
                action = Const.Value.FLASH_MAGISK
            ))
            Method.INACTIVE_SLOT -> navigateTo(Route.Flash(
                action = Const.Value.FLASH_INACTIVE_SLOT
            ))
            else -> error("Unknown method")
        }
    }

    val canInstall: Boolean
        get() {
            val state = _uiState.value
            return when (state.method) {
                Method.PATCH -> state.patchUri != null
                Method.DIRECT, Method.INACTIVE_SLOT -> true
                Method.NONE -> false
            }
        }

    @Parcelize
    class UriCallback : ContentResultCallback {
        override fun onActivityLaunch() {
            AppContext.toast(CoreR.string.patch_file_msg, Toast.LENGTH_LONG)
        }

        override fun onActivityResult(result: Uri) {
            uri.value = result
        }
    }

    companion object {
        private val uri = MutableLiveData<Uri?>()
    }
}
