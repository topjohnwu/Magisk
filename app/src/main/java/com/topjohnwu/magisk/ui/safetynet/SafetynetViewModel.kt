package com.topjohnwu.magisk.ui.safetynet

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.events.SafetyNetResult
import com.topjohnwu.magisk.model.events.UpdateSafetyNetEvent
import com.topjohnwu.magisk.ui.base.BaseViewModel
import com.topjohnwu.magisk.ui.safetynet.SafetyNetState.*
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.RxBus
import org.json.JSONObject

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
    val evalType = KObservableField("")

    val isChecking @Bindable get() = currentState == LOADING
    val isFailed @Bindable get() = currentState == FAILED
    val isSuccess @Bindable get() = currentState == PASS

    init {
        rxBus.register<SafetyNetResult>()
            .subscribeK { resolveResponse(it) }
            .add()

        cachedResult?.also {
            resolveResponse(SafetyNetResult(it))
        } ?: attest()
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

    private fun resolveResponse(response: SafetyNetResult) {
        if (response.dismiss) {
            back()
            return
        }

        response.response?.apply {
            runCatching {
                val cts = getBoolean("ctsProfileMatch")
                val basic = getBoolean("basicIntegrity")
                val eval = optString("evaluationType")
                val result = cts && basic
                cachedResult = this
                ctsState.value = cts
                basicIntegrityState.value = basic
                evalType.value = if (eval.contains("HARDWARE")) "HARDWARE" else "BASIC"
                currentState = if (result) PASS else FAILED
                safetyNetTitle.value =
                    if (result) R.string.safetynet_attest_success
                    else R.string.safetynet_attest_failure
            }.onFailure {
                currentState = FAILED
                ctsState.value = false
                basicIntegrityState.value = false
                evalType.value = "N/A"
                safetyNetTitle.value = R.string.safetynet_res_invalid
            }
        } ?: {
            currentState = FAILED
            ctsState.value = false
            basicIntegrityState.value = false
            evalType.value = "N/A"
            safetyNetTitle.value = R.string.safetynet_api_error
        }()
    }

    companion object {
        private var cachedResult: JSONObject? = null
    }

}
