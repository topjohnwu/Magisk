package com.topjohnwu.magisk.ui.home

import android.content.res.Resources
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.extensions.doOnSubscribeUi
import com.skoumal.teanity.extensions.subscribeK
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.magisk.utils.ISafetyNetHelper
import com.topjohnwu.magisk.utils.toggle
import com.topjohnwu.superuser.Shell


class HomeViewModel(
    private val resources: Resources,
    private val app: App,
    private val magiskRepo: MagiskRepository
) : MagiskViewModel() {

    val isAdvancedExpanded = KObservableField(false)

    val isForceEncryption = KObservableField(Config.keepEnc)
    val isKeepVerity = KObservableField(Config.keepVerity)

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

    val safetyNetTitle = KObservableField(resources.getString(R.string.safetyNet_check_text))
    val ctsState = KObservableField(SafetyNetState.IDLE)
    val basicIntegrityState = KObservableField(SafetyNetState.IDLE)
    val safetyNetState = Observer(ctsState, basicIntegrityState) {
        val cts = ctsState.value
        val basic = basicIntegrityState.value
        val states = listOf(cts, basic)

        when {
            states.any { it == SafetyNetState.LOADING } -> State.LOADING
            states.any { it == SafetyNetState.IDLE } -> State.LOADING
            else -> State.LOADED
        }
    }

    val hasRoot = KObservableField(false)

    private var shownDialog = false
    private val current = resources.getString(R.string.current_installed)
    private val latest = resources.getString(R.string.latest_version)

    init {
        isForceEncryption.addOnPropertyChangedCallback {
            Config.keepEnc = it ?: return@addOnPropertyChangedCallback
        }
        isKeepVerity.addOnPropertyChangedCallback {
            Config.keepVerity = it ?: return@addOnPropertyChangedCallback
        }

        refresh()
    }

    fun paypalPressed() = OpenLinkEvent(Const.Url.PAYPAL_URL).publish()
    fun patreonPressed() = OpenLinkEvent(Const.Url.PATREON_URL).publish()
    fun twitterPressed() = OpenLinkEvent(Const.Url.TWITTER_URL).publish()
    fun githubPressed() = OpenLinkEvent(Const.Url.SOURCE_CODE_URL).publish()
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

    fun safetyNetPressed() {
        ctsState.value = SafetyNetState.LOADING
        basicIntegrityState.value = SafetyNetState.LOADING
        safetyNetTitle.value = resources.getString(R.string.checking_safetyNet_status)

        UpdateSafetyNetEvent().publish()
    }

    fun finishSafetyNetCheck(response: Int) = when {
        response and 0x0F == 0 -> {
            val hasCtsPassed = response and ISafetyNetHelper.CTS_PASS != 0
            val hasBasicIntegrityPassed = response and ISafetyNetHelper.BASIC_PASS != 0
            safetyNetTitle.value = resources.getString(R.string.safetyNet_check_success)
            ctsState.value = if (hasCtsPassed) {
                SafetyNetState.PASS
            } else {
                SafetyNetState.FAILED
            }
            basicIntegrityState.value = if (hasBasicIntegrityPassed) {
                SafetyNetState.PASS
            } else {
                SafetyNetState.FAILED
            }
        }
        response == -2 -> {
            ctsState.value = SafetyNetState.IDLE
            basicIntegrityState.value = SafetyNetState.IDLE
        }
        else -> {
            ctsState.value = SafetyNetState.IDLE
            basicIntegrityState.value = SafetyNetState.IDLE
            val errorString = when (response) {
                ISafetyNetHelper.RESPONSE_ERR -> R.string.safetyNet_res_invalid
                else -> R.string.safetyNet_api_error
            }
            safetyNetTitle.value = resources.getString(errorString)
        }
    }

    fun refresh() {
        magiskRepo.fetchConfig()
            .applyViewModel(this)
            .doOnSubscribeUi {
                magiskState.value = MagiskState.LOADING
                managerState.value = MagiskState.LOADING
            }
            .subscribeK {
                it.app.let {
                    Config.remoteManagerVersionCode = it.versionCode.toIntOrNull() ?: -1
                    Config.remoteManagerVersionString = it.version
                }
                it.magisk.let {
                    Config.remoteMagiskVersionCode = it.versionCode.toIntOrNull() ?: -1
                    Config.remoteMagiskVersionString = it.version
                }
                updateSelf()
                ensureEnv()
            }

        hasRoot.value = Shell.rootAccess()
    }

    private fun updateSelf() {
        state = State.LOADED
        magiskState.value = when (Config.magiskVersionCode) {
            in Int.MIN_VALUE until 0 -> MagiskState.NOT_INSTALLED
            !in Config.remoteMagiskVersionCode..Int.MAX_VALUE -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        if (magiskState.value != MagiskState.NOT_INSTALLED) {
            magiskCurrentVersion.value = version
                .format(Config.magiskVersionString, Config.magiskVersionCode)
                .let { current.format(it) }
        } else {
            magiskCurrentVersion.value = ""
        }
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
        val invalidStates =
            listOf(MagiskState.NOT_INSTALLED, MagiskState.NO_ROOT, MagiskState.LOADING)

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
