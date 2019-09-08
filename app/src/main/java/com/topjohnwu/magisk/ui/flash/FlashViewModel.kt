package com.topjohnwu.magisk.ui.flash

import android.Manifest.permission.READ_EXTERNAL_STORAGE
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.content.res.Resources
import android.net.Uri
import android.os.Handler
import androidx.core.os.postDelayed
import androidx.databinding.ObservableArrayList
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.util.KObservableField
import com.skoumal.teanity.viewevents.SnackbarEvent
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.*
import com.topjohnwu.magisk.model.entity.recycler.ConsoleRvItem
import com.topjohnwu.magisk.model.flash.FlashResultListener
import com.topjohnwu.magisk.model.flash.Flashing
import com.topjohnwu.magisk.model.flash.Patching
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.superuser.Shell
import me.tatarka.bindingcollectionadapter2.ItemBinding
import java.io.File
import java.util.*

class FlashViewModel(
    action: String,
    installer: Uri,
    uri: Uri,
    private val resources: Resources
) : MagiskViewModel(), FlashResultListener {

    val canShowReboot = Shell.rootAccess()
    val showRestartTitle = KObservableField(false)

    val behaviorText = KObservableField(resources.getString(R.string.flashing))

    val items = DiffObservableList(ComparableRvItem.callback)
    val itemBinding = ItemBinding.of<ComparableRvItem<*>> { itemBinding, _, item ->
        item.bind(itemBinding)
        itemBinding.bindExtra(BR.viewModel, this@FlashViewModel)
    }

    private val outItems = ObservableArrayList<String>()
    private val logItems = Collections.synchronizedList(mutableListOf<String>())

    init {
        outItems.sendUpdatesTo(items) { it.map { ConsoleRvItem(it) } }
        outItems.copyNewInputInto(logItems)

        state = State.LOADING

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
                .File(installer, uri, outItems, logItems, this)
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

    fun savePressed() = withPermissions(READ_EXTERNAL_STORAGE, WRITE_EXTERNAL_STORAGE)
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