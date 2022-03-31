package com.topjohnwu.magisk.ui.superuser

import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import androidx.databinding.ObservableArrayList
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.databinding.AnyDiffRvItem
import com.topjohnwu.magisk.databinding.diffListOf
import com.topjohnwu.magisk.databinding.itemBindingOf
import com.topjohnwu.magisk.di.AppContext
import com.topjohnwu.magisk.events.SnackbarEvent
import com.topjohnwu.magisk.events.dialog.BiometricEvent
import com.topjohnwu.magisk.events.dialog.SuperuserRevokeDialog
import com.topjohnwu.magisk.ktx.getLabel
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.utils.asText
import com.topjohnwu.magisk.view.TextItem
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import me.tatarka.bindingcollectionadapter2.collections.MergeObservableList

class SuperuserViewModel(
    private val db: PolicyDao
) : BaseViewModel() {

    private val itemNoData = TextItem(R.string.superuser_policy_none)

    private val itemsPolicies = diffListOf<PolicyRvItem>()
    private val itemsHelpers = ObservableArrayList<TextItem>()

    val items = MergeObservableList<AnyDiffRvItem>()
        .insertList(itemsHelpers)
        .insertList(itemsPolicies)
    val itemBinding = itemBindingOf<AnyDiffRvItem> {
        it.bindExtra(BR.listener, this)
    }

    // ---

    @SuppressLint("InlinedApi")
    override fun refresh() = viewModelScope.launch {
        if (!Utils.showSuperUser()) {
            state = State.LOADING_FAILED
            return@launch
        }
        state = State.LOADING
        val (policies, diff) = withContext(Dispatchers.IO) {
            db.deleteOutdated()
            val policies = ArrayList<PolicyRvItem>()
            val pm = AppContext.packageManager
            for (policy in db.fetchAll()) {
                val pkgs = pm.getPackagesForUid(policy.uid) ?: continue
                policies.addAll(pkgs.mapNotNull { pkg ->
                    try {
                        val info = pm.getPackageInfo(pkg, MATCH_UNINSTALLED_PACKAGES)
                        PolicyRvItem(
                            this@SuperuserViewModel, policy,
                            info.packageName,
                            info.sharedUserId != null,
                            info.applicationInfo.loadIcon(pm),
                            info.applicationInfo.getLabel(pm)
                        )
                    } catch (e: PackageManager.NameNotFoundException) {
                        null
                    }
                })
            }
            policies.sortWith(compareBy(
                { it.appName.lowercase(currentLocale) },
                { it.packageName }
            ))
            policies to itemsPolicies.calculateDiff(policies)
        }
        itemsPolicies.update(policies, diff)
        if (itemsPolicies.isNotEmpty())
            itemsHelpers.clear()
        else if (itemsHelpers.isEmpty())
            itemsHelpers.add(itemNoData)
        state = State.LOADED
    }

    // ---

    fun deletePressed(item: PolicyRvItem) {
        fun updateState() = viewModelScope.launch {
            db.delete(item.item.uid)
            itemsPolicies.removeAll { it.itemSameAs(item) }
            if (itemsPolicies.isEmpty() && itemsHelpers.isEmpty()) {
                itemsHelpers.add(itemNoData)
            }
        }

        if (BiometricHelper.isEnabled) {
            BiometricEvent {
                onSuccess { updateState() }
            }.publish()
        } else {
            SuperuserRevokeDialog {
                appName = item.title
                onSuccess { updateState() }
            }.publish()
        }
    }

    fun updateNotify(item: PolicyRvItem) {
        viewModelScope.launch {
            db.update(item.item)
            val res = when {
                item.item.notification -> R.string.su_snack_notif_on
                else -> R.string.su_snack_notif_off
            }
            itemsPolicies.forEach {
                if (it.item.uid == item.item.uid) {
                    it.notifyPropertyChanged(BR.shouldNotify)
                }
            }
            SnackbarEvent(res.asText(item.appName)).publish()
        }
    }

    fun updateLogging(item: PolicyRvItem) {
        viewModelScope.launch {
            db.update(item.item)
            val res = when {
                item.item.logging -> R.string.su_snack_log_on
                else -> R.string.su_snack_log_off
            }
            itemsPolicies.forEach {
                if (it.item.uid == item.item.uid) {
                    it.notifyPropertyChanged(BR.shouldLog)
                }
            }
            SnackbarEvent(res.asText(item.appName)).publish()
        }
    }

    fun togglePolicy(item: PolicyRvItem, enable: Boolean) {
        fun updateState() {
            viewModelScope.launch {
                val res = if (enable) R.string.su_snack_grant else R.string.su_snack_deny
                item.item.policy = if (enable) SuPolicy.ALLOW else SuPolicy.DENY
                db.update(item.item)
                itemsPolicies.forEach {
                    if (it.item.uid == item.item.uid) {
                        it.notifyPropertyChanged(BR.enabled)
                    }
                }
                SnackbarEvent(res.asText(item.appName)).publish()
            }
        }

        if (BiometricHelper.isEnabled) {
            BiometricEvent {
                onSuccess { updateState() }
            }.publish()
        } else {
            updateState()
        }
    }
}
