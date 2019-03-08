package com.topjohnwu.magisk.dialogs;

import android.app.Activity;
import android.content.DialogInterface;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.annotation.StyleRes;
import androidx.appcompat.app.AlertDialog;

import com.topjohnwu.magisk.R;

import butterknife.BindView;

public class CustomAlertDialog extends AlertDialog.Builder {

    private DialogInterface.OnClickListener positiveListener;
    private DialogInterface.OnClickListener negativeListener;
    private DialogInterface.OnClickListener neutralListener;

    protected AlertDialog dialog;
    protected ViewHolder vh;

    public class ViewHolder {
        @BindView(R.id.dialog_layout) public LinearLayout dialogLayout;
        @BindView(R.id.button_panel) public LinearLayout buttons;

        @BindView(R.id.message) public TextView messageView;
        @BindView(R.id.negative) public Button negative;
        @BindView(R.id.positive) public Button positive;
        @BindView(R.id.neutral) public Button neutral;

        ViewHolder(View v) {
            new CustomAlertDialog$ViewHolder_ViewBinding(this, v);
            messageView.setVisibility(View.GONE);
            negative.setVisibility(View.GONE);
            positive.setVisibility(View.GONE);
            neutral.setVisibility(View.GONE);
            buttons.setVisibility(View.GONE);
        }
    }

    {
        View v = LayoutInflater.from(getContext()).inflate(R.layout.alert_dialog, null);
        vh = new ViewHolder(v);
        super.setView(v);

    }

    public CustomAlertDialog(@NonNull Activity context) {
        super(context);
    }

    public CustomAlertDialog(@NonNull Activity context, @StyleRes int themeResId) {
        super(context, themeResId);
    }

    public ViewHolder getViewHolder() {
        return vh;
    }

    @Override
    public CustomAlertDialog setView(int layoutResId) { return this; }

    @Override
    public CustomAlertDialog setView(View view) { return this; }

    @Override
    public CustomAlertDialog setMessage(@Nullable CharSequence message) {
        vh.messageView.setVisibility(View.VISIBLE);
        vh.messageView.setText(message);
        return this;
    }

    @Override
    public CustomAlertDialog setMessage(@StringRes int messageId) {
        return setMessage(getContext().getString(messageId));
    }

    @Override
    public CustomAlertDialog setPositiveButton(CharSequence text, DialogInterface.OnClickListener listener) {
        vh.buttons.setVisibility(View.VISIBLE);
        vh.positive.setVisibility(View.VISIBLE);
        vh.positive.setText(text);
        positiveListener = listener;
        vh.positive.setOnClickListener((v) -> {
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
        vh.buttons.setVisibility(View.VISIBLE);
        vh.negative.setVisibility(View.VISIBLE);
        vh.negative.setText(text);
        negativeListener = listener;
        vh.negative.setOnClickListener((v) -> {
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
        vh.buttons.setVisibility(View.VISIBLE);
        vh.neutral.setVisibility(View.VISIBLE);
        vh.neutral.setText(text);
        neutralListener = listener;
        vh.neutral.setOnClickListener((v) -> {
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
