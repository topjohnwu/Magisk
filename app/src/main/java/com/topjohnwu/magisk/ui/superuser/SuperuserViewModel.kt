package com.topjohnwu.magisk.ui.superuser

import android.content.pm.PackageManager
import android.content.res.Resources
import androidx.databinding.ObservableArrayList
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.model.entity.recycler.PolicyItem
import com.topjohnwu.magisk.model.entity.recycler.TappableHeadlineItem
import com.topjohnwu.magisk.model.entity.recycler.TextItem
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.model.events.dialog.BiometricDialog
import com.topjohnwu.magisk.model.events.dialog.SuperuserRevokeDialog
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.base.adapterOf
import com.topjohnwu.magisk.ui.base.diffListOf
import com.topjohnwu.magisk.ui.base.itemBindingOf
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import me.tatarka.bindingcollectionadapter2.collections.MergeObservableList

class SuperuserViewModel(
    private val db: PolicyDao,
    private val packageManager: PackageManager,
    private val resources: Resources
) : BaseViewModel(), TappableHeadlineItem.Listener {

    private val itemNoData = TextItem(R.string.superuser_policy_none)

    private val itemsPolicies = diffListOf<PolicyItem>()
    private val itemsHelpers = ObservableArrayList<TextItem>().also {
        it.add(itemNoData)
    }

    val adapter = adapterOf<ComparableRvItem<*>>()
    val items = MergeObservableList<ComparableRvItem<*>>()
        .insertItem(TappableHeadlineItem.Hide)
        .insertList(itemsHelpers)
        .insertList(itemsPolicies)
    val itemBinding = itemBindingOf<ComparableRvItem<*>> {
        it.bindExtra(BR.viewModel, this)
        it.bindExtra(BR.listener, this)
    }

    // ---

    override fun refresh() = viewModelScope.launch {
        state = State.LOADING
        val (policies, diff) = withContext(Dispatchers.Default) {
            val policies = db.fetchAll {
                PolicyItem(it, it.applicationInfo.loadIcon(packageManager))
            }.sortedWith(compareBy(
                { it.item.appName.toLowerCase(currentLocale) },
                { it.item.packageName }
            ))
            policies to itemsPolicies.calculateDiff(policies)
        }
        itemsPolicies.update(policies, diff)
        if (itemsPolicies.isNotEmpty()) {
            itemsHelpers.remove(itemNoData)
        }
        state = State.LOADED
    }

    // ---

    @Suppress("REDUNDANT_ELSE_IN_WHEN")
    override fun onItemPressed(item: TappableHeadlineItem) = when (item) {
        TappableHeadlineItem.Hide -> hidePressed()
        else -> Unit
    }

    private fun hidePressed() =
        SuperuserFragmentDirections.actionSuperuserFragmentToHideFragment().publish()

    fun deletePressed(item: PolicyItem) {
        fun updateState() = viewModelScope.launch {
            db.delete(item.item.uid)
            itemsPolicies.removeAll { it.genericItemSameAs(item) }
            if (itemsPolicies.isEmpty() && itemsHelpers.isEmpty()) {
                itemsHelpers.add(itemNoData)
            }
        }

        if (BiometricHelper.isEnabled) {
            BiometricDialog {
                onSuccess { updateState() }
            }.publish()
        } else {
            SuperuserRevokeDialog {
                appName = item.item.appName
                onSuccess { updateState() }
            }.publish()
        }
    }

    //---

    fun updatePolicy(policy: MagiskPolicy, isLogging: Boolean) = viewModelScope.launch {
        db.update(policy)
        val str = when {
            isLogging -> when {
                policy.logging -> R.string.su_snack_log_on
                else -> R.string.su_snack_log_off
            }
            else -> when {
                policy.notification -> R.string.su_snack_notif_on
                else -> R.string.su_snack_notif_off
            }
        }
        SnackbarEvent(resources.getString(str, policy.appName)).publish()
    }

    fun togglePolicy(item: PolicyItem, enable: Boolean) {
        fun updateState() {
            val policy = if (enable) MagiskPolicy.ALLOW else MagiskPolicy.DENY
            val app = item.item.copy(policy = policy)

            viewModelScope.launch {
                db.update(app)
                val res = if (app.policy == MagiskPolicy.ALLOW) R.string.su_snack_grant
                else R.string.su_snack_deny
                SnackbarEvent(resources.getString(res).format(item.item.appName))
            }
        }

        if (BiometricHelper.isEnabled) {
            BiometricDialog {
                onSuccess { updateState() }
                onFailure { item.isEnabled = !item.isEnabled }
            }.publish()
        } else {
            updateState()
        }
    }
}
