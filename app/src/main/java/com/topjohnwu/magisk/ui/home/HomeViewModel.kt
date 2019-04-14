package com.topjohnwu.magisk.ui.home

import android.content.res.Resources
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.tasks.CheckUpdates
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.magisk.utils.Event
import com.topjohnwu.magisk.utils.toggle
import com.topjohnwu.net.Networking
import com.topjohnwu.superuser.Shell


class HomeViewModel(
    private val resources: Resources,
    private val app: App
) : MagiskViewModel() {

    val isAdvancedExpanded = KObservableField(false)

    val isForceEncryption = KObservableField(Config.keepEnc)
    val isKeepVerity = KObservableField(Config.keepVerity)

    private val prefsObserver = Observer(isForceEncryption, isKeepVerity) {
        Config.keepEnc = isForceEncryption.value
        Config.keepVerity = isKeepVerity.value
    }

    val magiskState = KObservableField(MagiskState.LOADING)
    val magiskStateText = Observer(magiskState) {
        when (magiskState.value) {
            MagiskState.NO_ROOT -> TODO()
            MagiskState.NOT_INSTALLED -> resources.getString(R.string.magisk_version_error)
            MagiskState.UP_TO_DATE -> resources.getString(R.string.magisk_up_to_date)
            MagiskState.LOADING -> resources.getString(R.string.checking_for_updates)
            MagiskState.OBSOLETE -> resources.getString(R.string.magisk_update_title)
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
        when (managerState.value) {
            MagiskState.NO_ROOT -> "wtf"
            MagiskState.NOT_INSTALLED -> resources.getString(R.string.invalid_update_channel)
            MagiskState.UP_TO_DATE -> resources.getString(R.string.manager_up_to_date)
            MagiskState.LOADING -> resources.getString(R.string.checking_for_updates)
            MagiskState.OBSOLETE -> resources.getString(R.string.manager_update_title)
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

    private var shownDialog = false
    private val current = resources.getString(R.string.current_installed)
    private val latest = resources.getString(R.string.latest_version)

    init {
        Event.register(this)
        refresh()
    }

    override fun onEvent(event: Int) {
        updateSelf()
        ensureEnv()
    }

    override fun getListeningEvents(): IntArray = intArrayOf(Event.UPDATE_CHECK_DONE)

    fun paypalPressed() = OpenLinkEvent(Const.Url.PAYPAL_URL).publish()
    fun patreonPressed() = OpenLinkEvent(Const.Url.PATREON_URL).publish()
    fun twitterPressed() = OpenLinkEvent(Const.Url.TWITTER_URL).publish()
    fun githubPressed() = OpenLinkEvent(Const.Url.REPO_URL).publish()
    fun xdaPressed() = OpenLinkEvent(Const.Url.XDA_THREAD).publish()
    fun uninstallPressed() = UninstallEvent().publish()

    fun advancedPressed() = isAdvancedExpanded.toggle()

    fun installPressed(item: MagiskItem) = when (item) {
        MagiskItem.MANAGER -> ManagerInstallEvent().publish()
        MagiskItem.MAGISK -> MagiskInstallEvent().publish()
    }

    fun cardPressed(item: MagiskItem) = when (item) {
        MagiskItem.MANAGER -> ManagerChangelogEvent().publish()
        MagiskItem.MAGISK -> MagiskChangelogEvent().publish()
    }

    fun refresh() {
        shownDialog = false
        state = State.LOADING
        magiskState.value = MagiskState.LOADING
        managerState.value = MagiskState.LOADING
        Event.reset(this)
        Config.remoteMagiskVersionString = null
        Config.remoteMagiskVersionCode = -1

        if (Networking.checkNetworkStatus(app)) {
            CheckUpdates.check()
        } else {
            state = State.LOADING_FAILED
        }
    }

    private fun updateSelf() {
        state = State.LOADED
        magiskState.value = when (Config.magiskVersionCode) {
            in Int.MIN_VALUE until 0 -> MagiskState.NOT_INSTALLED
            !in Config.remoteMagiskVersionCode..Int.MAX_VALUE -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        magiskCurrentVersion.value = version
            .format(Config.magiskVersionString, Config.magiskVersionCode)
            .let { current.format(it) }
        magiskLatestVersion.value = version
            .format(Config.remoteMagiskVersionString, Config.remoteMagiskVersionCode)
            .let { latest.format(it) }

        managerState.value = when (Config.remoteManagerVersionCode) {
            in Int.MIN_VALUE until 0 -> MagiskState.NOT_INSTALLED //wrong update channel
            in (BuildConfig.VERSION_CODE + 1)..Int.MAX_VALUE -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        managerCurrentVersion.value = version
            .format(BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE)
            .let { current.format(it) }
        managerLatestVersion.value = version
            .format(Config.remoteManagerVersionString, Config.remoteManagerVersionCode)
            .let { latest.format(it) }
    }

    private fun ensureEnv() {
        val invalidStates = listOf(MagiskState.NOT_INSTALLED, MagiskState.NO_ROOT, MagiskState.LOADING)

        // Don't bother checking env when magisk is not installed, loading or already has been shown
        if (invalidStates.any { it == magiskState.value } || shownDialog) return

        if (!Shell.su("env_check").exec().isSuccess) {
            shownDialog = true
            EnvFixEvent().publish()
        }
    }

    companion object {
        private const val version = "%s (%d)"
    }

}
