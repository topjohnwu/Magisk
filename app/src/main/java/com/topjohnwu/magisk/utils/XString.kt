package com.topjohnwu.magisk.utils

val specialChars = arrayOf('!', '@', '#', '$', '%', '&', '?')

fun String.replaceRandomWithSpecial(): String {
    var random: Char
    do {
        random = random()
    } while (random == '.')
    return replace(random, specialChars.random())
}