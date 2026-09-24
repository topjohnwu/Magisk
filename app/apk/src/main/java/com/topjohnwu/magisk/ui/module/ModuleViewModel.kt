package com.topjohnwu.magisk.ui.module

import android.content.Context
import android.net.Uri
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.arch.AsyncLoadViewModel
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.Subject
import com.topjohnwu.magisk.core.model.module.LocalModule
import com.topjohnwu.magisk.core.model.module.OnlineModule
import com.topjohnwu.magisk.core.utils.TextHolder
import com.topjohnwu.magisk.core.utils.asText
import com.topjohnwu.magisk.ui.flash.FlashUtils
import com.topjohnwu.magisk.ui.navigation.Route
import com.topjohnwu.magisk.utils.asFlow
import com.topjohnwu.magisk.view.Notifications
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlinx.parcelize.Parcelize
import com.topjohnwu.magisk.core.R as CoreR

data class ModuleItem(
    val module: LocalModule,
    val isEnabled: Boolean = module.enable,
    val isRemoved: Boolean = module.remove,
    val showUpdate: Boolean = module.updateInfo != null,
) {
    val showNotice: Boolean
    val showAction: Boolean
    val noticeText: TextHolder
    val isUpdated = module.updated
    val updateReady get() = module.outdated && !isRemoved && isEnabled

    init {
        val isZygisk = module.isZygisk
        val isRiru = module.isRiru
        val zygiskUnloaded = isZygisk && module.zygiskUnloaded

        showNotice = zygiskUnloaded ||
            (Info.isZygiskEnabled && isRiru) ||
            (!Info.isZygiskEnabled && isZygisk)
        showAction = module.hasAction && !showNotice
        noticeText =
            when {
                zygiskUnloaded -> CoreR.string.zygisk_module_unloaded.asText()
                isRiru -> CoreR.string.suspend_text_riru.asText(CoreR.string.zygisk.asText())
                else -> CoreR.string.suspend_text_zygisk.asText(CoreR.string.zygisk.asText())
            }
    }
}

@Parcelize
class OnlineModuleSubject(
    override val module: OnlineModule,
    override val autoLaunch: Boolean,
    override val notifyId: Int = Notifications.nextId(),
) : Subject.Module() {
    override fun pendingIntent(context: Context) = FlashUtils.installIntent(context, file)
}

class ModuleViewModel : AsyncLoadViewModel() {

    data class UiState(
        val loading: Boolean = true,
        val modules: List<ModuleItem> = emptyList(),
    )

    private val _uiState = MutableStateFlow(UiState())
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    init {
        viewModelScope.launch {
            Info.isConnected.asFlow().collect {
                startLoading()
            }
        }
    }

    override suspend fun doLoadWork() {
        _uiState.update { it.copy(loading = true) }
        val moduleLoaded = Info.env.isActive &&
            withContext(Dispatchers.IO) { LocalModule.loaded() }
        if (moduleLoaded) {
            val modules = withContext(Dispatchers.Default) {
                LocalModule.installed().map { ModuleItem(it) }
            }
            _uiState.update { it.copy(loading = false, modules = modules) }
            loadUpdateInfo()
        } else {
            _uiState.update { it.copy(loading = false) }
        }
    }

    private suspend fun loadUpdateInfo() {
        withContext(Dispatchers.IO) {
            _uiState.update { state ->
                state.copy(
                    modules = state.modules.map { item ->
                        if (item.module.fetch()) {
                            item.copy(showUpdate = item.module.updateInfo != null)
                        } else {
                            item
                        }
                    }
                )
            }
        }
    }

    fun confirmLocalInstall(uri: Uri) {
        navigateTo(Route.Flash(Const.Value.FLASH_ZIP, uri.toString()))
    }

    fun runAction(id: String, name: String) {
        navigateTo(Route.Action(id, name))
    }

    fun toggleEnabled(item: ModuleItem) {
        val newEnabled = !item.isEnabled
        item.module.enable = newEnabled
        _uiState.update { state ->
            state.copy(
                modules = state.modules.map {
                    if (it.module.id == item.module.id) it.copy(isEnabled = newEnabled) else it
                }
            )
        }
    }

    fun toggleRemove(item: ModuleItem) {
        val newRemoved = !item.isRemoved
        item.module.remove = newRemoved
        _uiState.update { state ->
            state.copy(
                modules = state.modules.map {
                    if (it.module.id == item.module.id) it.copy(isRemoved = newRemoved) else it
                }
            )
        }
    }
}
