package com.topjohnwu.magisk.ui.settings

import android.content.Context
import android.content.res.Resources
import android.view.View
import androidx.databinding.Bindable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.ktx.activity
import com.topjohnwu.magisk.databinding.ObservableRvItem
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.utils.TextHolder
import com.topjohnwu.magisk.view.MagiskDialog

sealed class BaseSettingsItem : ObservableRvItem() {

    interface Handler {
        fun onItemPressed(view: View, item: BaseSettingsItem, andThen: () -> Unit)
        fun onItemAction(view: View, item: BaseSettingsItem)
    }

    override val layoutRes get() = R.layout.item_settings

    open val icon: Int get() = 0
    open val title: TextHolder get() = TextHolder.EMPTY
    @get:Bindable
    open val description: TextHolder get() = TextHolder.EMPTY
    @get:Bindable
    var isEnabled = true
        set(value) = set(value, field, { field = it }, BR.enabled, BR.description)

    open fun onPressed(view: View, handler: Handler) {
        handler.onItemPressed(view, this) {
            handler.onItemAction(view, this)
        }
    }
    open fun refresh() {}

    // Only for toggle
    open val showSwitch get() = false
    @get:Bindable
    open val isChecked get() = false
    fun onToggle(view: View, handler: Handler, checked: Boolean) =
        set(checked, isChecked, { onPressed(view, handler) })

    abstract class Value<T> : BaseSettingsItem() {

        /**
         * Represents last agreed-upon value by the validation process and the user for current
         * child. Be very aware that this shouldn't be **set** unless both sides agreed that _that_
         * is the new value.
         * */
        abstract var value: T
            protected set
    }

    abstract class Toggle : Value<Boolean>() {

        override val showSwitch get() = true
        override val isChecked get() = value

        override fun onPressed(view: View, handler: Handler) {
            // Make sure the checked state is synced
            notifyPropertyChanged(BR.checked)
            handler.onItemPressed(view, this) {
                value = !value
                notifyPropertyChanged(BR.checked)
                handler.onItemAction(view, this)
            }
        }
    }

    abstract class Input : Value<String>() {

        @get:Bindable
        abstract val inputResult: String?

        override fun onPressed(view: View, handler: Handler) {
            handler.onItemPressed(view, this) {
                MagiskDialog(view.activity).apply {
                    setTitle(title.getText(view.resources))
                    setView(getView(view.context))
                    setButton(MagiskDialog.ButtonType.POSITIVE) {
                        text = android.R.string.ok
                        onClick {
                            inputResult?.let { result ->
                                doNotDismiss = false
                                value = result
                                handler.onItemAction(view, this@Input)
                                return@onClick
                            }
                            doNotDismiss = true
                        }
                    }
                    setButton(MagiskDialog.ButtonType.NEGATIVE) {
                        text = android.R.string.cancel
                    }
                }.show()
            }
        }

        abstract fun getView(context: Context): View
    }

    abstract class Selector : Value<Int>() {

        open val entryRes get() = -1
        open val descriptionRes get() = entryRes
        open fun entries(res: Resources) = res.getArrayOrEmpty(entryRes)
        open fun descriptions(res: Resources) = res.getArrayOrEmpty(descriptionRes)

        override val description = object : TextHolder() {
            override fun getText(resources: Resources): CharSequence {
                return descriptions(resources).getOrElse(value) { "" }
            }
        }

        private fun Resources.getArrayOrEmpty(id: Int): Array<String> =
            runCatching { getStringArray(id) }.getOrDefault(emptyArray())

        override fun onPressed(view: View, handler: Handler) {
            handler.onItemPressed(view, this) {
                MagiskDialog(view.activity).apply {
                    setTitle(title.getText(view.resources))
                    setButton(MagiskDialog.ButtonType.NEGATIVE) {
                        text = android.R.string.cancel
                    }
                    setListItems(entries(view.resources)) {
                        if (value != it) {
                            value = it
                            notifyPropertyChanged(BR.description)
                            handler.onItemAction(view, this@Selector)
                        }
                    }
                }.show()
            }
        }
    }

    abstract class Blank : BaseSettingsItem()

    abstract class Section : BaseSettingsItem() {
        override val layoutRes = R.layout.item_settings_section
    }
}
