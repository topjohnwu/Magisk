package com.topjohnwu.magisk.extensions

import com.skoumal.teanity.util.KObservableField


fun KObservableField<Boolean>.toggle() {
    value = !value
}