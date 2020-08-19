package com.topjohnwu.magisk.ui.safetynet

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.ui.safetynet.SafetyNetState.*
import com.topjohnwu.magisk.utils.set
import org.json.JSONObject

enum class SafetyNetState {
    LOADING, PASS, FAILED, IDLE
}

data class SafetyNetResult(
    val response: JSONObject? = null,
    val dismiss: Boolean = false
)

class SafetynetViewModel : BaseViewModel() {

    @get:Bindable
    var safetyNetTitle = R.string.empty
        set(value) = set(value, field, { field = it }, BR.safetyNetTitle)

    @get:Bindable
    var ctsState = false
        set(value) = set(value, field, { field = it }, BR.ctsState)

    @get:Bindable
    var basicIntegrityState = false
        set(value) = set(value, field, { field = it }, BR.basicIntegrityState)

    @get:Bindable
    var evalType = ""
        set(value) = set(value, field, { field = it }, BR.evalType)

    @get:Bindable
    val isChecking get() = currentState == LOADING
    @get:Bindable
    val isFailed get() = currentState == FAILED
    @get:Bindable
    val isSuccess get() = currentState == PASS

    private var currentState = IDLE
        set(value) = set(value, field, { field = it }, BR.checking, BR.failed, BR.success)

    init {
        cachedResult?.also {
            resolveResponse(SafetyNetResult(it))
        } ?: attest()
    }

    private fun attest() {
        currentState = LOADING
        CheckSafetyNetEvent {
            resolveResponse(it)
        }.publish()
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
                ctsState = cts
                basicIntegrityState = basic
                evalType = if (eval.contains("HARDWARE")) "HARDWARE" else "BASIC"
                currentState = if (result) PASS else FAILED
                safetyNetTitle =
                    if (result) R.string.safetynet_attest_success
                    else R.string.safetynet_attest_failure
            }.onFailure {
                currentState = FAILED
                ctsState = false
                basicIntegrityState = false
                evalType = "N/A"
                safetyNetTitle = R.string.safetynet_res_invalid
            }
        } ?: {
            currentState = FAILED
            ctsState = false
            basicIntegrityState = false
            evalType = "N/A"
            safetyNetTitle = R.string.safetynet_api_error
        }()
    }

    companion object {
        private var cachedResult: JSONObject? = null
    }

}
