package com.topjohnwu.magisk.ui.hide

class HideTarget(line: String) {

    val packageName: String
    val process: String

    init {
        val split = line.split(Regex("\\|"), 2)
        packageName = split[0]
        process = split.getOrElse(1) { packageName }
    }

}
