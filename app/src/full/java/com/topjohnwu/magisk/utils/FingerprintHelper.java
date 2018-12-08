package com.topjohnwu.magisk.utils;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.KeyguardManager;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.hardware.fingerprint.FingerprintManager;
import android.os.Build;
import android.os.CancellationSignal;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyPermanentlyInvalidatedException;
import android.security.keystore.KeyProperties;
import android.view.Gravity;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.CustomAlertDialog;

import java.security.KeyStore;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;

@TargetApi(Build.VERSION_CODES.M)
public abstract class FingerprintHelper {

    private FingerprintManager manager;
    private Cipher cipher;
    private CancellationSignal cancel;

    public static boolean useFingerPrint() {
        MagiskManager mm = Data.MM();
        boolean fp = mm.mDB.getSettings(Const.Key.SU_FINGERPRINT, 0) != 0;
        if (fp && !canUseFingerprint()) {
            mm.mDB.setSettings(Const.Key.SU_FINGERPRINT, 0);
            fp = false;
        }
        return fp;
    }

    public static boolean canUseFingerprint() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M)
            return false;
        MagiskManager mm = Data.MM();
        KeyguardManager km = mm.getSystemService(KeyguardManager.class);
        FingerprintManager fm = mm.getSystemService(FingerprintManager.class);
        return km.isKeyguardSecure() && fm != null && fm.isHardwareDetected() && fm.hasEnrolledFingerprints();
    }

    public static void showAuthDialog(Activity activity, Runnable onSuccess) {
        CustomAlertDialog dialog = new CustomAlertDialog(activity);
        CustomAlertDialog.ViewHolder vh = dialog.getViewHolder();
        try {
            FingerprintHelper helper = new FingerprintHelper() {
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
                    dialog.dismiss();
                    onSuccess.run();
                }
            };
            Drawable fingerprint = activity.getResources().getDrawable(R.drawable.ic_fingerprint);
            fingerprint.setBounds(0, 0, Utils.dpInPx(50), Utils.dpInPx(50));
            Resources.Theme theme = activity.getTheme();
            TypedArray ta = theme.obtainStyledAttributes(new int[] {R.attr.imageColorTint});
            fingerprint.setTint(ta.getColor(0, Color.GRAY));
            ta.recycle();
            vh.messageView.setCompoundDrawables(null, null, null, fingerprint);
            vh.messageView.setCompoundDrawablePadding(Utils.dpInPx(20));
            vh.messageView.setGravity(Gravity.CENTER);
            dialog.setMessage(R.string.auth_fingerprint)
                    .setNegativeButton(R.string.close, (d, w) -> helper.cancel())
                    .setOnCancelListener(d -> helper.cancel())
                    .show();
            helper.authenticate();
        } catch (Exception e) {
            e.printStackTrace();
            Utils.toast(R.string.auth_fail, Toast.LENGTH_SHORT);
        }

    }

    protected FingerprintHelper() throws Exception {
        KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
        manager = Data.MM().getSystemService(FingerprintManager.class);
        cipher = Cipher.getInstance(KeyProperties.KEY_ALGORITHM_AES + "/"
                + KeyProperties.BLOCK_MODE_CBC + "/"
                + KeyProperties.ENCRYPTION_PADDING_PKCS7);
        keyStore.load(null);
        SecretKey key = (SecretKey) keyStore.getKey(Const.SU_KEYSTORE_KEY, null);
        if (key == null) {
            key = generateKey();
        }
        try {
            cipher.init(Cipher.ENCRYPT_MODE, key);
        } catch (KeyPermanentlyInvalidatedException e) {
            // Only happens on Marshmallow
            key = generateKey();
            cipher.init(Cipher.ENCRYPT_MODE, key);
        }
    }

    public abstract void onAuthenticationError(int errorCode, CharSequence errString);

    public abstract void onAuthenticationHelp(int helpCode, CharSequence helpString);

    public abstract void onAuthenticationSucceeded(FingerprintManager.AuthenticationResult result);

    public abstract void onAuthenticationFailed();

    public void authenticate() {
        cancel = new CancellationSignal();
        FingerprintManager.CryptoObject cryptoObject = new FingerprintManager.CryptoObject(cipher);
        manager.authenticate(cryptoObject, cancel, 0, new Callback(), null);
    }

    public void cancel() {
        if (cancel != null)
            cancel.cancel();
    }

    private SecretKey generateKey() throws Exception {
        KeyGenerator keygen = KeyGenerator
                .getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore");
        KeyGenParameterSpec.Builder builder = new KeyGenParameterSpec.Builder(
                Const.SU_KEYSTORE_KEY,
                KeyProperties.PURPOSE_ENCRYPT | KeyProperties.PURPOSE_DECRYPT)
                .setBlockModes(KeyProperties.BLOCK_MODE_CBC)
                .setUserAuthenticationRequired(true)
                .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_PKCS7);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            builder.setInvalidatedByBiometricEnrollment(false);
        }
        keygen.init(builder.build());
        return keygen.generateKey();
    }

    private class Callback extends FingerprintManager.AuthenticationCallback {
        @Override
        public void onAuthenticationError(int errorCode, CharSequence errString) {
            FingerprintHelper.this.onAuthenticationError(errorCode, errString);
        }

        @Override
        public void onAuthenticationHelp(int helpCode, CharSequence helpString) {
            FingerprintHelper.this.onAuthenticationHelp(helpCode, helpString);
        }

        @Override
        public void onAuthenticationSucceeded(FingerprintManager.AuthenticationResult result) {
            FingerprintHelper.this.onAuthenticationSucceeded(result);
        }

        @Override
        public void onAuthenticationFailed() {
            FingerprintHelper.this.onAuthenticationFailed();
        }
    }
}
