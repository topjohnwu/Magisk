package com.topjohnwu.magisk.core.utils

import android.content.res.Resources

abstract class TextHolder {

    open val isEmpty: Boolean get() = false
    abstract fun getText(resources: Resources): kotlin.String

    // ---

    class String(
        private val value: kotlin.String
    ) : TextHolder() {
        override val isEmpty get() = value.isEmpty()
        override fun getText(resources: Resources) = value
    }

    open class Resource(
        protected val value: Int
    ) : TextHolder() {
        override val isEmpty get() = value == 0
        override fun getText(resources: Resources) = resources.getString(value)
    }

    class ResourceArgs(
        value: Int,
        private vararg val params: Any
    ) : Resource(value) {
        override fun getText(resources: Resources): kotlin.String {
            // Replace TextHolder with strings
            val args = params.map { if (it is TextHolder) it.getText(resources) else it }
            return resources.getString(value, *args.toTypedArray())
        }
    }

    // ---

    companion object {
        val EMPTY = String("")
    }
}

fun Int.asText(): TextHolder = TextHolder.Resource(this)
fun Int.asText(vararg params: Any): TextHolder = TextHolder.ResourceArgs(this, *params)
fun CharSequence.asText(): TextHolder = TextHolder.String(this.toString())
