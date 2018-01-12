package com.topjohnwu.magisk.utils;

import android.annotation.TargetApi;
import android.app.KeyguardManager;
import android.hardware.fingerprint.FingerprintManager;
import android.os.Build;
import android.os.CancellationSignal;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyPermanentlyInvalidatedException;
import android.security.keystore.KeyProperties;

import com.topjohnwu.magisk.MagiskManager;

import java.security.KeyStore;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;

@TargetApi(Build.VERSION_CODES.M)
public abstract class FingerprintHelper {

    private FingerprintManager manager;
    private Cipher cipher;
    private CancellationSignal cancel;

    public static boolean canUseFingerprint() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M)
            return false;
        MagiskManager mm = MagiskManager.get();
        KeyguardManager km = mm.getSystemService(KeyguardManager.class);
        FingerprintManager fm = mm.getSystemService(FingerprintManager.class);
        return km.isKeyguardSecure() && fm != null && fm.isHardwareDetected() && fm.hasEnrolledFingerprints();
    }

    protected FingerprintHelper() throws Exception {
        MagiskManager mm = MagiskManager.get();
        KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
        manager = mm.getSystemService(FingerprintManager.class);
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

    public void startAuth() {
        cancel = new CancellationSignal();
        FingerprintManager.CryptoObject cryptoObject = new FingerprintManager.CryptoObject(cipher);
        manager.authenticate(cryptoObject, cancel, 0, new FingerprintManager.AuthenticationCallback() {
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
        }, null);
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
}
