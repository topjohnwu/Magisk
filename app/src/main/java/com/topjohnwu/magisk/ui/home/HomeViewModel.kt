package com.topjohnwu.magisk.ui.home

import android.content.pm.PackageManager
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.base.viewmodel.BaseViewModel
import com.topjohnwu.magisk.data.repository.MagiskRepository
import com.topjohnwu.magisk.extensions.*
import com.topjohnwu.magisk.model.events.*
import com.topjohnwu.magisk.model.observer.Observer
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.SafetyNetHelper
import com.topjohnwu.superuser.Shell
import io.reactivex.Completable

enum class SafetyNetState {
    LOADING, PASS, FAILED, IDLE
}

enum class MagiskState {
    NOT_INSTALLED, UP_TO_DATE, OBSOLETE, LOADING
}

enum class MagiskItem {
    MANAGER, MAGISK
}

class HomeViewModel(
    private val magiskRepo: MagiskRepository
) : BaseViewModel(State.LOADED) {

    val hasGMS = runCatching {
        get<PackageManager>().getPackageInfo("com.google.android.gms", 0); true
    }.getOrElse { false }

    val isAdvancedExpanded = KObservableField(false)

    val isForceEncryption = KObservableField(Info.keepEnc)
    val isKeepVerity = KObservableField(Info.keepVerity)
    val isRecovery = KObservableField(Info.recovery)

    val magiskState = KObservableField(MagiskState.LOADING)
    val magiskStateText = Observer(magiskState) {
        when (magiskState.value) {
            MagiskState.NOT_INSTALLED -> R.string.magisk_version_error.res()
            MagiskState.UP_TO_DATE -> R.string.magisk_up_to_date.res()
            MagiskState.LOADING -> R.string.checking_for_updates.res()
            MagiskState.OBSOLETE -> R.string.magisk_update_title.res()
        }
    }
    val magiskCurrentVersion = KObservableField("")
    val magiskLatestVersion = KObservableField("")
    val magiskAdditionalInfo = Observer(magiskState) {
        if (Config.coreOnly)
            R.string.core_only_enabled.res()
        else
            ""
    }

    private val _managerState = KObservableField(MagiskState.LOADING)
    val managerState = Observer(_managerState, isConnected) {
        if (isConnected.value) _managerState.value else MagiskState.UP_TO_DATE
    }
    val managerStateText = Observer(managerState) {
        when (managerState.value) {
            MagiskState.NOT_INSTALLED -> R.string.invalid_update_channel.res()
            MagiskState.UP_TO_DATE -> R.string.manager_up_to_date.res()
            MagiskState.LOADING -> R.string.checking_for_updates.res()
            MagiskState.OBSOLETE -> R.string.manager_update_title.res()
        }
    }
    val managerCurrentVersion = KObservableField("")
    val managerLatestVersion = KObservableField("")
    val managerAdditionalInfo = Observer(managerState) {
        if (packageName != BuildConfig.APPLICATION_ID)
            "($packageName)"
        else
            ""
    }

    val safetyNetTitle = KObservableField(R.string.safetyNet_check_text.res())
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

    val isActive = KObservableField(false)

    private var shownDialog = false

    init {
        isForceEncryption.addOnPropertyChangedCallback {
            Info.keepEnc = it ?: return@addOnPropertyChangedCallback
        }
        isKeepVerity.addOnPropertyChangedCallback {
            Info.keepVerity = it ?: return@addOnPropertyChangedCallback
        }
        isRecovery.addOnPropertyChangedCallback {
            Info.recovery = it ?: return@addOnPropertyChangedCallback
        }
        isConnected.addOnPropertyChangedCallback {
            if (it == true) refresh(false)
        }

        refresh(false)
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
        safetyNetTitle.value = R.string.checking_safetyNet_status.res()

        UpdateSafetyNetEvent().publish()
    }

    fun finishSafetyNetCheck(response: Int) = when {
        response and 0x0F == 0 -> {
            val hasCtsPassed = response and SafetyNetHelper.CTS_PASS != 0
            val hasBasicIntegrityPassed = response and SafetyNetHelper.BASIC_PASS != 0
            safetyNetTitle.value = R.string.safetyNet_check_success.res()
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
            safetyNetTitle.value = when (response) {
                SafetyNetHelper.RESPONSE_ERR -> R.string.safetyNet_res_invalid.res()
                else -> R.string.safetyNet_api_error.res()
            }
        }
    }

    @JvmOverloads
    fun refresh(invalidate: Boolean = true) {
        if (invalidate)
            Info.envRef.invalidate()

        isActive.value = Info.env.isActive

        val fetchUpdate = if (isConnected.value)
            magiskRepo.fetchUpdate().ignoreElement()
        else
            Completable.complete()

        Completable.fromAction {
            // Ensure value is ready
            Info.env
        }.andThen(fetchUpdate)
        .applyViewModel(this)
        .doOnSubscribeUi {
            magiskState.value = MagiskState.LOADING
            _managerState.value = MagiskState.LOADING
            ctsState.value = SafetyNetState.IDLE
            basicIntegrityState.value = SafetyNetState.IDLE
            safetyNetTitle.value = R.string.safetyNet_check_text.res()
        }.subscribeK {
            updateSelf()
            ensureEnv()
            refreshVersions()
        }
    }

    private fun refreshVersions() {
        magiskCurrentVersion.value = if (magiskState.value != MagiskState.NOT_INSTALLED) {
            VERSION_FMT.format(Info.env.magiskVersionString, Info.env.magiskVersionCode)
        } else {
            ""
        }

        managerCurrentVersion.value = if (isRunningAsStub) MGR_VER_FMT
            .format(BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE, Info.stub!!.version)
        else
            VERSION_FMT.format(BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE)
    }

    private fun updateSelf() {
        magiskState.value = when (Info.env.magiskVersionCode) {
            in Int.MIN_VALUE .. 0 -> MagiskState.NOT_INSTALLED
            in 1 until Info.remote.magisk.versionCode -> MagiskState.OBSOLETE
            else -> MagiskState.UP_TO_DATE
        }

        magiskLatestVersion.value =
            VERSION_FMT.format(Info.remote.magisk.version, Info.remote.magisk.versionCode)

        _managerState.value = when (Info.remote.app.versionCode) {
            in Int.MIN_VALUE .. 0 -> MagiskState.NOT_INSTALLED //wrong update channel
            in (BuildConfig.VERSION_CODE + 1) .. Int.MAX_VALUE -> MagiskState.OBSOLETE
            else -> {
                if (Info.stub?.version ?: Int.MAX_VALUE < Info.remote.stub.versionCode)
                    MagiskState.OBSOLETE
                else
                    MagiskState.UP_TO_DATE
            }
        }

        managerLatestVersion.value = MGR_VER_FMT
            .format(Info.remote.app.version, Info.remote.app.versionCode, Info.remote.stub.versionCode)
    }

    private fun ensureEnv() {
        val invalidStates =
            listOf(MagiskState.NOT_INSTALLED, MagiskState.LOADING)

        // Don't bother checking env when magisk is not installed, loading or already has been shown
        if (invalidStates.any { it == magiskState.value } || shownDialog) return

        if (!Shell.su("env_check").exec().isSuccess) {
            shownDialog = true
            EnvFixEvent().publish()
        }
    }

    companion object {
        private const val VERSION_FMT = "%s (%d)"
        private const val MGR_VER_FMT = "%s (%d) (%d)"
    }

}
