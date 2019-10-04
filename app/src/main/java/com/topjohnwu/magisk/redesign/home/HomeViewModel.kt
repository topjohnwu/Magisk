package com.topjohnwu.magisk.redesign.home

import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.res
import com.topjohnwu.magisk.model.entity.MagiskJson
import com.topjohnwu.magisk.model.entity.ManagerJson
import com.topjohnwu.magisk.model.entity.UpdateInfo
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.ui.home.MagiskState

class HomeViewModel(
    private val repoMagisk: MagiskRepository
) : CompatViewModel() {

    val stateMagisk = KObservableField(MagiskState.LOADING)
    val stateManager = KObservableField(MagiskState.LOADING)
    val stateTextMagisk = Observer(stateMagisk) {
        when (stateMagisk.value) {
            MagiskState.NOT_INSTALLED -> R.string.magisk_version_error.res()
            MagiskState.UP_TO_DATE -> R.string.magisk_up_to_date.res()
            MagiskState.LOADING -> R.string.checking_for_updates.res()
            MagiskState.OBSOLETE -> R.string.magisk_update_title.res()
        }
    }
    val stateTextManager = Observer(stateManager) {
        when (stateManager.value) {
            MagiskState.NOT_INSTALLED -> R.string.invalid_update_channel.res()
            MagiskState.UP_TO_DATE -> R.string.manager_up_to_date.res()
            MagiskState.LOADING -> R.string.checking_for_updates.res()
            MagiskState.OBSOLETE -> R.string.manager_update_title.res()
        }
    }

    override fun refresh() = repoMagisk.fetchUpdate()
        .subscribeK { updateBy(it) }

    private fun updateBy(info: UpdateInfo) {
        stateMagisk.value = when {
            !info.magisk.isInstalled -> MagiskState.NOT_INSTALLED
            info.magisk.isObsolete -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        stateManager.value = when {
            !info.app.isUpdateChannelCorrect -> MagiskState.NOT_INSTALLED
            info.app.isObsolete -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }
    }

    fun onDeletePressed() {}

}

@Suppress("unused")
val MagiskJson.isInstalled
    get() = Info.magiskVersionCode > 0
val MagiskJson.isObsolete
    get() = Info.magiskVersionCode < versionCode
val ManagerJson.isUpdateChannelCorrect
    get() = versionCode > 0
val ManagerJson.isObsolete
    get() = BuildConfig.VERSION_CODE < versionCode