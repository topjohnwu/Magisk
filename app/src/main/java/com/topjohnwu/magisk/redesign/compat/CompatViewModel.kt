package com.topjohnwu.magisk.redesign.compat

import android.graphics.Insets
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.ui.base.MagiskViewModel

abstract class CompatViewModel : MagiskViewModel() {

    val insets = KObservableField(Insets.NONE)

}