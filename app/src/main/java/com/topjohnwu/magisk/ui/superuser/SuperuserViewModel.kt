package com.topjohnwu.magisk.ui.superuser

import android.content.pm.PackageManager
import android.content.res.Resources
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.extensions.applySchedulers
import com.skoumal.teanity.util.DiffObservableList
import com.skoumal.teanity.viewevents.SnackbarEvent
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.MagiskDB
import com.topjohnwu.magisk.model.entity.Policy
import com.topjohnwu.magisk.model.entity.recycler.PolicyRvItem
import com.topjohnwu.magisk.ui.base.MagiskViewModel
import com.topjohnwu.magisk.utils.FingerprintHelper
import com.topjohnwu.magisk.utils.toggle
import com.topjohnwu.magisk.view.dialogs.CustomAlertDialog
import com.topjohnwu.magisk.view.dialogs.FingerprintAuthDialog
import io.reactivex.Single
import me.tatarka.bindingcollectionadapter2.ItemBinding
import timber.log.Timber

class SuperuserViewModel(
    private val database: MagiskDB,
    private val packageManager: PackageManager,
    private val resources: Resources
) : MagiskViewModel() {

    val items = DiffObservableList(ComparableRvItem.callback)
    val itemBinding = ItemBinding.of<ComparableRvItem<*>> { itemBinding, _, item ->
        item.bind(itemBinding)
        itemBinding.bindExtra(BR.viewModel, this@SuperuserViewModel)
    }

    private var ignoreNext: PolicyRvItem? = null

    init {
        updatePolicies()
    }

    fun updatePolicies() {
        Single.fromCallable { database.policyList }
            .flattenAsFlowable { it }
            .map { PolicyRvItem(it, it.info.loadIcon(packageManager)).setListeners() }
            .toList()
            .applySchedulers()
            .applyViewModel(this)
            .subscribe({
                items.update(it)
            }, Timber::e)
            .add()
    }

    fun deletePressed(item: PolicyRvItem) {
        fun updateState() = deletePolicy(item.item)
            .subscribe({
                items.remove(item)
            }, Timber::e)
            .add()

        withView {
            if (FingerprintHelper.useFingerprint()) {
                FingerprintAuthDialog(this) { updateState() }.show()
            } else {
                CustomAlertDialog(this)
                    .setTitle(R.string.su_revoke_title)
                    .setMessage(getString(R.string.su_revoke_msg, item.item.appName))
                    .setPositiveButton(R.string.yes) { _, _ -> updateState() }
                    .setNegativeButton(R.string.no_thanks, null)
                    .setCancelable(true)
                    .show()
            }
        }
    }

    private fun PolicyRvItem.setListeners() = apply {
        isEnabled.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback

            if (ignoreNext == this) {
                ignoreNext = null
                return@addOnPropertyChangedCallback
            }

            fun updateState() {
                item.policy = if (it) Policy.ALLOW else Policy.DENY

                updatePolicy(item)
                    .map { it.policy == Policy.ALLOW }
                    .subscribe({
                        val textId = if (it) R.string.su_snack_grant else R.string.su_snack_deny
                        val text = resources.getString(textId).format(item.appName)
                        SnackbarEvent(text).publish()
                    }, Timber::e)
                    .add()
            }

            if (FingerprintHelper.useFingerprint()) {
                withView {
                    FingerprintAuthDialog(this, { updateState() }, {
                        ignoreNext = this@setListeners
                        isEnabled.toggle()
                    }).show()
                }
            } else {
                updateState()
            }
        }
        shouldNotify.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            item.notification = it

            updatePolicy(item)
                .map { it.notification }
                .subscribe({
                    val textId = if (it) R.string.su_snack_notif_on else R.string.su_snack_notif_off
                    val text = resources.getString(textId).format(item.appName)
                    SnackbarEvent(text).publish()
                }, Timber::e)
                .add()
        }
        shouldLog.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            item.logging = it

            updatePolicy(item)
                .map { it.logging }
                .subscribe({
                    val textId = if (it) R.string.su_snack_log_on else R.string.su_snack_log_off
                    val text = resources.getString(textId).format(item.appName)
                    SnackbarEvent(text).publish()
                }, Timber::e)
                .add()
        }
    }

    private fun updatePolicy(policy: Policy) =
        Single.fromCallable { database.updatePolicy(policy); policy }
            .applySchedulers()

    private fun deletePolicy(policy: Policy) =
        Single.fromCallable { database.deletePolicy(policy); policy }
            .applySchedulers()

}