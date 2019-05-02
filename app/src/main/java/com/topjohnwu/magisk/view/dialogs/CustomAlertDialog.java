package com.topjohnwu.magisk.view.dialogs;

import android.content.Context;
import android.content.DialogInterface;
import android.view.LayoutInflater;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.annotation.StyleRes;
import androidx.appcompat.app.AlertDialog;

import com.topjohnwu.magisk.databinding.AlertDialogBinding;

public class CustomAlertDialog extends AlertDialog.Builder {

    private DialogInterface.OnClickListener positiveListener;
    private DialogInterface.OnClickListener negativeListener;
    private DialogInterface.OnClickListener neutralListener;

    protected AlertDialog dialog;
    protected AlertDialogBinding binding;

    {
        binding = AlertDialogBinding.inflate(LayoutInflater.from(getContext()));
        super.setView(binding.getRoot());
        binding.message.setVisibility(View.GONE);
        binding.negative.setVisibility(View.GONE);
        binding.positive.setVisibility(View.GONE);
        binding.neutral.setVisibility(View.GONE);
        binding.buttonPanel.setVisibility(View.GONE);
    }

    public CustomAlertDialog(@NonNull Context context) {
        super(context);
    }

    public CustomAlertDialog(@NonNull Context context, @StyleRes int themeResId) {
        super(context, themeResId);
    }

    @Override
    public CustomAlertDialog setView(int layoutResId) { return this; }

    @Override
    public CustomAlertDialog setView(View view) { return this; }

    @Override
    public CustomAlertDialog setMessage(@Nullable CharSequence message) {
        binding.message.setVisibility(View.VISIBLE);
        binding.message.setText(message);
        return this;
    }

    @Override
    public CustomAlertDialog setMessage(@StringRes int messageId) {
        return setMessage(getContext().getString(messageId));
    }

    @Override
    public CustomAlertDialog setPositiveButton(CharSequence text, DialogInterface.OnClickListener listener) {
        binding.buttonPanel.setVisibility(View.VISIBLE);
        binding.positive.setVisibility(View.VISIBLE);
        binding.positive.setText(text);
        positiveListener = listener;
        binding.positive.setOnClickListener(v -> {
            if (positiveListener != null) {
                positiveListener.onClick(dialog, DialogInterface.BUTTON_POSITIVE);
            }
            dialog.dismiss();
        });
        return this;
    }

    @Override
    public CustomAlertDialog setPositiveButton(@StringRes int textId, DialogInterface.OnClickListener listener) {
        return setPositiveButton(getContext().getString(textId), listener);
    }

    @Override
    public CustomAlertDialog setNegativeButton(CharSequence text, DialogInterface.OnClickListener listener) {
        binding.buttonPanel.setVisibility(View.VISIBLE);
        binding.negative.setVisibility(View.VISIBLE);
        binding.negative.setText(text);
        negativeListener = listener;
        binding.negative.setOnClickListener(v -> {
            if (negativeListener != null) {
                negativeListener.onClick(dialog, DialogInterface.BUTTON_NEGATIVE);
            }
            dialog.dismiss();
        });
        return this;
    }

    @Override
    public CustomAlertDialog setNegativeButton(@StringRes int textId, DialogInterface.OnClickListener listener) {
        return setNegativeButton(getContext().getString(textId), listener);
    }

    @Override
    public CustomAlertDialog setNeutralButton(CharSequence text, DialogInterface.OnClickListener listener) {
        binding.buttonPanel.setVisibility(View.VISIBLE);
        binding.neutral.setVisibility(View.VISIBLE);
        binding.neutral.setText(text);
        neutralListener = listener;
        binding.neutral.setOnClickListener(v -> {
            if (neutralListener != null) {
                neutralListener.onClick(dialog, DialogInterface.BUTTON_NEUTRAL);
            }
            dialog.dismiss();
        });
        return this;
    }

    @Override
    public CustomAlertDialog setNeutralButton(@StringRes int textId, DialogInterface.OnClickListener listener) {
        return setNeutralButton(getContext().getString(textId), listener);
    }

    @Override
    public AlertDialog create() {
        dialog = super.create();
        return dialog;
    }

    @Override
    public AlertDialog show() {
        create();
        dialog.show();
        return dialog;
    }

    public void dismiss() {
        dialog.dismiss();
    }
}
