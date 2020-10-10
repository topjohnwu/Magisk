package com.topjohnwu.magisk.core.base

typealias SimpleCallback = () -> Unit
typealias PermissionRationaleCallback = (List<String>) -> Unit

class PermissionRequestBuilder {

    private var onSuccessCallback: SimpleCallback = {}
    private var onFailureCallback: SimpleCallback = {}
    private var onShowRationaleCallback: PermissionRationaleCallback = {}

    fun onSuccess(callback: SimpleCallback) {
        onSuccessCallback = callback
    }

    fun onFailure(callback: SimpleCallback) {
        onFailureCallback = callback
    }

    fun onShowRationale(callback: PermissionRationaleCallback) {
        onShowRationaleCallback = callback
    }

    fun build(): PermissionRequest {
        return PermissionRequest(onSuccessCallback, onFailureCallback, onShowRationaleCallback)
    }

}

class PermissionRequest(
    private val onSuccessCallback: SimpleCallback,
    private val onFailureCallback: SimpleCallback,
    private val onShowRationaleCallback: PermissionRationaleCallback
) {

    fun onSuccess() = onSuccessCallback()
    fun onFailure() = onFailureCallback()
    fun onShowRationale(permissions: List<String>) = onShowRationaleCallback(permissions)

}
