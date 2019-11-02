package com.topjohnwu.magisk.extensions

import com.topjohnwu.magisk.utils.KObservableField


fun KObservableField<Boolean>.toggle() {
    value = !value
}