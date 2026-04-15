package com.topjohnwu.magisk.ui.install

import android.net.Uri
import android.widget.Toast
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig.APP_VERSION_CODE
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.ui.navigation.Route
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File
import java.io.IOException
import com.topjohnwu.magisk.core.R as CoreR

class InstallViewModel(svc: NetworkService) : BaseViewModel() {

    enum class Method { NONE, PATCH, DIRECT, INACTIVE_SLOT, DOWNLOAD }

    data class UiState(
        val step: Int = 0,
        val method: Method = Method.NONE,
        val notes: String = "",
        val patchUri: Uri? = null,
        val requestFilePicker: Boolean = false,
        val showSecondSlotWarning: Boolean = false,
        val showDownloadDialog: Boolean = false,
    )

    val isRooted get() = Info.isRooted
    val skipOptions = Info.isEmulator || (Info.isSAR && !Info.isFDE && Info.ramdisk)
    val noSecondSlot = !isRooted || !Info.isAB || Info.isEmulator

    private val _uiState = MutableStateFlow(UiState(step = if (skipOptions) 1 else 0))
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

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
                withContext(Dispatchers.Main) {
                    _uiState.update { it.copy(notes = noteText) }
                }
            } catch (e: IOException) {
                Timber.e(e)
            }
        }
    }

    fun nextStep() {
        _uiState.update { it.copy(step = 1) }
    }

    fun selectMethod(method: Method) {
        _uiState.update { it.copy(method = method) }
        when (method) {
            Method.PATCH -> {
                AppContext.toast(CoreR.string.patch_file_msg, Toast.LENGTH_LONG)
                _uiState.update { it.copy(requestFilePicker = true) }
            }
            Method.INACTIVE_SLOT -> {
                _uiState.update { it.copy(showSecondSlotWarning = true) }
            }
            Method.DOWNLOAD -> {
                _uiState.update { it.copy(showDownloadDialog = true) }
            }
            else -> {}
        }
    }

    fun onFilePickerConsumed() {
        _uiState.update { it.copy(requestFilePicker = false) }
    }

    fun onSecondSlotWarningConsumed() {
        _uiState.update { it.copy(showSecondSlotWarning = false) }
    }

    fun onDownloadDialogConsumed() {
        _uiState.update { it.copy(showDownloadDialog = false) }
    }

    fun onPatchFileSelected(uri: Uri) {
        _uiState.update { it.copy(patchUri = uri) }
        if (_uiState.value.method == Method.PATCH) {
            install()
        }
    }

    fun onDownloadUrlSelected(uri: Uri) {
        _uiState.update { it.copy(patchUri = uri) }
        if (_uiState.value.method == Method.DOWNLOAD) {
            install()
        }
    }

    fun install() {
        when (_uiState.value.method) {
            Method.PATCH -> navigateTo(Route.Flash(
                action = Const.Value.PATCH_FILE,
                additionalData = _uiState.value.patchUri!!.toString()
            ))
            Method.DOWNLOAD -> navigateTo(Route.Flash(
                action = Const.Value.DOWNLOAD,
                additionalData = _uiState.value.patchUri!!.toString()
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
}
