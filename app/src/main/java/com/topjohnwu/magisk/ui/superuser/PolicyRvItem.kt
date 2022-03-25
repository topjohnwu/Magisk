package com.topjohnwu.magisk.ui.superuser

import android.graphics.drawable.Drawable
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.databinding.ObservableDiffRvItem
import com.topjohnwu.magisk.databinding.RvContainer
import com.topjohnwu.magisk.databinding.set

class PolicyRvItem(
    override val item: SuPolicy,
    val icon: Drawable,
    val viewModel: SuperuserViewModel
) : ObservableDiffRvItem<PolicyRvItem>(), RvContainer<SuPolicy> {
    override val layoutRes = R.layout.item_policy_md2

    @get:Bindable
    var isExpanded = false
        set(value) = set(value, field, { field = it }, BR.expanded)

    // This property binds with the UI state
    @get:Bindable
    var isEnabled
        get() = item.policy == SuPolicy.ALLOW
        set(value) {
            if (value != isEnabled)
                viewModel.togglePolicy(this, value)
        }

    @get:Bindable
    var shouldNotify
        get() = item.notification
        set(value) = set(value, shouldNotify, { item.notification = it }, BR.shouldNotify) {
            viewModel.updatePolicy(item, isLogging = false)
        }

    @get:Bindable
    var shouldLog
        get() = item.logging
        set(value) = set(value, shouldLog, { item.logging = it }, BR.shouldLog) {
            viewModel.updatePolicy(item, isLogging = true)
        }

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

    override fun itemSameAs(other: PolicyRvItem) = item.uid == other.item.uid

}
