package com.topjohnwu.magisk.view

import android.content.Context
import android.content.DialogInterface
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.view.LayoutInflater
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.appcompat.app.AlertDialog
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import com.skoumal.teanity.util.KObservableField
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.DialogMagiskBaseBinding

class MagiskDialog @JvmOverloads constructor(
    context: Context, theme: Int = 0
) : AlertDialog(context, theme) {

    private val binding: DialogMagiskBaseBinding
    private val data = Data()

    init {
        val layoutInflater = LayoutInflater.from(context)
        binding = DataBindingUtil.inflate(layoutInflater, R.layout.dialog_magisk_base, null, false)
        binding.setVariable(BR.data, data)
        super.setView(binding.root)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window?.setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))
    }

    inner class Data {
        val icon = KObservableField(0)
        val iconRaw = KObservableField<Drawable?>(null)
        val title = KObservableField<CharSequence>("")
        val message = KObservableField<CharSequence>("")

        val buttonPositive = Button()
        val buttonNeutral = Button()
        val buttonNegative = Button()
        val buttonIDGAF = Button()
    }

    enum class ButtonType {
        POSITIVE, NEUTRAL, NEGATIVE, IDGAF
    }

    inner class Button {
        val icon = KObservableField(0)
        val title = KObservableField<CharSequence>("")
        val isEnabled = KObservableField(true)

        var onClickAction: OnDialogButtonClickListener = {}

        fun clicked() {
            onClickAction(this@MagiskDialog)
            dismiss()
        }
    }

    inner class ButtonBuilder(private val button: Button) {
        var icon: Int
            get() = button.icon.value
            set(value) {
                button.icon.value = value
            }
        var title: CharSequence
            get() = button.title.value
            set(value) {
                button.title.value = value
            }
        var titleRes: Int
            get() = 0
            set(value) {
                button.title.value = context.getString(value)
            }
        var isEnabled: Boolean
            get() = button.isEnabled.value
            set(value) {
                button.isEnabled.value = value
            }

        fun onClick(listener: OnDialogButtonClickListener) {
            button.onClickAction = listener
        }
    }

    fun applyTitle(@StringRes stringRes: Int) =
        apply { data.title.value = context.getString(stringRes) }

    fun applyTitle(title: CharSequence) =
        apply { data.title.value = title }

    fun applyMessage(@StringRes stringRes: Int) =
        apply { data.message.value = context.getString(stringRes) }

    fun applyMessage(message: CharSequence) =
        apply { data.message.value = message }

    fun applyIcon(@DrawableRes drawableRes: Int) =
        apply { data.icon.value = drawableRes }

    fun applyIcon(drawable: Drawable) =
        apply { data.iconRaw.value = drawable }

    fun applyButton(buttonType: ButtonType, builder: ButtonBuilder.() -> Unit) = apply {
        val button = when (buttonType) {
            ButtonType.POSITIVE -> data.buttonPositive
            ButtonType.NEUTRAL -> data.buttonNeutral
            ButtonType.NEGATIVE -> data.buttonNegative
            ButtonType.IDGAF -> data.buttonIDGAF
        }
        ButtonBuilder(button).apply(builder)
    }

    fun cancellable(isCancellable: Boolean) = apply {
        setCancelable(isCancellable)
    }

    fun <Binding : ViewDataBinding> applyView(binding: Binding, body: Binding.() -> Unit) =
        apply {
            this.binding.dialogBaseContainer.removeAllViews()
            this.binding.dialogBaseContainer.addView(binding.root)
            binding.apply(body)
        }

    fun onDismiss(callback: OnDialogButtonClickListener) =
        apply { setOnDismissListener(callback) }

    fun onShow(callback: OnDialogButtonClickListener) =
        apply { setOnShowListener(callback) }

    fun reveal() = apply { super.show() }

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