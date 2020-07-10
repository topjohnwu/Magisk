package com.topjohnwu.magisk.ui.flash

import android.content.res.Resources
import android.net.Uri
import android.view.MenuItem
import androidx.databinding.ObservableArrayList
import androidx.databinding.ObservableField
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.tasks.FlashZip
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.magisk.extensions.*
import com.topjohnwu.magisk.model.binding.BindingAdapter
import com.topjohnwu.magisk.model.entity.recycler.ConsoleItem
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.base.diffListOf
import com.topjohnwu.magisk.ui.base.itemBindingOf
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import java.util.*

class FlashViewModel(
    args: FlashFragmentArgs,
    private val resources: Resources
) : BaseViewModel() {

    val showReboot = ObservableField(Shell.rootAccess())
    val behaviorText = ObservableField(resources.getString(R.string.flashing))

    val adapter = BindingAdapter<ConsoleItem>()
    val items = diffListOf<ConsoleItem>()
    val itemBinding = itemBindingOf<ConsoleItem>()

    private val outItems = ObservableArrayList<String>()
    private val logItems = Collections.synchronizedList(mutableListOf<String>())

    init {
        outItems.sendUpdatesTo(items, viewModelScope) { it.map { ConsoleItem(it) } }
        outItems.copyNewInputInto(logItems)

        args.dismissId.takeIf { it != -1 }?.also {
            Notifications.mgr.cancel(it)
        }
        val (installer, action, uri) = args
        startFlashing(installer, uri, action)
    }

    private fun startFlashing(installer: Uri, uri: Uri?, action: String) {
        viewModelScope.launch {
            val result = when (action) {
                Const.Value.FLASH_ZIP -> {
                    FlashZip(installer, outItems, logItems).exec()
                }
                Const.Value.UNINSTALL -> {
                    showReboot.value = false
                    FlashZip.Uninstall(installer, outItems, logItems).exec()
                }
                Const.Value.FLASH_MAGISK -> {
                    MagiskInstaller.Direct(installer, outItems, logItems).exec()
                }
                Const.Value.FLASH_INACTIVE_SLOT -> {
                    MagiskInstaller.SecondSlot(installer, outItems, logItems).exec()
                }
                Const.Value.PATCH_FILE -> {
                    uri ?: return@launch
                    showReboot.value = false
                    MagiskInstaller.Patch(installer, uri, outItems, logItems).exec()
                }
                else -> {
                    back()
                    return@launch
                }
            }
            onResult(result)
        }
    }

    private fun onResult(success: Boolean) {
        state = if (success) State.LOADED else State.LOADING_FAILED
        behaviorText.value = when {
            success -> resources.getString(R.string.done)
            else -> resources.getString(R.string.failure)
        }
    }

    fun onMenuItemClicked(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_save -> savePressed()
        }
        return true
    }

    private fun savePressed() = withExternalRW {
        if (!it)
            return@withExternalRW
        viewModelScope.launch {
            val name = Const.MAGISK_INSTALL_LOG_FILENAME.format(now.toTime(timeFormatStandard))
            val file = File(Config.downloadDirectory, name)
            withContext(Dispatchers.IO) {
                file.bufferedWriter().use { writer ->
                    logItems.forEach {
                        writer.write(it)
                        writer.newLine()
                    }
                }
            }
            SnackbarEvent(file.path).publish()
        }
    }

    fun restartPressed() = reboot()
}
