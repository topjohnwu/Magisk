package com.topjohnwu.magisk.redesign.home

import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.redesign.compat.CompatViewModel

class HomeViewModel : CompatViewModel() {

    val stateTextMagisk = KObservableField("is up to date")
    val stateTextManager = KObservableField("is up to date")

    fun onDeletePressed() {}

}