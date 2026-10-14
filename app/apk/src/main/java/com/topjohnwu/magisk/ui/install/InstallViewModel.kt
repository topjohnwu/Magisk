package com.topjohnwu.magisk.ui.install

import android.net.Uri
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.BuildConfig.APP_VERSION_CODE
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.repository.FirmwareCrawler
import com.topjohnwu.magisk.core.repository.NetworkService
import com.topjohnwu.magisk.core.utils.UsbBlockDevice
import com.topjohnwu.magisk.ui.navigation.Route
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import timber.log.Timber
import java.io.File
import java.io.IOException

class InstallViewModel(
    svc: NetworkService,
    crawler: FirmwareCrawler,
) : BaseViewModel() {

    enum class Method { NONE, PATCH, DIRECT, INACTIVE_SLOT }

    data class UiState(
        val method: Method = Method.NONE,
        val notes: String = "",
        val patchUri: Uri? = null,
        val usbDevice: String? = null,
        val showSecondSlotWarning: Boolean = false,
        val showPatchDialog: Boolean = false,
        val candidate: FirmwareCrawler.Candidate? = null,
    )

    val isRooted get() = Info.isRooted
    val isGooglebook get() = Info.isGooglebook
    val skipOptions = Info.isEmulator || (Info.isSAR && !Info.isFDE && Info.ramdisk)
    val noSecondSlot = !isRooted || !Info.isAB || Info.isEmulator

    private val _uiState = MutableStateFlow(UiState())
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    private var candidateJob: Job? = null

    init {
        candidateJob = viewModelScope.launch(Dispatchers.IO) {
            val candidate = crawler.crawl()
            _uiState.update { it.copy(candidate = candidate) }
        }
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
                _uiState.update { it.copy(notes = noteText) }
            } catch (e: IOException) {
                Timber.e(e)
            }
        }
    }

    fun selectMethod(method: Method) {
        _uiState.update { it.copy(method = method) }
        when (method) {
            Method.PATCH -> {
                val job = candidateJob
                if (job != null && job.isActive) {
                    viewModelScope.launch {
                        job.join()
                        _uiState.update { it.copy(showPatchDialog = true) }
                    }
                } else {
                    _uiState.update { it.copy(showPatchDialog = true) }
                }
            }
            Method.INACTIVE_SLOT -> {
                _uiState.update { it.copy(showSecondSlotWarning = true) }
            }
            else -> {}
        }
    }

    fun onSecondSlotWarningConsumed() {
        _uiState.update { it.copy(showSecondSlotWarning = false) }
    }

    fun onPatchDialogConsumed() {
        _uiState.update { it.copy(showPatchDialog = false) }
    }

    fun onPatchSelected(uri: Uri, usbDeviceName: String? = null) {
        if (usbDeviceName != null) {
            viewModelScope.launch {
                if (UsbBlockDevice.requestPermission(AppContext, usbDeviceName)) {
                    _uiState.update { it.copy(patchUri = uri, usbDevice = usbDeviceName) }
                    if (_uiState.value.method == Method.PATCH) {
                        install()
                    }
                }
            }
        } else {
            _uiState.update { it.copy(patchUri = uri, usbDevice = null) }
            if (_uiState.value.method == Method.PATCH) {
                install()
            }
        }
    }

    fun install() {
        when (_uiState.value.method) {
            Method.PATCH -> navigateTo(Route.Flash(
                action = Const.Value.PATCH_FILE,
                additionalData = _uiState.value.patchUri!!.toString(),
                usbDevice = _uiState.value.usbDevice
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
