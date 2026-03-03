package com.topjohnwu.magisk.view

import android.app.Activity
import android.graphics.drawable.Drawable
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.appcompat.content.res.AppCompatResources
import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.ui.util.rememberDrawablePainter
import top.yukonga.miuix.kmp.basic.Text
import top.yukonga.miuix.kmp.basic.TextButton
import top.yukonga.miuix.kmp.extra.SuperDialog
import top.yukonga.miuix.kmp.theme.MiuixTheme

class MagiskDialog(val context: Activity) {

    val activity: UIActivity get() = context as UIActivity
    val ownerActivity: Activity get() = context
    val showState = mutableStateOf(true)

    internal var title by mutableStateOf<CharSequence>("")
    internal var message by mutableStateOf<CharSequence>("")
    internal var icon by mutableStateOf<Drawable?>(null)
    internal var isCancelable by mutableStateOf(true)
    internal var customContent by mutableStateOf<(@Composable () -> Unit)?>(null)

    val buttonPositive = ButtonState()
    val buttonNeutral = ButtonState()
    val buttonNegative = ButtonState()

    fun setTitle(@StringRes titleId: Int) { title = context.getString(titleId) }
    fun setTitle(title: CharSequence?) { this.title = title ?: "" }
    fun setMessage(@StringRes msgId: Int, vararg args: Any) {
        message = context.getString(msgId, *args)
    }
    fun setMessage(message: CharSequence) { this.message = message }
    fun setIcon(@DrawableRes drawableRes: Int) {
        icon = AppCompatResources.getDrawable(context, drawableRes)
    }
    fun setIcon(drawable: Drawable) { icon = drawable }
    fun setCancelable(cancelable: Boolean) { isCancelable = cancelable }

    fun setButton(buttonType: ButtonType, builder: Button.() -> Unit) {
        when (buttonType) {
            ButtonType.POSITIVE -> buttonPositive
            ButtonType.NEUTRAL -> buttonNeutral
            ButtonType.NEGATIVE -> buttonNegative
        }.apply(builder)
    }

    fun setView(content: @Composable () -> Unit) {
        customContent = content
    }

    fun setListItems(
        list: Array<out CharSequence>,
        listener: DialogClickListener
    ) {
        customContent = {
            Column {
                list.forEachIndexed { index, text ->
                    Text(
                        text = text.toString(),
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable {
                                listener.onClick(index)
                                dismiss()
                            }
                            .padding(horizontal = 16.dp, vertical = 12.dp),
                        style = MiuixTheme.textStyles.body1,
                    )
                }
            }
        }
    }

    fun resetButtons() {
        ButtonType.entries.forEach {
            setButton(it) {
                text = ""
                icon = 0
                isEnabled = true
                doNotDismiss = false
                onClick {}
            }
        }
    }

    fun show() { DialogManager.show(this) }

    fun dismiss() {
        showState.value = false
        DialogManager.dismiss(this)
    }

    enum class ButtonType { POSITIVE, NEUTRAL, NEGATIVE }

    interface Button {
        var icon: Int
        var text: Any
        var isEnabled: Boolean
        var doNotDismiss: Boolean
        fun onClick(listener: () -> Unit)
    }

    inner class ButtonState : Button {
        var label by mutableStateOf("")
        var iconRes by mutableIntStateOf(0)
        override var isEnabled by mutableStateOf(true)
        override var doNotDismiss = false
        internal var onClickAction: () -> Unit = {}

        val isVisible get() = iconRes != 0 || label.isNotEmpty()

        override var icon: Int
            get() = iconRes
            set(value) { iconRes = value }

        override var text: Any
            get() = label
            set(value) {
                label = when (value) {
                    is Int -> context.getText(value)
                    else -> value
                }.toString()
            }

        override fun onClick(listener: () -> Unit) {
            onClickAction = listener
        }

        fun performClick() {
            onClickAction()
            if (!doNotDismiss) dismiss()
        }
    }

    fun interface DialogClickListener {
        fun onClick(position: Int)
    }
}

object DialogManager {
    val dialogs = mutableStateListOf<MagiskDialog>()
    fun show(dialog: MagiskDialog) { dialogs.add(dialog) }
    fun dismiss(dialog: MagiskDialog) { dialogs.remove(dialog) }
}

@Composable
fun MagiskDialogHost() {
    for (dialog in DialogManager.dialogs.toList()) {
        MagiskDialogContent(dialog)
    }
}

@Composable
private fun MagiskDialogContent(dialog: MagiskDialog) {
    SuperDialog(
        title = dialog.title.toString(),
        show = dialog.showState,
        onDismissRequest = {
            if (dialog.isCancelable) dialog.dismiss()
        }
    ) {
        dialog.icon?.let { iconDrawable ->
            Box(modifier = Modifier.fillMaxWidth(), contentAlignment = Alignment.Center) {
                Image(
                    painter = rememberDrawablePainter(iconDrawable),
                    contentDescription = null,
                    modifier = Modifier.size(48.dp)
                )
            }
            Spacer(Modifier.height(8.dp))
        }

        if (dialog.message.isNotEmpty()) {
            Text(
                text = dialog.message.toString(),
                style = MiuixTheme.textStyles.body1,
                modifier = Modifier.fillMaxWidth()
            )
        }

        dialog.customContent?.invoke()

        val hasButtons = dialog.buttonNeutral.isVisible ||
            dialog.buttonNegative.isVisible ||
            dialog.buttonPositive.isVisible
        if (hasButtons) {
            Spacer(Modifier.height(16.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                if (dialog.buttonNeutral.isVisible) {
                    TextButton(
                        text = dialog.buttonNeutral.label,
                        enabled = dialog.buttonNeutral.isEnabled,
                        onClick = { dialog.buttonNeutral.performClick() }
                    )
                }
                Spacer(Modifier.weight(1f))
                if (dialog.buttonNegative.isVisible) {
                    TextButton(
                        text = dialog.buttonNegative.label,
                        enabled = dialog.buttonNegative.isEnabled,
                        onClick = { dialog.buttonNegative.performClick() }
                    )
                }
                if (dialog.buttonPositive.isVisible) {
                    Spacer(Modifier.width(8.dp))
                    TextButton(
                        text = dialog.buttonPositive.label,
                        enabled = dialog.buttonPositive.isEnabled,
                        onClick = { dialog.buttonPositive.performClick() }
                    )
                }
            }
        }
    }
}
