package com.topjohnwu.magisk.ui.home

import android.content.res.Resources
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.magisk.utils.toggle


class HomeViewModel(
    private val resources: Resources,
    private val app: App
) : MagiskViewModel() {

    val isAdvancedExpanded = KObservableField(false)

    val isForceEncryption = KObservableField(false /*todo*/)
    val isKeepVerity = KObservableField(false /*todo*/)

    private val prefsObserver = Observer(isForceEncryption, isKeepVerity) {
        Config.keepEnc = isForceEncryption.value
        Config.keepVerity = isKeepVerity.value
    }

    val magiskState = KObservableField(MagiskState.LOADING)
    val magiskStateText = Observer(magiskState) {
        @Suppress("WhenWithOnlyElse")
        when (magiskState.value) {
            MagiskState.NO_ROOT -> TODO()
            MagiskState.NOT_INSTALLED -> TODO()
            MagiskState.UP_TO_DATE -> TODO()
            MagiskState.LOADING -> TODO()
            MagiskState.OBSOLETE -> TODO()
        }
    }
    val magiskCurrentVersion = KObservableField("")
    val magiskLatestVersion = KObservableField("")
    val magiskAdditionalInfo = Observer(magiskState) {
        if (Config.get<Boolean>(Config.Key.COREONLY))
            resources.getString(R.string.core_only_enabled)
        else
            ""
    }

    val managerState = KObservableField(MagiskState.LOADING)
    val managerStateText = Observer(managerState) {
        @Suppress("WhenWithOnlyElse")
        when (managerState.value) {
            MagiskState.NO_ROOT -> TODO()
            MagiskState.NOT_INSTALLED -> TODO()
            MagiskState.UP_TO_DATE -> TODO()
            MagiskState.LOADING -> TODO()
            MagiskState.OBSOLETE -> TODO()
        }
    }
    val managerCurrentVersion = KObservableField("")
    val managerLatestVersion = KObservableField("")
    val managerAdditionalInfo = Observer(managerState) {
        if (app.packageName != BuildConfig.APPLICATION_ID)
            "(${app.packageName})"
        else
            ""
    }

    fun paypalPressed() = OpenLinkEvent(Const.Url.PAYPAL_URL).publish()
    fun patreonPressed() = OpenLinkEvent(Const.Url.PATREON_URL).publish()
    fun twitterPressed() = OpenLinkEvent(Const.Url.TWITTER_URL).publish()
    fun githubPressed() = OpenLinkEvent(Const.Url.REPO_URL).publish()
    fun xdaPressed() = OpenLinkEvent(Const.Url.XDA_THREAD).publish()
    fun uninstallPressed() = UninstallEvent.publish()

    fun refresh() {}

    fun advancedPressed() = isAdvancedExpanded.toggle()

    fun installPressed(item: MagiskItem) = when (item) {
        MagiskItem.MANAGER -> ManagerInstallEvent.publish()
        MagiskItem.MAGISK -> MagiskInstallEvent.publish()
    }

    fun cardPressed(item: MagiskItem) = when (item) {
        MagiskItem.MANAGER -> ManagerChangelogEvent.publish()
        MagiskItem.MAGISK -> MagiskChangelogEvent.publish()
    }

}
