package com.topjohnwu.magisk.ui.log

import android.content.res.Resources
import androidx.databinding.ObservableArrayList
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.extensions.doOnSuccessUi
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.util.KObservableField
import com.skoumal.teanity.viewevents.SnackbarEvent
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.MagiskDB
import com.topjohnwu.magisk.model.entity.recycler.*
import com.topjohnwu.magisk.model.events.PageChangedEvent
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.magisk.utils.toSingle
import com.topjohnwu.magisk.utils.zip
import com.topjohnwu.superuser.Shell
import io.reactivex.Single
import me.tatarka.bindingcollectionadapter2.BindingViewPagerAdapter
import me.tatarka.bindingcollectionadapter2.OnItemBind
import java.io.File
import java.io.IOException
import java.util.*

class LogViewModel(
    private val resources: Resources,
    private val database: MagiskDB
) : MagiskViewModel(), BindingViewPagerAdapter.PageTitles<ComparableRvItem<*>> {

    val items = DiffObservableList(ComparableRvItem.callback)
    val itemBinding = OnItemBind<ComparableRvItem<*>> { itemBinding, _, item ->
        item.bind(itemBinding)
        itemBinding.bindExtra(BR.viewModel, this@LogViewModel)
    }
    val currentPage = KObservableField(0)
    private val currentItem get() = items[currentPage.value]

    private val logItem get() = items[0] as LogRvItem
    private val magiskLogItem get() = items[1] as MagiskLogRvItem

    init {
        currentPage.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            PageChangedEvent().publish()
        }

        items.addAll(listOf(LogRvItem(), MagiskLogRvItem()))
        refresh()
    }

    override fun getPageTitle(position: Int, item: ComparableRvItem<*>?) = when (item) {
        is LogRvItem -> resources.getString(R.string.superuser)
        is MagiskLogRvItem -> resources.getString(R.string.magisk)
        else -> ""
    }

    fun refresh() = zip(updateLogs(), updateMagiskLog()) { _, _ -> true }
        .subscribeK()
        .add()

    fun saveLog() {
        val now = Calendar.getInstance()
        val filename = "magisk_log_%04d%02d%02d_%02d%02d%02d.log".format(
            now.get(Calendar.YEAR), now.get(Calendar.MONTH) + 1,
            now.get(Calendar.DAY_OF_MONTH), now.get(Calendar.HOUR_OF_DAY),
            now.get(Calendar.MINUTE), now.get(Calendar.SECOND)
        )

        val logFile = File(Const.EXTERNAL_PATH, filename)
        try {
            logFile.createNewFile()
        } catch (e: IOException) {
            return
        }

        Shell.su("cat ${Const.MAGISK_LOG} > $logFile").submit {
            SnackbarEvent(logFile.path).publish()
        }
    }

    fun clearLog() = when (currentItem) {
        is LogRvItem -> clearLogs { refresh() }
        is MagiskLogRvItem -> clearMagiskLogs { refresh() }
        else -> Unit
    }

    private fun clearLogs(callback: () -> Unit) {
        Single.fromCallable { database.clearLogs() }
            .subscribeK {
                SnackbarEvent(R.string.logs_cleared).publish()
                callback()
            }
            .add()
    }

    private fun clearMagiskLogs(callback: () -> Unit) {
        Shell.su("echo -n > " + Const.MAGISK_LOG).submit {
            SnackbarEvent(R.string.logs_cleared).publish()
            callback()
        }
    }

    private fun updateLogs() = Single.fromCallable { database.logs }
        .flattenAsFlowable { it }
        .map { it.map { LogItemEntryRvItem(it) } }
        .map { LogItemRvItem(ObservableArrayList<ComparableRvItem<*>>().apply { addAll(it) }) }
        .toList()
        .doOnSuccessUi { logItem.update(it) }

    private fun updateMagiskLog() = Shell.su("tail -n 5000 ${Const.MAGISK_LOG}").toSingle()
        .map { it.exec() }
        .map { it.out }
        .flattenAsFlowable { it }
        .map { ConsoleRvItem(it) }
        .toList()
        .doOnSuccessUi { magiskLogItem.update(it) }

}