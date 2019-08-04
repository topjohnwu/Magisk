package com.topjohnwu.magisk.ui.log

import android.content.res.Resources
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.extensions.doOnSubscribeUi
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.util.KObservableField
import com.skoumal.teanity.viewevents.SnackbarEvent
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.LogRepository
import com.topjohnwu.magisk.model.binding.BindingAdapter
import com.topjohnwu.magisk.model.entity.recycler.ConsoleRvItem
import com.topjohnwu.magisk.model.entity.recycler.LogItemRvItem
import com.topjohnwu.magisk.model.entity.recycler.LogRvItem
import com.topjohnwu.magisk.model.entity.recycler.MagiskLogRvItem
import com.topjohnwu.magisk.model.events.PageChangedEvent
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.superuser.Shell
import me.tatarka.bindingcollectionadapter2.BindingViewPagerAdapter
import me.tatarka.bindingcollectionadapter2.OnItemBind
import timber.log.Timber
import java.io.File
import java.util.*

class LogViewModel(
    private val resources: Resources,
    private val logRepo: LogRepository
) : MagiskViewModel(), BindingViewPagerAdapter.PageTitles<ComparableRvItem<*>> {

    val itemsAdapter = BindingAdapter()
    val items = DiffObservableList(ComparableRvItem.callback)
    val itemBinding = OnItemBind<ComparableRvItem<*>> { itemBinding, _, item ->
        item.bind(itemBinding)
        itemBinding.bindExtra(BR.viewModel, this@LogViewModel)
    }
    val currentPage = KObservableField(0)
    private val currentItem get() = items[currentPage.value]

    private val logItem get() = items[0] as LogRvItem
    private val magiskLogItem get() = items[1] as MagiskLogRvItem

    val scrollPosition = KObservableField(0)

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

    fun scrollDownPressed() {
        scrollPosition.value = magiskLogItem.items.size - 1
    }

    fun refresh() {
        fetchLogs().subscribeK { logItem.update(it) }
        fetchMagiskLog().subscribeK { magiskLogItem.update(it) }
    }

    fun saveLog() {
        val now = Calendar.getInstance()
        val filename = "magisk_log_%04d%02d%02d_%02d%02d%02d.log".format(
            now.get(Calendar.YEAR), now.get(Calendar.MONTH) + 1,
            now.get(Calendar.DAY_OF_MONTH), now.get(Calendar.HOUR_OF_DAY),
            now.get(Calendar.MINUTE), now.get(Calendar.SECOND)
        )

        val logFile = File(Config.downloadDirectory, filename)
        runCatching {
            logFile.createNewFile()
        }.onFailure {
            Timber.e(it)
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

    private fun clearLogs(callback: () -> Unit) = logRepo.clearLogs()
        .doOnSubscribeUi(callback)
        .subscribeK { SnackbarEvent(R.string.logs_cleared).publish() }
        .add()

    private fun clearMagiskLogs(callback: () -> Unit) = logRepo.clearMagiskLogs()
        .ignoreElement()
        .doOnComplete(callback)
        .subscribeK { SnackbarEvent(R.string.logs_cleared).publish() }
        .add()

    private fun fetchLogs() = logRepo.fetchLogs()
        .flattenAsFlowable { it }
        .map { LogItemRvItem(it) }
        .toList()

    private fun fetchMagiskLog() = logRepo.fetchMagiskLogs()
        .flattenAsFlowable { it }
        .map { ConsoleRvItem(it) }
        .toList()

}