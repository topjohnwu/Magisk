package com.topjohnwu.magisk.model.entity.internal

import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.model.observer.Observer

class DownloadDialogData(initialValue: String) {

    val text = KObservableField(initialValue)
    val path = Observer(text) { Config.downloadsFile(text.value)?.absolutePath.orEmpty() }

}