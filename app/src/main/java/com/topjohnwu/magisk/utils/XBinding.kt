package com.topjohnwu.magisk.utils

import com.skoumal.teanity.util.KObservableField


fun KObservableField<Boolean>.toggle() {
    value = !value
}