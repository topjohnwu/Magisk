package com.topjohnwu.magisk.view.dialogs

import android.content.Context
import android.content.DialogInterface
import android.view.LayoutInflater
import android.view.View
import androidx.annotation.StringRes
import androidx.annotation.StyleRes
import androidx.appcompat.app.AlertDialog

import com.topjohnwu.magisk.databinding.AlertDialogBinding

open class CustomAlertDialog : AlertDialog.Builder {

    private var positiveListener: DialogInterface.OnClickListener? = null
    private var negativeListener: DialogInterface.OnClickListener? = null
    private var neutralListener: DialogInterface.OnClickListener? = null

    protected var dialog: AlertDialog? = null
    protected var binding: AlertDialogBinding =
            AlertDialogBinding.inflate(LayoutInflater.from(context))

    init {
        super.setView(binding.root)
        binding.message.visibility = View.GONE
        binding.negative.visibility = View.GONE
        binding.positive.visibility = View.GONE
        binding.neutral.visibility = View.GONE
        binding.buttonPanel.visibility = View.GONE
    }

    constructor(context: Context) : super(context)

    constructor(context: Context, @StyleRes themeResId: Int) : super(context, themeResId)

    override fun setView(layoutResId: Int): CustomAlertDialog {
        return this
    }

    override fun setView(view: View): CustomAlertDialog {
        return this
    }

    override fun setMessage(message: CharSequence?): CustomAlertDialog {
        binding.message.visibility = View.VISIBLE
        binding.message.text = message
        return this
    }

    override fun setMessage(@StringRes messageId: Int): CustomAlertDialog {
        return setMessage(context.getString(messageId))
    }

    override fun setPositiveButton(text: CharSequence, listener: DialogInterface.OnClickListener?): CustomAlertDialog {
        binding.buttonPanel.visibility = View.VISIBLE
        binding.positive.visibility = View.VISIBLE
        binding.positive.text = text
        positiveListener = listener
        binding.positive.setOnClickListener {
            positiveListener?.onClick(dialog, DialogInterface.BUTTON_POSITIVE)
            dialog?.dismiss()
        }
        return this
    }

    override fun setPositiveButton(@StringRes textId: Int, listener: DialogInterface.OnClickListener?): CustomAlertDialog {
        return setPositiveButton(context.getString(textId), listener)
    }

    override fun setNegativeButton(text: CharSequence, listener: DialogInterface.OnClickListener?): CustomAlertDialog {
        binding.buttonPanel.visibility = View.VISIBLE
        binding.negative.visibility = View.VISIBLE
        binding.negative.text = text
        negativeListener = listener
        binding.negative.setOnClickListener {
            negativeListener?.onClick(dialog, DialogInterface.BUTTON_NEGATIVE)
            dialog?.dismiss()
        }
        return this
    }

    override fun setNegativeButton(@StringRes textId: Int, listener: DialogInterface.OnClickListener?): CustomAlertDialog {
        return setNegativeButton(context.getString(textId), listener)
    }

    override fun setNeutralButton(text: CharSequence, listener: DialogInterface.OnClickListener?): CustomAlertDialog {
        binding.buttonPanel.visibility = View.VISIBLE
        binding.neutral.visibility = View.VISIBLE
        binding.neutral.text = text
        neutralListener = listener
        binding.neutral.setOnClickListener {
            neutralListener?.onClick(dialog, DialogInterface.BUTTON_NEUTRAL)
            dialog?.dismiss()
        }
        return this
    }

    override fun setNeutralButton(@StringRes textId: Int, listener: DialogInterface.OnClickListener?): CustomAlertDialog {
        return setNeutralButton(context.getString(textId), listener)
    }

    override fun create(): AlertDialog {
        return super.create().apply { dialog = this }
    }

    override fun show(): AlertDialog {
        return create().apply { show() }
    }

    fun dismiss() {
        dialog?.dismiss()
    }
}
