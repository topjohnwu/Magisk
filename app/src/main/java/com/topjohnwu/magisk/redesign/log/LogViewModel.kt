package com.topjohnwu.magisk.redesign.log

import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.data.repository.LogRepository
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.recycler.LogItem
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf

class LogViewModel(
    private val repo: LogRepository
) : CompatViewModel() {

    val items = diffListOf<LogItem>()
    val itemBinding = itemBindingOf<LogItem> {
        it.bindExtra(BR.viewModel, this)
    }

    override fun refresh() = repo.fetchLogsNowrap()
        .map { it.map { LogItem(it) } }
        .map { it to items.calculateDiff(it) }
        .subscribeK {
            items.firstOrNull()?.isTop = false
            items.lastOrNull()?.isBottom = false

            items.update(it.first, it.second)

            items.firstOrNull()?.isTop = true
            items.lastOrNull()?.isBottom = true
        }

}