package com.topjohnwu.magisk.model.entity

class HideTarget(line: String) {

    private val split = line.split(Regex("\\|"), 2)

    val packageName = split[0]
    val process = split.getOrElse(1) { packageName }

}