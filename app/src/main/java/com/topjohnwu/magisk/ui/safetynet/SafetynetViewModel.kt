package com.topjohnwu.magisk.ui.safetynet

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.utils.set
import org.json.JSONObject

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
    var isChecking = false
        set(value) = set(value, field, { field = it }, BR.checking)

    @get:Bindable
    var isSuccess = false
        set(value) = set(value, field, { field = it }, BR.success, BR.textColorAttr)

    @get:Bindable
    val textColorAttr get() = if (isSuccess) R.attr.colorOnPrimary else R.attr.colorOnError

    init {
        cachedResult?.also {
            resolveResponse(SafetyNetResult(it))
        } ?: attest()
    }

    private fun attest() {
        isChecking = true
        CheckSafetyNetEvent {
            resolveResponse(it)
        }.publish()
    }

    fun reset() = attest()

    private fun resolveResponse(response: SafetyNetResult) {
        isChecking = false

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
                isSuccess = result
                safetyNetTitle =
                    if (result) R.string.safetynet_attest_success
                    else R.string.safetynet_attest_failure
            }.onFailure {
                isSuccess = false
                ctsState = false
                basicIntegrityState = false
                evalType = "N/A"
                safetyNetTitle = R.string.safetynet_res_invalid
            }
        } ?: {
            isSuccess = false
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
