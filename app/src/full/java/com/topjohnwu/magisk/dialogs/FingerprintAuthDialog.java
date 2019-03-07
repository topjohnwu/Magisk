package com.topjohnwu.magisk.dialogs;

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

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.FingerprintHelper;
import com.topjohnwu.magisk.utils.Utils;

@TargetApi(Build.VERSION_CODES.M)
public class FingerprintAuthDialog extends CustomAlertDialog {

    private Runnable callback;
    private DialogFingerprintHelper helper;

    public FingerprintAuthDialog(@NonNull Activity activity, Runnable onSuccess) {
        super(activity);
        callback = onSuccess;
        Drawable fingerprint = activity.getResources().getDrawable(R.drawable.ic_fingerprint);
        fingerprint.setBounds(0, 0, Utils.dpInPx(50), Utils.dpInPx(50));
        Resources.Theme theme = activity.getTheme();
        TypedArray ta = theme.obtainStyledAttributes(new int[] {R.attr.imageColorTint});
        fingerprint.setTint(ta.getColor(0, Color.GRAY));
        ta.recycle();
        vh.messageView.setCompoundDrawables(null, null, null, fingerprint);
        vh.messageView.setCompoundDrawablePadding(Utils.dpInPx(20));
        vh.messageView.setGravity(Gravity.CENTER);
        setMessage(R.string.auth_fingerprint);
        setNegativeButton(R.string.close, (d, w) -> helper.cancel());
        setOnCancelListener(d -> helper.cancel());
        try {
            helper = new DialogFingerprintHelper();
        } catch (Exception ignored) {}
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
            vh.messageView.setTextColor(Color.RED);
            vh.messageView.setText(errString);
        }

        @Override
        public void onAuthenticationHelp(int helpCode, CharSequence helpString) {
            vh.messageView.setTextColor(Color.RED);
            vh.messageView.setText(helpString);
        }

        @Override
        public void onAuthenticationFailed() {
            vh.messageView.setTextColor(Color.RED);
            vh.messageView.setText(R.string.auth_fail);
        }

        @Override
        public void onAuthenticationSucceeded(FingerprintManager.AuthenticationResult result) {
            dismiss();
            callback.run();
        }
    }
}
