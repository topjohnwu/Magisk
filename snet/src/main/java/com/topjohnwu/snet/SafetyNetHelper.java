package com.topjohnwu.snet;

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
import com.topjohnwu.magisk.asyncs.CheckSafetyNet;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.ISafetyNetHelper;

import org.json.JSONException;
import org.json.JSONObject;

import java.security.SecureRandom;

public class SafetyNetHelper implements ISafetyNetHelper, GoogleApiClient.ConnectionCallbacks,
        GoogleApiClient.OnConnectionFailedListener, ResultCallback<SafetyNetApi.AttestationResult> {

    public static final int CAUSE_SERVICE_DISCONNECTED = 0x01;
    public static final int CAUSE_NETWORK_LOST = 0x02;
    public static final int RESPONSE_ERR = 0x04;
    public static final int CONNECTION_FAIL = 0x08;

    public static final int BASIC_PASS = 0x10;
    public static final int CTS_PASS = 0x20;

    public static final int SNET_EXT_VER = 8;

    private GoogleApiClient mGoogleApiClient;
    private Activity mActivity;
    private Callback callback;

    @Override
    public int getVersion() {
        return SNET_EXT_VER;
    }

    public SafetyNetHelper(Activity activity, Callback cb) {
        mActivity = activity;
        callback = cb;
    }

    // Entry point to start test
    @Override
    public void attest() {
        // Connect Google Service
        mGoogleApiClient = new GoogleApiClient.Builder(mActivity)
                .addApi(SafetyNet.API)
                .addOnConnectionFailedListener(this)
                .addConnectionCallbacks(this)
                .build();
        mGoogleApiClient.connect();
    }

    @Override
    public void onConnectionSuspended(int i) {
        callback.onResponse(i);
    }

    @Override
    public void onConnectionFailed(@NonNull ConnectionResult result) {
        mActivity.swapResources(CheckSafetyNet.dexPath.getPath());
        GooglePlayServicesUtil.getErrorDialog(result.getErrorCode(), mActivity, 0).show();
        mActivity.restoreResources();
        callback.onResponse(CONNECTION_FAIL);
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
        callback.onResponse(code);
    }
}
