package com.topjohnwu.magisk.ui.settings

import android.Manifest
import android.os.Build
import android.view.View
import android.widget.Toast
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.core.utils.PatchAPK
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.data.database.RepoDao
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.recycler.SettingsItem
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.RecreateEvent
import com.topjohnwu.magisk.model.events.dialog.BiometricDialog
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.base.adapterOf
import com.topjohnwu.magisk.ui.base.diffListOf
import com.topjohnwu.magisk.ui.base.itemBindingOf
import com.topjohnwu.superuser.Shell
import io.reactivex.Completable
import io.reactivex.subjects.PublishSubject
import org.koin.core.get

class SettingsViewModel(
    private val repositoryDao: RepoDao
) : BaseViewModel(), SettingsItem.Callback {

    val adapter = adapterOf<SettingsItem>()
    val itemBinding = itemBindingOf<SettingsItem> { it.bindExtra(BR.callback, this) }
    val items = diffListOf(createItems())

    private fun createItems(): List<SettingsItem> {
        // Customization
        val list = mutableListOf(
            Customization,
            Theme, Language, GridSize
        )
        if (Build.VERSION.SDK_INT < 21) {
            // Pre 5.0 does not support getting colors from attributes,
            // making theming a pain in the ass. Just forget about it
            list.remove(Theme)
        }

        // Manager
        list.addAll(listOf(
            Manager,
            UpdateChannel, UpdateChannelUrl, UpdateChecker, DownloadPath
        ))
        if (Info.env.isActive) {
            list.add(ClearRepoCache)
            if (Const.USER_ID == 0 && Info.isConnected.value)
                list.add(HideOrRestore())
        }

        // Magisk
        if (Info.env.isActive) {
            list.addAll(listOf(
                Magisk,
                MagiskHide, SystemlessHosts, SafeMode
            ))
        }

        // Superuser
        if (Utils.showSuperUser()) {
            list.addAll(listOf(
                Superuser,
                Biometrics, AccessMode, MultiuserMode, MountNamespaceMode,
                AutomaticResponse, RequestTimeout, SUNotification
            ))
            if (Build.VERSION.SDK_INT < 23) {
                // Biometric is only available on 6.0+
                list.remove(Biometrics)
            }
            if (Build.VERSION.SDK_INT < 26) {
                // Re-authenticate is not feasible on 8.0+
                list.add(Reauthenticate)
            }
        }

        return list
    }

    override fun onItemPressed(view: View, item: SettingsItem) = when (item) {
        is DownloadPath -> requireRWPermission()
        else -> Unit
    }

    override fun onItemChanged(view: View, item: SettingsItem) = when (item) {
        // use only instances you want, don't declare everything
        is Theme -> SettingsFragmentDirections.actionSettingsFragmentToThemeFragment().publish()
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
        if (UpdateChannelUrl.isEnabled && UpdateChannelUrl.value.isBlank()) {
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
