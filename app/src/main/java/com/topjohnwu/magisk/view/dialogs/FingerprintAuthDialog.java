package com.topjohnwu.magisk.view.dialogs;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.hardware.fingerprint.FingerprintManager;
import android.os.Build;
import android.view.Gravity;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.FingerprintHelper;
import com.topjohnwu.magisk.utils.Utils;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;

@TargetApi(Build.VERSION_CODES.M)
public class FingerprintAuthDialog extends CustomAlertDialog {

    private final Runnable callback;
    @Nullable
    private Runnable failureCallback;
    private DialogFingerprintHelper helper;

    public FingerprintAuthDialog(@NonNull Activity activity, @NonNull Runnable onSuccess) {
        super(activity);
        callback = onSuccess;
        Drawable fingerprint = activity.getResources().getDrawable(R.drawable.ic_fingerprint);
        fingerprint.setBounds(0, 0, Utils.dpInPx(50), Utils.dpInPx(50));
        Resources.Theme theme = activity.getTheme();
        TypedArray ta = theme.obtainStyledAttributes(new int[] {R.attr.imageColorTint});
        fingerprint.setTint(ta.getColor(0, Color.GRAY));
        ta.recycle();
        binding.message.setCompoundDrawables(null, null, null, fingerprint);
        binding.message.setCompoundDrawablePadding(Utils.dpInPx(20));
        binding.message.setGravity(Gravity.CENTER);
        setMessage(R.string.auth_fingerprint);
        setNegativeButton(R.string.close, (d, w) -> {
            helper.cancel();
            if (failureCallback != null) {
                failureCallback.run();
            }
        });
        setOnCancelListener(d -> {
            helper.cancel();
            if (failureCallback != null) {
                failureCallback.run();
            }
        });
        try {
            helper = new DialogFingerprintHelper();
        } catch (Exception ignored) {}
    }

    public FingerprintAuthDialog(@NonNull Activity activity, @NonNull Runnable onSuccess, @NonNull Runnable onFailure) {
        this(activity, onSuccess);
        failureCallback = onFailure;
    }

    @Override
    public AlertDialog show() {
        create();
        if (helper == null) {
            dialog.dismiss();
            Utils.toast(R.string.auth_fail, Toast.LENGTH_SHORT);
        } else {
            helper.authenticate();
            dialog.show();
        }
        return dialog;
    }

    class DialogFingerprintHelper extends FingerprintHelper {

        DialogFingerprintHelper() throws Exception {}

        @Override
        public void onAuthenticationError(int errorCode, CharSequence errString) {
            binding.message.setTextColor(Color.RED);
            binding.message.setText(errString);
        }

        @Override
        public void onAuthenticationHelp(int helpCode, CharSequence helpString) {
            binding.message.setTextColor(Color.RED);
            binding.message.setText(helpString);
        }

        @Override
        public void onAuthenticationFailed() {
            binding.message.setTextColor(Color.RED);
            binding.message.setText(R.string.auth_fail);
        }

        @Override
        public void onAuthenticationSucceeded(FingerprintManager.AuthenticationResult result) {
            dismiss();
            callback.run();
        }
    }
}
