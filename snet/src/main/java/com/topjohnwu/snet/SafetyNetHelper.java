package com.topjohnwu.snet;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.util.Base64;
import android.util.Log;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.common.internal.ConnectionErrorMessages;
import com.google.android.gms.common.internal.DialogRedirect;
import com.google.android.gms.safetynet.SafetyNet;
import com.google.android.gms.safetynet.SafetyNetApi;
import com.google.android.gms.safetynet.SafetyNetClient;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.security.SecureRandom;

import androidx.annotation.NonNull;

public class SafetyNetHelper implements InvocationHandler,
        OnSuccessListener<SafetyNetApi.AttestationResponse>, OnFailureListener {

    private static final int RESPONSE_ERR = 0x01;
    private static final int CONNECTION_FAIL = 0x02;
    private static final int BASIC_PASS = 0x10;
    private static final int CTS_PASS = 0x20;

    private static final GoogleApiAvailability API_AVAIL = GoogleApiAvailability.getInstance();
    private static final SecureRandom RANDOM = new SecureRandom();
    private static final String TAG = "SNET";

    private final Activity mActivity;
    private final Object callback;

    SafetyNetHelper(Activity activity, Object cb) {
        mActivity = activity;
        callback = cb;
    }

    private void invokeCallback(int code) {
        Class<?> clazz = callback.getClass();
        try {
            clazz.getMethod("onResponse", int.class).invoke(callback, code);
        } catch (Exception ignored) {}
    }

    /* Return magic API key here :) */
    private String getApiKey() {
        return "";
    }

    /* Override ISafetyNetHelper.getVersion */
    private int getVersion() {
        return BuildConfig.VERSION_CODE;
    }

    /* Override ISafetyNetHelper.attest */
    private void attest() {
        int code = API_AVAIL.isGooglePlayServicesAvailable(mActivity);
        if (code != ConnectionResult.SUCCESS) {
            if (API_AVAIL.isUserResolvableError(code))
                getErrorDialog(code, 0).show();
            invokeCallback(CONNECTION_FAIL);
            return;
        }
        // Create nonce
        byte[] nonce = new byte[24];
        RANDOM.nextBytes(nonce);

        SafetyNetClient client = SafetyNet.getClient(mActivity.getBaseContext());
        client.attest(nonce, getApiKey()).addOnSuccessListener(this).addOnFailureListener(this);
    }

    private Dialog getErrorDialog(int errorCode, int requestCode) {
        AlertDialog.Builder builder = new AlertDialog.Builder(mActivity);
        Context swapCtx = new SwapResContext(mActivity, Snet.dexPath);
        Intent intent = API_AVAIL.getErrorResolutionIntent(swapCtx, errorCode, "d");

        builder.setMessage(ConnectionErrorMessages.getErrorMessage(swapCtx, errorCode));
        builder.setPositiveButton(
                ConnectionErrorMessages.getErrorDialogButtonMessage(swapCtx, errorCode),
                DialogRedirect.getInstance(mActivity, intent, requestCode));

        String title;
        if ((title = ConnectionErrorMessages.getErrorTitle(swapCtx, errorCode)) != null) {
            builder.setTitle(title);
        }

        return builder.create();
    }

    @Override
    public void onSuccess(SafetyNetApi.AttestationResponse result) {
        int code = 0;
        try {
            String jsonStr = new String(Base64.decode(
                    result.getJwsResult().split("\\.")[1], Base64.DEFAULT));
            JSONObject json = new JSONObject(jsonStr);
            code |= json.getBoolean("ctsProfileMatch") ? CTS_PASS : 0;
            code |= json.getBoolean("basicIntegrity") ? BASIC_PASS : 0;
        } catch (JSONException e) {
            code = RESPONSE_ERR;
        }

        // Return results
        invokeCallback(code);
    }

    @Override
    public void onFailure(@NonNull Exception e) {
        if (e instanceof ApiException) {
            int errCode = ((ApiException) e).getStatusCode();
            if (API_AVAIL.isUserResolvableError(errCode))
                getErrorDialog(errCode, 0).show();
            else
                Log.e(TAG, "Cannot resolve: " + e.getMessage());
        } else {
            Log.e(TAG, "Unknown: " + e.getMessage());
        }
        invokeCallback(CONNECTION_FAIL);
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) {
        switch (method.getName()) {
            case "attest":
                attest();
                return null;
            case "getVersion":
                return getVersion();
            default:
                return null;
        }
    }
}
