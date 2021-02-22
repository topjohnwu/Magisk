package com.topjohnwu.magisk.view

import android.content.Context
import android.content.DialogInterface
import android.content.res.ColorStateList
import android.graphics.drawable.Drawable
import android.graphics.drawable.InsetDrawable
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.appcompat.app.AppCompatDialog
import androidx.appcompat.content.res.AppCompatResources
import androidx.databinding.Bindable
import androidx.databinding.PropertyChangeRegistry
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.color.MaterialColors
import com.google.android.material.shape.MaterialShapeDrawable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.itemBindingOf
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.databinding.DialogMagiskBaseBinding
import com.topjohnwu.magisk.utils.ObservableHost
import com.topjohnwu.magisk.utils.set
import me.tatarka.bindingcollectionadapter2.BindingRecyclerViewAdapters
import me.tatarka.bindingcollectionadapter2.ItemBinding

class MagiskDialog(
    context: Context, theme: Int = 0
) : AppCompatDialog(context, theme) {

    private val binding: DialogMagiskBaseBinding =
        DialogMagiskBaseBinding.inflate(LayoutInflater.from(context))
    private val data = Data()

    init {
        binding.setVariable(BR.data, data)
        setCancelable(true)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        super.setContentView(binding.root)

        val default = MaterialColors.getColor(context, R.attr.colorSurface, javaClass.canonicalName)
        val surfaceColor = MaterialColors.getColor(context, R.attr.colorSurfaceSurfaceVariant, default)
        val materialShapeDrawable = MaterialShapeDrawable(context, null, R.attr.alertDialogStyle, R.style.MaterialAlertDialog_MaterialComponents)
        materialShapeDrawable.initializeElevationOverlay(context)
        materialShapeDrawable.fillColor = ColorStateList.valueOf(surfaceColor)
        materialShapeDrawable.elevation = context.resources.getDimension(R.dimen.margin_generic)
        materialShapeDrawable.setCornerSize(context.resources.getDimension(R.dimen.l_50))

        val inset = context.resources.getDimensionPixelSize(R.dimen.appcompat_dialog_background_inset)
        window?.apply {
            setBackgroundDrawable(InsetDrawable(materialShapeDrawable, inset, inset, inset, inset))
            setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        }
    }

    inner class Data : ObservableHost {
        override var callbacks: PropertyChangeRegistry? = null

        @get:Bindable
        var icon: Drawable? = null
            set(value) = set(value, field, { field = it }, BR.icon)

        @get:Bindable
        var title: CharSequence = ""
            set(value) = set(value, field, { field = it }, BR.title)

        @get:Bindable
        var message: CharSequence = ""
            set(value) = set(value, field, { field = it }, BR.message)

        val buttonPositive = Button()
        val buttonNeutral = Button()
        val buttonNegative = Button()
        val buttonIDGAF = Button()
    }

    enum class ButtonType {
        POSITIVE, NEUTRAL, NEGATIVE, IDGAF
    }

    inner class Button : ObservableHost {
        override var callbacks: PropertyChangeRegistry? = null

        @get:Bindable
        var icon = 0
            set(value) = set(value, field, { field = it }, BR.icon)

        @get:Bindable
        var title: CharSequence = ""
            set(value) = set(value, field, { field = it }, BR.title)

        @get:Bindable
        var isEnabled = true
            set(value) = set(value, field, { field = it }, BR.enabled)

        var onClickAction: OnDialogButtonClickListener = {}
        var preventDismiss = false

        fun clicked() {
            //we might not want the click to dismiss the button to begin with
            var prevention = preventDismiss

            onClickAction(this@MagiskDialog)

            //in case we don't want the dialog to close after clicking the button
            //ie. the input is incorrect ...
            //otherwise we disregard the request, bcs it just might reset the button in the new
            //instance
            if (preventDismiss) {
                prevention = preventDismiss
            }

            if (!prevention) {
                dismiss()
            }
        }
    }

    inner class ButtonBuilder(private val button: Button) {
        var icon: Int
            get() = button.icon
            set(value) {
                button.icon = value
            }
        var title: CharSequence
            get() = button.title
            set(value) {
                button.title = value
            }
        var titleRes: Int
            get() = 0
            set(value) {
                button.title = context.getString(value)
            }
        var isEnabled: Boolean
            get() = button.isEnabled
            set(value) {
                button.isEnabled = value
            }
        var preventDismiss: Boolean
            get() = button.preventDismiss
            set(value) {
                button.preventDismiss = value
            }

        fun onClick(listener: OnDialogButtonClickListener) {
            button.onClickAction = listener
        }
    }

    fun applyTitle(@StringRes stringRes: Int) =
        apply { data.title = context.getString(stringRes) }

    fun applyTitle(title: CharSequence) =
        apply { data.title = title }

    fun applyMessage(@StringRes stringRes: Int, vararg args: Any) =
        apply { data.message = context.getString(stringRes, *args) }

    fun applyMessage(message: CharSequence) =
        apply { data.message = message }

    fun applyIcon(@DrawableRes drawableRes: Int) =
        apply {
            data.icon = AppCompatResources.getDrawable(context, drawableRes)
        }

    fun applyIcon(drawable: Drawable) =
        apply { data.icon = drawable }

    fun applyButton(buttonType: ButtonType, builder: ButtonBuilder.() -> Unit) = apply {
        val button = when (buttonType) {
            ButtonType.POSITIVE -> data.buttonPositive
            ButtonType.NEUTRAL -> data.buttonNeutral
            ButtonType.NEGATIVE -> data.buttonNegative
            ButtonType.IDGAF -> data.buttonIDGAF
        }
        ButtonBuilder(button).apply(builder)
    }

    class DialogItem(
        val item: CharSequence,
        val position: Int
    ) : ComparableRvItem<DialogItem>() {
        override val layoutRes = R.layout.item_list_single_line
        override fun itemSameAs(other: DialogItem) = item == other.item
        override fun contentSameAs(other: DialogItem) = itemSameAs(other)
    }

    interface ActualOnDialogClickListener {
        fun onClick(position: Int)
    }

    fun applyAdapter(
        list: Array<out CharSequence>,
        listener: OnDialogClickListener
    ) = applyView(
        RecyclerView(context).also {
            it.isNestedScrollingEnabled = false
            it.layoutManager = LinearLayoutManager(context)

            val actualListener = object : ActualOnDialogClickListener {
                override fun onClick(position: Int) {
                    listener(position)
                    dismiss()
                }
            }
            val items = list.mapIndexed { i, cs -> DialogItem(cs, i) }
            val binding = itemBindingOf<DialogItem> { it.bindExtra(BR.listener, actualListener) }
                .let { ItemBinding.of(it) }

            BindingRecyclerViewAdapters.setAdapter(it, binding, items, null, null, null, null)
        }
    )

    fun cancellable(isCancellable: Boolean) = apply {
        setCancelable(isCancellable)
    }

    fun <Binding : ViewDataBinding> applyView(
        binding: Binding,
        body: Binding.() -> Unit = {}
    ) = applyView(binding.root).also { binding.apply(body) }

    fun applyView(view: View) = apply {
        resetView()
        binding.dialogBaseContainer.addView(
            view,
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
    }

    fun onCancel(callback: OnDialogButtonClickListener) =
        apply { setOnCancelListener(callback) }

    fun onDismiss(callback: OnDialogButtonClickListener) =
        apply { setOnDismissListener(callback) }

    fun onShow(callback: OnDialogButtonClickListener) =
        apply { setOnShowListener(callback) }

    fun reveal() = apply { super.show() }

    // ---

    fun resetView() = apply {
        binding.dialogBaseContainer.removeAllViews()
    }

    fun resetTitle() = applyTitle("")
    fun resetMessage() = applyMessage("")
    fun resetIcon() = apply { data.icon = null }

    fun resetButtons() = apply {
        ButtonType.values().forEach {
            applyButton(it) {
                title = ""
                icon = 0
                isEnabled = true
                preventDismiss = false
                onClick {}
            }
        }
    }

    fun reset() = resetTitle()
        .resetMessage()
        .resetView()
        .resetIcon()
        .resetButtons()

    //region Deprecated Members
    @Deprecated("Use applyTitle instead", ReplaceWith("applyTitle"))
    override fun setTitle(title: CharSequence?) = Unit

    @Deprecated("Use applyTitle instead", ReplaceWith("applyTitle"))
    override fun setTitle(titleId: Int) = Unit

    @Deprecated("Use reveal()", ReplaceWith("reveal()"))
    override fun show() {
    }
    //endregion
}

typealias OnDialogButtonClickListener = (DialogInterface) -> Unit
typealias OnDialogClickListener = (position: Int) -> Unit
