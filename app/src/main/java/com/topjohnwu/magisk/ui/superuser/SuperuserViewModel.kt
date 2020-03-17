package com.topjohnwu.magisk.ui.superuser

import android.content.pm.PackageManager
import android.content.res.Resources
import androidx.databinding.ObservableArrayList
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.magiskdb.PolicyDao
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.core.utils.BiometricHelper
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.applySchedulers
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.entity.recycler.PolicyItem
import com.topjohnwu.magisk.model.entity.recycler.TappableHeadlineItem
import com.topjohnwu.magisk.model.entity.recycler.TextItem
import com.topjohnwu.magisk.model.events.PolicyUpdateEvent
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.model.events.dialog.BiometricDialog
import com.topjohnwu.magisk.model.events.dialog.SuperuserRevokeDialog
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.base.adapterOf
import com.topjohnwu.magisk.ui.base.diffListOf
import com.topjohnwu.magisk.ui.base.itemBindingOf
import io.reactivex.Single
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
        .insertItem(TappableHeadlineItem.Safetynet)
        .insertList(itemsHelpers)
        .insertList(itemsPolicies)
    val itemBinding = itemBindingOf<ComparableRvItem<*>> {
        it.bindExtra(BR.viewModel, this)
        it.bindExtra(BR.listener, this)
    }

    // ---

    override fun refresh() = db.fetchAll()
        .flattenAsFlowable { it }
        .parallel()
        .map { PolicyItem(it, it.applicationInfo.loadIcon(packageManager)) }
        .sequential()
        .sorted { o1, o2 ->
            compareBy<PolicyItem>(
                { it.item.appName.toLowerCase(currentLocale) },
                { it.item.packageName }
            ).compare(o1, o2)
        }
        .toList()
        .map { it to itemsPolicies.calculateDiff(it) }
        .applySchedulers()
        .applyViewModel(this)
        .subscribeK {
            itemsPolicies.update(it.first, it.second)
            if (itemsPolicies.isNotEmpty()) {
                itemsHelpers.remove(itemNoData)
            }
        }

    // ---

    @Suppress("REDUNDANT_ELSE_IN_WHEN")
    override fun onItemPressed(item: TappableHeadlineItem) = when (item) {
        TappableHeadlineItem.Hide -> hidePressed()
        TappableHeadlineItem.Safetynet -> safetynetPressed()
        else -> Unit
    }

    private fun safetynetPressed() =
        SuperuserFragmentDirections.actionSuperuserFragmentToSafetynetFragment().publish()

    private fun hidePressed() =
        SuperuserFragmentDirections.actionSuperuserFragmentToHideFragment().publish()

    fun deletePressed(item: PolicyItem) {
        fun updateState() = deletePolicy(item.item)
            .subscribeK {
                itemsPolicies.removeAll { it.genericItemSameAs(item) }
                if (itemsPolicies.isEmpty() && itemsHelpers.isEmpty()) {
                    itemsHelpers.add(itemNoData)
                }
            }
            .add()

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

    fun updatePolicy(it: PolicyUpdateEvent) = when (it) {
        is PolicyUpdateEvent.Notification -> updatePolicy(it.item).map {
            when {
                it.notification -> R.string.su_snack_notif_on
                else -> R.string.su_snack_notif_off
            } to it.appName
        }
        is PolicyUpdateEvent.Log -> updatePolicy(it.item).map {
            when {
                it.logging -> R.string.su_snack_log_on
                else -> R.string.su_snack_log_off
            } to it.appName
        }
    }.map { resources.getString(it.first, it.second) }
        .subscribeK { SnackbarEvent(it).publish() }
        .add()

    fun togglePolicy(item: PolicyItem, enable: Boolean) {
        fun updateState() {
            val policy = if (enable) MagiskPolicy.ALLOW else MagiskPolicy.DENY
            val app = item.item.copy(policy = policy)

            updatePolicy(app)
                .map { it.policy == MagiskPolicy.ALLOW }
                .map { if (it) R.string.su_snack_grant else R.string.su_snack_deny }
                .map { resources.getString(it).format(item.item.appName) }
                .subscribeK { SnackbarEvent(it).publish() }
                .add()
        }

        if (BiometricHelper.isEnabled) {
            BiometricDialog {
                onSuccess { updateState() }
                onFailure { item.isEnabled.toggle() }
            }.publish()
        } else {
            updateState()
        }
    }

    //---

    private fun updatePolicy(policy: MagiskPolicy) =
        db.update(policy).andThen(Single.just(policy))

    private fun deletePolicy(policy: MagiskPolicy) =
        db.delete(policy.uid).andThen(Single.just(policy))

}
