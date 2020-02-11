package com.topjohnwu.magisk.model.entity.recycler

import android.graphics.drawable.Drawable
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.model.MagiskPolicy
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.events.PolicyUpdateEvent
import com.topjohnwu.magisk.ui.superuser.SuperuserViewModel
import com.topjohnwu.magisk.utils.KObservableField

class PolicyItem(val item: MagiskPolicy, val icon: Drawable) : ComparableRvItem<PolicyItem>() {
    override val layoutRes = R.layout.item_policy_md2

    val isExpanded = KObservableField(false)
    val isEnabled = KObservableField(item.policy == MagiskPolicy.ALLOW)
    val shouldNotify = KObservableField(item.notification)
    val shouldLog = KObservableField(item.logging)

    private val updatedPolicy
        get() = item.copy(
            policy = if (isEnabled.value) MagiskPolicy.ALLOW else MagiskPolicy.DENY,
            notification = shouldNotify.value,
            logging = shouldLog.value
        )

    fun toggle(viewModel: SuperuserViewModel) {
        if (isExpanded.value) {
            toggle()
            return
        }
        isEnabled.toggle()
        viewModel.togglePolicy(this, isEnabled.value)
    }

    fun toggle() {
        isExpanded.toggle()
    }

    fun toggleNotify(viewModel: SuperuserViewModel) {
        shouldNotify.toggle()
        viewModel.updatePolicy(PolicyUpdateEvent.Notification(updatedPolicy))
    }

    fun toggleLog(viewModel: SuperuserViewModel) {
        shouldLog.toggle()
        viewModel.updatePolicy(PolicyUpdateEvent.Log(updatedPolicy))
    }

    override fun onBindingBound(binding: ViewDataBinding) {
        super.onBindingBound(binding)
        val params = binding.root.layoutParams as? StaggeredGridLayoutManager.LayoutParams
        params?.isFullSpan = true
    }

    override fun contentSameAs(other: PolicyItem) = itemSameAs(other)
    override fun itemSameAs(other: PolicyItem) = item.uid == other.item.uid

}
