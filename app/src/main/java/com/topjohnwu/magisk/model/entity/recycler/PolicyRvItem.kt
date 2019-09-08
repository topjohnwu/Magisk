package com.topjohnwu.magisk.model.entity.recycler

import android.graphics.drawable.Drawable
import com.skoumal.teanity.databinding.ComparableRvItem
import com.skoumal.teanity.extensions.addOnPropertyChangedCallback
import com.skoumal.teanity.rxbus.RxBus
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.inject
import com.topjohnwu.magisk.extensions.toggle
import com.topjohnwu.magisk.model.entity.MagiskPolicy
import com.topjohnwu.magisk.model.events.PolicyEnableEvent
import com.topjohnwu.magisk.model.events.PolicyUpdateEvent

class PolicyRvItem(val item: MagiskPolicy, val icon: Drawable) : ComparableRvItem<PolicyRvItem>() {

    override val layoutRes: Int = R.layout.item_policy

    val isExpanded = KObservableField(false)
    val isEnabled = KObservableField(item.policy == MagiskPolicy.ALLOW)
    val shouldNotify = KObservableField(item.notification)
    val shouldLog = KObservableField(item.logging)

    fun toggle() = isExpanded.toggle()

    private val rxBus: RxBus by inject()

    private val currentStateItem
        get() = item.copy(
            policy = if (isEnabled.value) MagiskPolicy.ALLOW else MagiskPolicy.DENY,
            notification = shouldNotify.value,
            logging = shouldLog.value
        )

    init {
        isEnabled.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            rxBus.post(PolicyEnableEvent(this@PolicyRvItem, it))
        }
        shouldNotify.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            rxBus.post(PolicyUpdateEvent.Notification(currentStateItem))
        }
        shouldLog.addOnPropertyChangedCallback {
            it ?: return@addOnPropertyChangedCallback
            rxBus.post(PolicyUpdateEvent.Log(currentStateItem))
        }
    }

    override fun contentSameAs(other: PolicyRvItem): Boolean = itemSameAs(other)
    override fun itemSameAs(other: PolicyRvItem): Boolean = item.uid == other.item.uid
}