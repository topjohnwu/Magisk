package com.topjohnwu.magisk.legacy.flash

import android.Manifest.permission.READ_EXTERNAL_STORAGE
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.content.Intent
import android.content.res.Resources
import android.net.Uri
import android.os.Handler
import android.view.MenuItem
import androidx.core.os.postDelayed
import androidx.databinding.ObservableArrayList
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.extensions.*
import com.topjohnwu.magisk.model.binding.BindingAdapter
import com.topjohnwu.magisk.model.entity.recycler.ConsoleItem
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.model.flash.FlashResultListener
import com.topjohnwu.magisk.model.flash.Flashing
import com.topjohnwu.magisk.model.flash.Patching
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.base.diffListOf
import com.topjohnwu.magisk.ui.base.itemBindingOf
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.superuser.Shell
import java.io.File
import java.util.*

class FlashViewModel(
    private val resources: Resources
) : BaseViewModel(), FlashResultListener {

    val canShowReboot = Shell.rootAccess()
    val showRestartTitle = KObservableField(false)

    val behaviorText = KObservableField(resources.getString(R.string.flashing))

    val adapter = BindingAdapter<ConsoleItem>()
    val items = diffListOf<ConsoleItem>()
    val itemBinding = itemBindingOf<ConsoleItem>()

    private val outItems = ObservableArrayList<String>()
    private val logItems = Collections.synchronizedList(mutableListOf<String>())

    init {
        outItems.sendUpdatesTo(items) { it.map { ConsoleItem(it) } }
        outItems.copyNewInputInto(logItems)
    }

    fun startFlashing(intent: Intent) {
        val installer = intent.data ?: return
        val uri: Uri? = intent.getParcelableExtra(Const.Key.FLASH_DATA)
        val action = intent.getStringExtra(Const.Key.FLASH_ACTION) ?: return

        when (action) {
            Const.Value.FLASH_ZIP -> Flashing
                .Install(installer, outItems, logItems, this)
                .exec()
            Const.Value.UNINSTALL -> Flashing
                .Uninstall(installer, outItems, logItems, this)
                .exec()
            Const.Value.FLASH_MAGISK -> Patching
                .Direct(installer, outItems, logItems, this)
                .exec()
            Const.Value.FLASH_INACTIVE_SLOT -> Patching
                .SecondSlot(installer, outItems, logItems, this)
                .exec()
            Const.Value.PATCH_FILE -> Patching
                .File(installer, uri ?: return, outItems, logItems, this)
                .exec()
        }
    }

    override fun onResult(isSuccess: Boolean) {
        state = if (isSuccess) State.LOADED else State.LOADING_FAILED
        behaviorText.value = when {
            isSuccess -> resources.getString(R.string.done)
            else -> resources.getString(R.string.failure)
        }

        if (isSuccess) {
            Handler().postDelayed(500) {
                showRestartTitle.value = true
            }
        }
    }

    fun onMenuItemClicked(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_save -> savePressed()
        }
        return true
    }

    private fun savePressed() = withPermissions(READ_EXTERNAL_STORAGE, WRITE_EXTERNAL_STORAGE)
        .map { now }
        .map { it.toTime(timeFormatStandard) }
        .map { Const.MAGISK_INSTALL_LOG_FILENAME.format(it) }
        .map { File(Config.downloadDirectory, it) }
        .map { file ->
            file.bufferedWriter().use { writer ->
                logItems.forEach {
                    writer.write(it)
                    writer.newLine()
                }
            }
            file.path
        }
        .subscribeK { SnackbarEvent(it).publish() }
        .add()

    fun restartPressed() = reboot()

    fun backPressed() = back()

}
