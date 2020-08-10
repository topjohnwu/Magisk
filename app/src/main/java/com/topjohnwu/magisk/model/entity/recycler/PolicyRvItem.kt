package com.topjohnwu.magisk.model.entity.recycler

import android.graphics.drawable.Drawable
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.databinding.ObservableItem
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.utils.set

class PolicyItem(
    val item: MagiskPolicy,
    val icon: Drawable,
    val viewModel: SuperuserViewModel
) : ObservableItem<PolicyItem>() {
    override val layoutRes = R.layout.item_policy_md2

    @get:Bindable
    var isExpanded = false
        set(value) = set(value, field, { field = it }, BR.expanded)

    // This property hosts the policy state
    var policyState = item.policy == MagiskPolicy.ALLOW
        set(value) = set(value, field, { field = it }, BR.enabled)

    // This property binds with the UI state
    @get:Bindable
    var isEnabled
        get() = policyState
        set(value) = set(value, policyState, { viewModel.togglePolicy(this, it) }, BR.enabled)

    @get:Bindable
    var shouldNotify = item.notification
        set(value) = set(value, field, { field = it }, BR.shouldNotify) {
            viewModel.updatePolicy(updatedPolicy, isLogging = false)
        }

    @get:Bindable
    var shouldLog = item.logging
        set(value) = set(value, field, { field = it }, BR.shouldLog) {
            viewModel.updatePolicy(updatedPolicy, isLogging = true)
        }

    private val updatedPolicy
        get() = item.copy(
            policy = if (policyState) MagiskPolicy.ALLOW else MagiskPolicy.DENY,
            notification = shouldNotify,
            logging = shouldLog
        )

    fun toggleExpand() {
        isExpanded = !isExpanded
    }

    fun toggleNotify() {
        shouldNotify = !shouldNotify
    }

    fun toggleLog() {
        shouldLog = !shouldLog
    }

    fun revoke() {
        viewModel.deletePressed(this)
    }

    override fun contentSameAs(other: PolicyItem) = itemSameAs(other)
    override fun itemSameAs(other: PolicyItem) = item.uid == other.item.uid

}
