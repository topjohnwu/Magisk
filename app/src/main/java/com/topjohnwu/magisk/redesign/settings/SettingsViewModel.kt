package com.topjohnwu.magisk.redesign.settings

import android.content.Context
import android.content.res.Resources
import android.view.MotionEvent
import android.view.View
import androidx.annotation.CallSuper
import androidx.databinding.Bindable
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.entity.recycler.ObservableItem
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.module.adapterOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf
import com.topjohnwu.magisk.utils.TransitiveText
import com.topjohnwu.magisk.view.MagiskDialog
import org.koin.core.KoinComponent
import org.koin.core.get
import kotlin.properties.ObservableProperty
import kotlin.reflect.KProperty

class SettingsViewModel : CompatViewModel(), SettingsItem.Callback {

    val adapter = adapterOf<SettingsItem>()
    val itemBinding = itemBindingOf<SettingsItem> { it.bindExtra(BR.callback, this) }
    val items = diffListOf(
        Customization,
        Theme, Language, Redesign, DownloadPath,

        Manager,
        UpdateChannel, UpdateChannelUrl, ClearRepoCache, HideOrRestore(), UpdateChecker,
        SystemlessHosts, Biometrics,

        Magisk,
        SafeMode, MagiskHide,

        Superuser,
        AccessMode, MultiuserMode, MountNamespaceMode, AutomaticResponse, RequestTimeout,
        SUNotification
    )

    override fun onItemPressed(view: View, item: SettingsItem) = when (item) {
        // use only instances you want, don't declare everything
        is Theme -> Navigation.theme().publish()
        is Redesign -> DieEvent().publish()
        is UpdateChannel -> item.openUrlIfNecessary(view)
        else -> Unit
    }

    private fun UpdateChannel.openUrlIfNecessary(view: View) {
        if (value == Config.Value.CUSTOM_CHANNEL) {
            if (UpdateChannelUrl.value.isBlank()) {
                UpdateChannelUrl.onPressed(view, this@SettingsViewModel)
            }
            UpdateChannelUrl.isEnabled = true
        } else {
            UpdateChannelUrl.isEnabled = false
        }
    }

}

sealed class SettingsItem : ObservableItem<SettingsItem>() {

    @Bindable
    open val icon: Int = 0
    @Bindable
    open val title: TransitiveText = TransitiveText.empty
    @Bindable
    open val description: TransitiveText = TransitiveText.empty

    var isEnabled = true
        @Bindable get
        set(value) {
            field = value
            notifyChange(BR.enabled)
        }

    protected open val isFullSpan: Boolean = false

    @CallSuper
    open fun onPressed(view: View, callback: Callback) {
        callback.onItemPressed(view, this)

        // notify only after the callback invocation; callback can invalidate the backing data,
        // which wouldn't be recognized with reverse approach
        notifyChange(BR.icon)
        notifyChange(BR.title)
        notifyChange(BR.description)
    }

    override fun onBindingBound(binding: ViewDataBinding) {
        super.onBindingBound(binding)
        if (isFullSpan) {
            val params = binding.root.layoutParams as? StaggeredGridLayoutManager.LayoutParams
            params?.isFullSpan = true
        }
    }

    override fun itemSameAs(other: SettingsItem) = this === other
    override fun contentSameAs(other: SettingsItem) = itemSameAs(other)

    // ---

    interface Callback {
        fun onItemPressed(view: View, item: SettingsItem)
    }

    // ---

    abstract class Value<T> : SettingsItem() {

        abstract var value: T
            @Bindable get

        protected inline fun dataObservable(
            initialValue: T,
            crossinline setter: (T) -> Unit
        ) = object : ObservableProperty<T>(initialValue) {
            override fun afterChange(property: KProperty<*>, oldValue: T, newValue: T) {
                setter(newValue)
                notifyChange(BR.value)
            }
        }

    }

    abstract class Toggle : Value<Boolean>() {

        override val layoutRes = R.layout.item_settings_toggle

        override fun onPressed(view: View, callback: Callback) {
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

        protected val resources get() = get<Resources>()
        protected abstract val intermediate: String?

        override fun onPressed(view: View, callback: Callback) {
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

        @Bindable
        var entries: Array<out CharSequence> = arrayOf()
            private set
        @Bindable
        var entryValues: Array<out CharSequence> = arrayOf()
            private set

        val selectedEntry
            @Bindable get() = entries.getOrNull(value)

        fun setValues(
            entries: Array<out CharSequence>,
            values: Array<out CharSequence>
        ) {
            check(entries.size <= values.size) { "List sizes must match" }

            this.entries = entries
            this.entryValues = values

            notifyChange(BR.entries)
            notifyChange(BR.entryValues)
            notifyChange(BR.selectedEntry)
        }

        override fun onPressed(view: View, callback: Callback) {
            if (entries.isEmpty() || entryValues.isEmpty()) return
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

    }

    abstract class Section : SettingsItem() {

        override val layoutRes = R.layout.item_settings_section
        override val isFullSpan = true

    }

}