package com.topjohnwu.magisk.view

import android.app.Activity
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
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.color.MaterialColors
import com.google.android.material.shape.MaterialShapeDrawable
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.databinding.DialogMagiskBaseBinding
import com.topjohnwu.magisk.databinding.DiffItem
import com.topjohnwu.magisk.databinding.ItemWrapper
import com.topjohnwu.magisk.databinding.ObservableHost
import com.topjohnwu.magisk.databinding.RvItem
import com.topjohnwu.magisk.databinding.bindExtra
import com.topjohnwu.magisk.databinding.set
import com.topjohnwu.magisk.databinding.setAdapter
import com.topjohnwu.magisk.view.MagiskDialog.DialogClickListener

typealias DialogButtonClickListener = (DialogInterface) -> Unit

class MagiskDialog(
    context: Activity, theme: Int = 0
) : AppCompatDialog(context, theme) {

    private val binding: DialogMagiskBaseBinding =
        DialogMagiskBaseBinding.inflate(LayoutInflater.from(context))
    private val data = Data()

    val activity: BaseActivity get() = ownerActivity as BaseActivity

    init {
        binding.setVariable(BR.data, data)
        setCancelable(true)
        setOwnerActivity(context)
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

        val buttonPositive = ButtonViewModel()
        val buttonNeutral = ButtonViewModel()
        val buttonNegative = ButtonViewModel()
    }

    enum class ButtonType {
        POSITIVE, NEUTRAL, NEGATIVE
    }

    interface Button {
        var icon: Int
        var text: Any
        var isEnabled: Boolean
        var doNotDismiss: Boolean

        fun onClick(listener: DialogButtonClickListener)
    }

    inner class ButtonViewModel : Button, ObservableHost {
        override var callbacks: PropertyChangeRegistry? = null

        @get:Bindable
        override var icon = 0
            set(value) = set(value, field, { field = it }, BR.icon, BR.gone)

        @get:Bindable
        var message: String = ""
            set(value) = set(value, field, { field = it }, BR.message, BR.gone)

        override var text: Any
            get() = message
            set(value) {
                message = when (value) {
                    is Int -> context.getText(value)
                    else -> value
                }.toString()
            }

        @get:Bindable
        val gone get() = icon == 0 && message.isEmpty()

        @get:Bindable
        override var isEnabled = true
            set(value) = set(value, field, { field = it }, BR.enabled)

        override var doNotDismiss = false

        private var onClickAction: DialogButtonClickListener = {}

        override fun onClick(listener: DialogButtonClickListener) {
            onClickAction = listener
        }

        fun clicked() {
            onClickAction(this@MagiskDialog)
            if (!doNotDismiss) {
                dismiss()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        super.setContentView(binding.root)

        val default = MaterialColors.getColor(context, com.google.android.material.R.attr.colorSurface, javaClass.canonicalName)
        val surfaceColor = MaterialColors.getColor(context, R.attr.colorSurfaceSurfaceVariant, default)
        val materialShapeDrawable = MaterialShapeDrawable(context, null, androidx.appcompat.R.attr.alertDialogStyle, com.google.android.material.R.style.MaterialAlertDialog_MaterialComponents)
        materialShapeDrawable.initializeElevationOverlay(context)
        materialShapeDrawable.fillColor = ColorStateList.valueOf(surfaceColor)
        materialShapeDrawable.elevation = context.resources.getDimension(R.dimen.margin_generic)
        materialShapeDrawable.setCornerSize(context.resources.getDimension(R.dimen.l_50))

        val inset = context.resources.getDimensionPixelSize(com.google.android.material.R.dimen.appcompat_dialog_background_inset)
        window?.apply {
            setBackgroundDrawable(InsetDrawable(materialShapeDrawable, inset, inset, inset, inset))
            setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        }
    }

    override fun setTitle(@StringRes titleId: Int) { data.title = context.getString(titleId) }

    override fun setTitle(title: CharSequence?) { data.title = title ?: "" }

    fun setMessage(@StringRes msgId: Int, vararg args: Any) {
        data.message = context.getString(msgId, *args)
    }

    fun setMessage(message: CharSequence) { data.message = message }

    fun setIcon(@DrawableRes drawableRes: Int) {
        data.icon = AppCompatResources.getDrawable(context, drawableRes)
    }

    fun setIcon(drawable: Drawable) { data.icon = drawable }

    fun setButton(buttonType: ButtonType, builder: Button.() -> Unit) {
        val button = when (buttonType) {
            ButtonType.POSITIVE -> data.buttonPositive
            ButtonType.NEUTRAL -> data.buttonNeutral
            ButtonType.NEGATIVE -> data.buttonNegative
        }
        button.apply(builder)
    }

    class DialogItem(
        override val item: CharSequence,
        val position: Int
    ) : RvItem(), DiffItem<DialogItem>, ItemWrapper<CharSequence> {
        override val layoutRes = R.layout.item_list_single_line
    }

    fun interface DialogClickListener {
        fun onClick(position: Int)
    }

    fun setListItems(
        list: Array<out CharSequence>,
        listener: DialogClickListener
    ) = setView(
        RecyclerView(context).also {
            it.isNestedScrollingEnabled = false
            it.layoutManager = LinearLayoutManager(context)

            val items = list.mapIndexed { i, cs -> DialogItem(cs, i) }
            val extraBindings = bindExtra { sa ->
                sa.put(BR.listener, DialogClickListener { pos ->
                    listener.onClick(pos)
                    dismiss()
                })
            }
            it.setAdapter(items, extraBindings)
        }
    )

    fun setView(view: View) {
        binding.dialogBaseContainer.removeAllViews()
        binding.dialogBaseContainer.addView(
            view,
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
    }

    fun resetButtons() {
        ButtonType.values().forEach {
            setButton(it) {
                text = ""
                icon = 0
                isEnabled = true
                doNotDismiss = false
                onClick {}
            }
        }
    }

    // Prevent calling setContentView

    @Deprecated("Please use setView(view)", level = DeprecationLevel.ERROR)
    override fun setContentView(layoutResID: Int) {}
    @Deprecated("Please use setView(view)", level = DeprecationLevel.ERROR)
    override fun setContentView(view: View) {}
    @Deprecated("Please use setView(view)", level = DeprecationLevel.ERROR)
    override fun setContentView(view: View, params: ViewGroup.LayoutParams?) {}
}
