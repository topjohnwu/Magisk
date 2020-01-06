package com.topjohnwu.magisk.redesign.settings

import android.Manifest
import android.view.View
import android.widget.Toast
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.RepoDao
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.recycler.SettingsItem
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.RecreateEvent
import com.topjohnwu.magisk.model.events.dialog.BiometricDialog
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.compat.adapterOf
import com.topjohnwu.magisk.redesign.compat.diffListOf
import com.topjohnwu.magisk.redesign.compat.itemBindingOf
import com.topjohnwu.magisk.utils.PatchAPK
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell
import io.reactivex.Completable
import io.reactivex.subjects.PublishSubject
import org.koin.core.get

class SettingsViewModel(
    private val repositoryDao: RepoDao
) : CompatViewModel(), SettingsItem.Callback {

    val adapter = adapterOf<SettingsItem>()
    val itemBinding = itemBindingOf<SettingsItem> { it.bindExtra(BR.callback, this) }
    val items = diffListOf(
        Customization,
        Theme, Language, Redesign, DownloadPath, GridSize,

        Manager,
        UpdateChannel, UpdateChannelUrl, ClearRepoCache, HideOrRestore(), UpdateChecker,
        Biometrics, Reauthenticate,

        Magisk,
        SafeMode, MagiskHide, SystemlessHosts,

        Superuser,
        AccessMode, MultiuserMode, MountNamespaceMode, AutomaticResponse, RequestTimeout,
        SUNotification
    )

    override fun onItemPressed(view: View, item: SettingsItem) = when (item) {
        is DownloadPath -> requireRWPermission()
        else -> Unit
    }

    override fun onItemChanged(view: View, item: SettingsItem) = when (item) {
        // use only instances you want, don't declare everything
        is Theme -> Navigation.theme().publish()
        is Redesign -> DieEvent().publish()
        is Language -> RecreateEvent().publish()

        is UpdateChannel -> openUrlIfNecessary(view)
        is Biometrics -> authenticateOrRevert()
        is ClearRepoCache -> clearRepoCache()
        is SystemlessHosts -> createHosts()
        is Hide -> updateManager(hide = true)
        is Restore -> updateManager(hide = false)

        else -> Unit
    }

    private fun openUrlIfNecessary(view: View) {
        UpdateChannelUrl.refresh()
        if (UpdateChannelUrl.value.isBlank()) {
            UpdateChannelUrl.onPressed(view, this@SettingsViewModel)
        }
    }

    private fun authenticateOrRevert() {
        // immediately revert the preference
        Biometrics.value = !Biometrics.value
        BiometricDialog {
            // allow the change on success
            onSuccess { Biometrics.value = !Biometrics.value }
        }.publish()
    }

    private fun clearRepoCache() {
        Completable.fromAction { repositoryDao.clear() }
            .subscribeK { Utils.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT) }
    }

    private fun createHosts() {
        Shell.su("add_hosts_module").submit {
            Utils.toast(R.string.settings_hosts_toast, Toast.LENGTH_SHORT)
        }
    }

    private fun requireRWPermission() {
        val callback = PublishSubject.create<Boolean>()
        callback.subscribeK { if (!it) requireRWPermission() }
        PermissionEvent(
            listOf(
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            ), callback
        ).publish()
    }

    private fun updateManager(hide: Boolean) {
        if (hide) {
            PatchAPK.hideManager(get(), Hide.value)
        } else {
            DownloadService(get()) {
                subject = DownloadSubject.Manager(Configuration.APK.Restore)
            }
        }
    }

}