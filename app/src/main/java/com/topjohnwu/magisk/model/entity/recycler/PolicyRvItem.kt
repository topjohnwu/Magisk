package com.topjohnwu.magisk.model.entity.recycler

import android.graphics.drawable.Drawable
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.rxbus.RxBus
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.entity.Policy
import com.topjohnwu.magisk.model.events.PolicyEnableEvent
import com.topjohnwu.magisk.model.events.PolicyUpdateEvent
import com.topjohnwu.magisk.utils.inject
import com.topjohnwu.magisk.utils.toggle

class PolicyRvItem(val item: Policy, val icon: Drawable) : ComparableRvItem<PolicyRvItem>() {

    override val layoutRes: Int = R.layout.item_policy

    val isExpanded = KObservableField(false)
    val isEnabled = KObservableField(item.policy == Policy.ALLOW)
    val shouldNotify = KObservableField(item.notification)
    val shouldLog = KObservableField(item.logging)

    fun toggle() = isExpanded.toggle()

    private val rxBus: RxBus by inject()

    init {
        isEnabled.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            rxBus.post(PolicyEnableEvent(this@PolicyRvItem, it))
        }
        shouldNotify.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            item.notification = it
            rxBus.post(PolicyUpdateEvent.Notification(this@PolicyRvItem))
        }
        shouldLog.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            item.logging = it
            rxBus.post(PolicyUpdateEvent.Log(this@PolicyRvItem))
        }
    }

    override fun contentSameAs(other: PolicyRvItem): Boolean = itemSameAs(other)
    override fun itemSameAs(other: PolicyRvItem): Boolean = item.uid == other.item.uid
}