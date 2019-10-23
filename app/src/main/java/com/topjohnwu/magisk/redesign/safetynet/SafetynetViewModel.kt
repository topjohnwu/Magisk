package com.topjohnwu.magisk.redesign.safetynet

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.events.SafetyNetResult
import com.topjohnwu.magisk.model.events.UpdateSafetyNetEvent
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.ui.home.SafetyNetState.*
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.RxBus
import com.topjohnwu.magisk.utils.SafetyNetHelper

class SafetynetViewModel(
    rxBus: RxBus
) : CompatViewModel() {

    val safetyNetTitle = KObservableField(R.string.empty)
    val ctsState = KObservableField(IDLE)
    val basicIntegrityState = KObservableField(IDLE)

    init {
        rxBus.register<SafetyNetResult>()
            .subscribeK { resolveResponse(it.responseCode) }
            .add()
    }

    fun attest() = UpdateSafetyNetEvent().publish()

    private fun resolveResponse(response: Int) = when {
        //todo animate (reveal) to result (green/error)
        response and 0x0F == 0 -> {
            val hasCtsPassed = response and SafetyNetHelper.CTS_PASS != 0
            val hasBasicIntegrityPassed = response and SafetyNetHelper.BASIC_PASS != 0
            safetyNetTitle.value = R.string.safetyNet_check_success
            ctsState.value = if (hasCtsPassed) {
                PASS
            } else {
                FAILED
            }
            basicIntegrityState.value = if (hasBasicIntegrityPassed) {
                PASS
            } else {
                FAILED
            }
        }
        //todo animate (collapse) back to initial (fade error)
        response == -2 -> {
            ctsState.value = IDLE
            basicIntegrityState.value = IDLE
        }
        //todo animate (collapse) back to initial (surface)
        else -> {
            ctsState.value = IDLE
            basicIntegrityState.value = IDLE
            safetyNetTitle.value = when (response) {
                SafetyNetHelper.RESPONSE_ERR -> R.string.safetyNet_res_invalid
                else -> R.string.safetyNet_api_error
            }
        }
    }

}