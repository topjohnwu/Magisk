package com.topjohnwu.magisk.model.entity.recycler

import android.content.Context
import android.content.res.Resources
import android.view.MotionEvent
import android.view.View
import androidx.annotation.ArrayRes
import androidx.annotation.CallSuper
import androidx.databinding.Bindable
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.utils.TransitiveText
import com.topjohnwu.magisk.view.MagiskDialog
import org.koin.core.KoinComponent
import org.koin.core.get
import kotlin.properties.ObservableProperty
import kotlin.reflect.KProperty

sealed class SettingsItem : ObservableItem<SettingsItem>() {

    open val icon: Int get() = 0
    open val title: TransitiveText get() = TransitiveText.EMPTY

    @get:Bindable
    open val description: TransitiveText get() = TransitiveText.EMPTY

    @get:Bindable
    var isEnabled by bindable(true, BR.enabled)

    protected open val isFullSpan get() = false

    @CallSuper
    open fun onPressed(view: View, callback: Callback) {
        callback.onItemChanged(view, this)

        // notify only after the callback invocation; callback can invalidate the backing data,
        // which wouldn't be recognized with reverse approach
        notifyChange(BR.description)
    }

    open fun refresh() {}

    override fun onBindingBound(binding: ViewDataBinding) {
        super.onBindingBound(binding)
        if (isFullSpan) {
            val params = binding.root.layoutParams as? StaggeredGridLayoutManager.LayoutParams
            params?.isFullSpan = true
        }
    }

    override fun itemSameAs(other: SettingsItem) = this === other
    override fun contentSameAs(other: SettingsItem) = itemSameAs(other)

    protected inline fun <T> bindable(
        initialValue: T,
        fieldId: Int,
        crossinline setter: (T) -> Unit = {}
    ) = object : ObservableProperty<T>(initialValue) {
        override fun afterChange(property: KProperty<*>, oldValue: T, newValue: T) {
            setter(newValue)
            notifyChange(fieldId)
        }
    }

    // ---

    interface Callback {
        fun onItemPressed(view: View, item: SettingsItem)
        fun onItemChanged(view: View, item: SettingsItem)
    }

    // ---

    abstract class Value<T> : SettingsItem() {

        @get:Bindable
        abstract var value: T

        protected inline fun bindableValue(
            initialValue: T,
            crossinline setter: (T) -> Unit
        ) = bindable(initialValue, BR.value, setter)

    }

    abstract class Toggle : Value<Boolean>() {

        override val layoutRes = R.layout.item_settings_toggle

        override fun onPressed(view: View, callback: Callback) {
            callback.onItemPressed(view, this)
            value = !value
            super.onPressed(view, callback)
        }

        fun onTouched(view: View, callback: Callback, event: MotionEvent): Boolean {
            if (event.action == MotionEvent.ACTION_UP) {
                onPressed(view, callback)
            }
            return true
        }

    }

    abstract class Input : Value<String>(), KoinComponent {

        override val layoutRes = R.layout.item_settings_input
        open val showStrip = true

        protected val resources get() = get<Resources>()
        protected abstract val intermediate: String?

        override fun onPressed(view: View, callback: Callback) {
            callback.onItemPressed(view, this)
            MagiskDialog(view.context)
                .applyTitle(title.getText(resources))
                .applyView(getView(view.context))
                .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                    titleRes = android.R.string.ok
                    onClick {
                        intermediate?.let { result ->
                            preventDismiss = false
                            value = result
                            it.dismiss()
                            super.onPressed(view, callback)
                            return@onClick
                        }
                        preventDismiss = true
                    }
                }
                .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                    titleRes = android.R.string.cancel
                }
                .reveal()
        }

        abstract fun getView(context: Context): View

    }

    abstract class Selector : Value<Int>(), KoinComponent {

        override val layoutRes = R.layout.item_settings_selector

        protected val resources get() = get<Resources>()

        @ArrayRes open val entryRes = -1
        @ArrayRes open val entryValRes = -1

        open val entries get() = resources.getArrayOrEmpty(entryRes)
        open val entryValues get() = resources.getArrayOrEmpty(entryValRes)

        @get:Bindable
        val selectedEntry
            get() = entries.getOrNull(value)

        private fun Resources.getArrayOrEmpty(id: Int): Array<String> =
            runCatching { getStringArray(id) }.getOrDefault(emptyArray())

        override fun onPressed(view: View, callback: Callback) {
            if (entries.isEmpty() || entryValues.isEmpty()) return
            callback.onItemPressed(view, this)
            MagiskDialog(view.context)
                .applyTitle(title.getText(resources))
                .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                    titleRes = android.R.string.cancel
                }
                .applyAdapter(entries) {
                    value = it
                    notifyChange(BR.selectedEntry)
                    super.onPressed(view, callback)
                }
                .reveal()
        }

    }

    abstract class Blank : SettingsItem() {

        override val layoutRes = R.layout.item_settings_blank

        override fun onPressed(view: View, callback: Callback) {
            callback.onItemPressed(view, this)
            super.onPressed(view, callback)
        }

    }

    abstract class Section : SettingsItem() {

        override val layoutRes = R.layout.item_settings_section
        override val isFullSpan get() = true

    }

}
