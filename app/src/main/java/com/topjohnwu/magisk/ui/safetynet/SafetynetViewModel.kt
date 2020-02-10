package com.topjohnwu.magisk.ui.safetynet

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.utils.SafetyNetHelper
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.events.SafetyNetResult
import com.topjohnwu.magisk.model.events.UpdateSafetyNetEvent
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.safetynet.SafetyNetState.*
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.RxBus

enum class SafetyNetState {
    LOADING, PASS, FAILED, IDLE
}

class SafetynetViewModel(
    rxBus: RxBus
) : BaseViewModel() {

    private var currentState = IDLE
        set(value) {
            field = value
            notifyStateChanged()
        }
    val safetyNetTitle = KObservableField(R.string.empty)
    val ctsState = KObservableField(false)
    val basicIntegrityState = KObservableField(false)

    val isChecking @Bindable get() = currentState == LOADING
    val isFailed @Bindable get() = currentState == FAILED
    val isSuccess @Bindable get() = currentState == PASS

    init {
        rxBus.register<SafetyNetResult>()
            .subscribeK { resolveResponse(it.responseCode) }
            .add()

        if (safetyNetResult >= 0) {
            resolveResponse(safetyNetResult)
        } else {
            attest()
        }
    }

    override fun notifyStateChanged() {
        super.notifyStateChanged()
        notifyPropertyChanged(BR.loading)
        notifyPropertyChanged(BR.failed)
        notifyPropertyChanged(BR.success)
    }

    private fun attest() {
        currentState = LOADING
        UpdateSafetyNetEvent().publish()
    }

    fun reset() = attest()

    private fun resolveResponse(response: Int) = when {
        response and 0x0F == 0 -> {
            val hasCtsPassed = response and SafetyNetHelper.CTS_PASS != 0
            val hasBasicIntegrityPassed = response and SafetyNetHelper.BASIC_PASS != 0
            val result = hasCtsPassed && hasBasicIntegrityPassed
            safetyNetResult = response
            ctsState.value = hasCtsPassed
            basicIntegrityState.value = hasBasicIntegrityPassed
            currentState = if (result) PASS else FAILED
            safetyNetTitle.value =
                if (result) R.string.safetynet_attest_success
                else R.string.safetynet_attest_failure
        }
        response == -2 -> {
            currentState = FAILED
            ctsState.value = false
            basicIntegrityState.value = false
            back()
        }
        else -> {
            currentState = FAILED
            ctsState.value = false
            basicIntegrityState.value = false
            safetyNetTitle.value = when (response) {
                SafetyNetHelper.RESPONSE_ERR -> R.string.safetynet_res_invalid
                else -> R.string.safetynet_api_error
            }
        }
    }

    companion object {
        private var safetyNetResult = -1
    }

}
