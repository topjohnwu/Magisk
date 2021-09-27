package com.topjohnwu.magisk.ktx

val specialChars = arrayOf('!', '@', '#', '$', '%', '&', '?')
val fullSpecialChars = arrayOf('！', '＠', '＃', '＄', '％', '＆', '？')

fun String.isCJK(): Boolean {
    for (i in 0 until length)
        if (isCJK(codePointAt(i)))
            return true
    return false
}

fun isCJK(codepoint: Int) = Character.isIdeographic(codepoint)

fun String.replaceRandomWithSpecial(passes: Int): String {
    var string = this
    repeat(passes) {
        string = string.replaceRandomWithSpecial()
    }
    return string
}

fun String.replaceRandomWithSpecial(): String {
    val sp = if (isCJK()) fullSpecialChars else specialChars
    var random: Char
    do {
        random = random()
    } while (random == '.')
    return replace(random, sp.random())
}

fun StringBuilder.appendIf(condition: Boolean, builder: StringBuilder.() -> Unit) =
    if (condition) apply(builder) else this

fun String.trimEmptyToNull(): String? = if (isBlank()) null else this

fun String.legalFilename() = replace(" ", "_").replace("'", "").replace("\"", "")
        .replace("$", "").replace("`", "").replace("*", "").replace("/", "_")
        .replace("#", "").replace("@", "").replace("\\", "_")

fun String.isEmptyInternal() = isNullOrBlank()
