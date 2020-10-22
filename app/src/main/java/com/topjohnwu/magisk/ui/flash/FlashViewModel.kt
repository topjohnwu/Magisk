package com.topjohnwu.magisk.ui.flash

import android.content.res.Resources
import android.net.Uri
import android.view.MenuItem
import androidx.databinding.Bindable
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.arch.diffListOf
import com.topjohnwu.magisk.arch.itemBindingOf
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.tasks.FlashZip
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.magisk.databinding.RvBindingAdapter
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.magisk.ktx.*
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.utils.set
import com.topjohnwu.magisk.view.Notifications
import com.topjohnwu.superuser.CallbackList
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class FlashViewModel(
    args: FlashFragmentArgs,
    private val resources: Resources
) : BaseViewModel() {

    @get:Bindable
    var showReboot = Shell.rootAccess()
        set(value) = set(value, field, { field = it }, BR.showReboot)

    @get:Bindable
    var behaviorText = resources.getString(R.string.flashing)
        set(value) = set(value, field, { field = it }, BR.behaviorText)

    val adapter = RvBindingAdapter<ConsoleItem>()
    val items = diffListOf<ConsoleItem>()
    val itemBinding = itemBindingOf<ConsoleItem>()

    private val logItems = mutableListOf<String>().synchronized()
    private val outItems = object : CallbackList<String>() {
        override fun onAddElement(e: String?) {
            e ?: return
            items.add(ConsoleItem(e))
            logItems.add(e)
        }
    }

    init {
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
                    showReboot = false
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
                    showReboot = false
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
        behaviorText = when {
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
        viewModelScope.launch {
            withContext(Dispatchers.IO) {
                val name = Const.MAGISK_INSTALL_LOG_FILENAME.format(now.toTime(timeFormatStandard))
                val file = MediaStoreUtils.getFile(name)
                file.uri.outputStream().bufferedWriter().use { writer ->
                    logItems.forEach {
                        writer.write(it)
                        writer.newLine()
                    }
                }
                SnackbarEvent(file.toString()).publish()
            }
        }
    }

    fun restartPressed() = reboot()
}
