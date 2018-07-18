package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.content.DialogInterface;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.StringRes;
import android.support.annotation.StyleRes;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.R;

import butterknife.BindView;
import butterknife.ButterKnife;

public class AlertDialogBuilder extends AlertDialog.Builder {

    @BindView(R.id.button_panel) LinearLayout buttons;
    @BindView(R.id.message_panel) LinearLayout messagePanel;

    @BindView(R.id.negative) Button negative;
    @BindView(R.id.positive) Button positive;
    @BindView(R.id.neutral) Button neutral;
    @BindView(R.id.message) TextView messageView;

    private DialogInterface.OnClickListener positiveListener;
    private DialogInterface.OnClickListener negativeListener;
    private DialogInterface.OnClickListener neutralListener;

    private AlertDialog dialog;

    public AlertDialogBuilder(@NonNull Activity context) {
        super(context);
        setup();
    }

    public AlertDialogBuilder(@NonNull Activity context, @StyleRes int themeResId) {
        super(context, themeResId);
        setup();
    }

    private void setup() {
        View v = LayoutInflater.from(getContext()).inflate(R.layout.alert_dialog, null);
        ButterKnife.bind(this, v);
        super.setView(v);
        negative.setVisibility(View.GONE);
        positive.setVisibility(View.GONE);
        neutral.setVisibility(View.GONE);
        buttons.setVisibility(View.GONE);
        messagePanel.setVisibility(View.GONE);
    }

    @Override
    public AlertDialog.Builder setTitle(int titleId) {
        return super.setTitle(titleId);
    }

    @Override
    public AlertDialog.Builder setView(int layoutResId) { return this; }

    @Override
    public AlertDialog.Builder setView(View view) { return this; }

    @Override
    public AlertDialog.Builder setMessage(@Nullable CharSequence message) {
        messageView.setText(message);
        messagePanel.setVisibility(View.VISIBLE);
        return this;
    }

    @Override
    public AlertDialog.Builder setMessage(@StringRes int messageId) {
        return setMessage(getContext().getString(messageId));
    }

    @Override
    public AlertDialog.Builder setPositiveButton(CharSequence text, DialogInterface.OnClickListener listener) {
        buttons.setVisibility(View.VISIBLE);
        positive.setVisibility(View.VISIBLE);
        positive.setText(text);
        positiveListener = listener;
        positive.setOnClickListener((v) -> {
            if (positiveListener != null) {
                positiveListener.onClick(dialog, DialogInterface.BUTTON_POSITIVE);
            }
            dialog.dismiss();
        });
        return this;
    }

    @Override
    public AlertDialog.Builder setPositiveButton(@StringRes int textId, DialogInterface.OnClickListener listener) {
        return setPositiveButton(getContext().getString(textId), listener);
    }

    @Override
    public AlertDialog.Builder setNegativeButton(CharSequence text, DialogInterface.OnClickListener listener) {
        buttons.setVisibility(View.VISIBLE);
        negative.setVisibility(View.VISIBLE);
        negative.setText(text);
        negativeListener = listener;
        negative.setOnClickListener((v) -> {
            if (negativeListener != null) {
                negativeListener.onClick(dialog, DialogInterface.BUTTON_NEGATIVE);
            }
            dialog.dismiss();
        });
        return this;
    }

    @Override
    public AlertDialog.Builder setNegativeButton(@StringRes int textId, DialogInterface.OnClickListener listener) {
        return setNegativeButton(getContext().getString(textId), listener);
    }

    @Override
    public AlertDialog.Builder setNeutralButton(CharSequence text, DialogInterface.OnClickListener listener) {
        buttons.setVisibility(View.VISIBLE);
        neutral.setVisibility(View.VISIBLE);
        neutral.setText(text);
        neutralListener = listener;
        neutral.setOnClickListener((v) -> {
            if (neutralListener != null) {
                neutralListener.onClick(dialog, DialogInterface.BUTTON_NEUTRAL);
            }
            dialog.dismiss();
        });
        return this;
    }

    @Override
    public AlertDialog.Builder setNeutralButton(@StringRes int textId, DialogInterface.OnClickListener listener) {
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
}
