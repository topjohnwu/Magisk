package com.topjohnwu.magisk.ui.safetynet

import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.utils.set

data class SafetyNetResult(
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
            handleResponse(SafetyNetResult(it))
        } ?: attest()
    }

    private fun attest() {
        isChecking = true
        CheckSafetyNetEvent {
            handleResponse(it)
        }.publish()
    }

    fun reset() = attest()

    private fun handleResponse(response: SafetyNetResult) {
        isChecking = false

        if (response.dismiss) {
            back()
            return
        }

        response.response?.apply {
            val result = ctsProfileMatch && basicIntegrity
            cachedResult = this
            ctsState = ctsProfileMatch
            basicIntegrityState = basicIntegrity
            evalType = if (evaluationType.contains("HARDWARE")) "HARDWARE" else "BASIC"
            isSuccess = result
            safetyNetTitle =
                if (result) R.string.safetynet_attest_success
                else R.string.safetynet_attest_failure
        } ?: {
            isSuccess = false
            ctsState = false
            basicIntegrityState = false
            evalType = "N/A"
            safetyNetTitle = R.string.safetynet_api_error
        }()
    }

    companion object {
        private var cachedResult: SafetyNetResponse? = null
    }

}
