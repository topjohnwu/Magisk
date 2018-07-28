package com.topjohnwu.snet;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Base64;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GooglePlayServicesUtil;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.safetynet.SafetyNet;
import com.google.android.gms.safetynet.SafetyNetApi;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.security.SecureRandom;

public class SafetyNetHelper implements InvocationHandler, GoogleApiClient.ConnectionCallbacks,
        GoogleApiClient.OnConnectionFailedListener, ResultCallback<SafetyNetApi.AttestationResult> {

    public static final int CAUSE_SERVICE_DISCONNECTED = 0x01;
    public static final int CAUSE_NETWORK_LOST = 0x02;
    public static final int RESPONSE_ERR = 0x04;
    public static final int CONNECTION_FAIL = 0x08;

    public static final int BASIC_PASS = 0x10;
    public static final int CTS_PASS = 0x20;

    public static final int SNET_EXT_VER = 9;

    private GoogleApiClient mGoogleApiClient;
    private Activity mActivity;
    private Object callback;

    SafetyNetHelper(Activity activity, Object cb) {
        mActivity = activity;
        callback = cb;
    }

    /* Override ISafetyNetHelper.getVersion */
    private int getVersion() {
        return SNET_EXT_VER;
    }

    /* Override ISafetyNetHelper.attest */
    private void attest() {
        // Connect Google Service
        mGoogleApiClient = new GoogleApiClient.Builder(mActivity)
                .addApi(SafetyNet.API)
                .addOnConnectionFailedListener(this)
                .addConnectionCallbacks(this)
                .build();
        mGoogleApiClient.connect();
    }

    @Override
    public Object invoke(Object o, Method method, Object[] args) {
        if (method.getName().equals("attest")) {
            attest();
        } else if (method.getName().equals("getVersion")) {
            return getVersion();
        }
        return null;
    }

    private void invokeCallback(int code) {
        Class<?> clazz = callback.getClass();
        try {
            clazz.getMethod("onResponse", int.class).invoke(callback, code);
        } catch (Exception ignored) {}
    }

    @Override
    public void onConnectionSuspended(int i) {
        invokeCallback(i);
    }

    @Override
    public void onConnectionFailed(@NonNull ConnectionResult result) {
        if (GooglePlayServicesUtil.isUserRecoverableError(result.getErrorCode()))
            ModdedGPSUtil.getErrorDialog(result.getErrorCode(), mActivity, 0).show();
        invokeCallback(CONNECTION_FAIL);
    }

    @Override
    public void onConnected(@Nullable Bundle bundle) {
        // Create nonce
        byte[] nonce = new byte[24];
        new SecureRandom().nextBytes(nonce);

        // Call SafetyNet
        SafetyNet.SafetyNetApi.attest(mGoogleApiClient, nonce).setResultCallback(this);
    }

    @Override
    public void onResult(SafetyNetApi.AttestationResult result) {
        int code = 0;
        try {
            if (!result.getStatus().isSuccess())
                throw new JSONException("");
            String jsonStr = new String(Base64.decode(
                    result.getJwsResult().split("\\.")[1], Base64.DEFAULT));
            JSONObject json = new JSONObject(jsonStr);
            code |= json.getBoolean("ctsProfileMatch") ? CTS_PASS : 0;
            code |= json.getBoolean("basicIntegrity") ? BASIC_PASS : 0;
        } catch (JSONException e) {
            code = RESPONSE_ERR;
        }

        // Disconnect
        mGoogleApiClient.disconnect();

        // Return results
        invokeCallback(code);
    }
}
