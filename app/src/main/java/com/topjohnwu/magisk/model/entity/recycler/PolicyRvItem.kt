package com.topjohnwu.magisk.model.entity.recycler

import android.graphics.drawable.Drawable
import androidx.databinding.Bindable
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.databinding.ObservableItem
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.utils.set

class PolicyItem(val item: MagiskPolicy, val icon: Drawable) : ObservableItem<PolicyItem>() {
    override val layoutRes = R.layout.item_policy_md2

    @get:Bindable
    var isExpanded = false
        set(value) = set(value, field, { field = it }, BR.expanded)

    @get:Bindable
    var isEnabled = item.policy == MagiskPolicy.ALLOW
        set(value) = set(value, field, { field = it }, BR.enabled)

    @get:Bindable
    var shouldNotify = item.notification
        set(value) = set(value, field, { field = it }, BR.shouldNotify)

    @get:Bindable
    var shouldLog = item.logging
        set(value) = set(value, field, { field = it }, BR.shouldLog)

    private val updatedPolicy
        get() = item.copy(
            policy = if (isEnabled) MagiskPolicy.ALLOW else MagiskPolicy.DENY,
            notification = shouldNotify,
            logging = shouldLog
        )

    fun toggle(viewModel: SuperuserViewModel) {
        if (isExpanded) {
            toggle()
            return
        }
        isEnabled = !isEnabled
        viewModel.togglePolicy(this, isEnabled)
    }

    fun toggle() {
        isExpanded = !isExpanded
    }

    fun toggleNotify(viewModel: SuperuserViewModel) {
        shouldNotify = !shouldNotify
        viewModel.updatePolicy(updatedPolicy, isLogging = false)
    }

    fun toggleLog(viewModel: SuperuserViewModel) {
        shouldLog = !shouldLog
        viewModel.updatePolicy(updatedPolicy, isLogging = true)
    }

    override fun onBindingBound(binding: ViewDataBinding) {
        super.onBindingBound(binding)
        val params = binding.root.layoutParams as? StaggeredGridLayoutManager.LayoutParams
        params?.isFullSpan = true
    }

    override fun contentSameAs(other: PolicyItem) = itemSameAs(other)
    override fun itemSameAs(other: PolicyItem) = item.uid == other.item.uid

}
