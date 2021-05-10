package com.topjohnwu.magisk.ui.safetynet

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.utils.set

class SafetyNetResult(
    val response: SafetyNetResponse? = null,
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
            handleResult(SafetyNetResult(it))
        } ?: attest()
    }

    private fun attest() {
        isChecking = true
        CheckSafetyNetEvent(::handleResult).publish()
    }

    fun reset() = attest()

    private fun handleResult(result: SafetyNetResult) {
        isChecking = false

        if (result.dismiss) {
            back()
            return
        }

        result.response?.apply {
            cachedResult = this
            if (this === INVALID_RESPONSE) {
                isSuccess = false
                ctsState = false
                basicIntegrityState = false
                evalType = "N/A"
                safetyNetTitle = R.string.safetynet_res_invalid
            } else {
                val success = ctsProfileMatch && basicIntegrity
                isSuccess = success
                ctsState = ctsProfileMatch
                basicIntegrityState = basicIntegrity
                evalType = if (evaluationType.contains("HARDWARE")) "HARDWARE" else "BASIC"
                safetyNetTitle =
                    if (success) R.string.safetynet_attest_success
                    else R.string.safetynet_attest_failure
            }
        } ?: run {
            isSuccess = false
            ctsState = false
            basicIntegrityState = false
            evalType = "N/A"
            safetyNetTitle = R.string.safetynet_api_error
        }
    }

    companion object {
        private var cachedResult: SafetyNetResponse? = null
    }

}
