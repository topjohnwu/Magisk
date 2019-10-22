package com.topjohnwu.magisk.ui.install

import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.utils.KObservableField

class InstallViewModel : CompatViewModel() {

    val step = KObservableField(0)
    val method = KObservableField(-1)

    fun step(nextStep: Int) {
        step.value = nextStep
    }

}