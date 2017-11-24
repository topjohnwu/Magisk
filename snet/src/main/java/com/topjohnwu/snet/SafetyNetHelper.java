package com.topjohnwu.snet;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Base64;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.common.api.Status;
import com.google.android.gms.safetynet.SafetyNet;
import com.google.android.gms.safetynet.SafetyNetApi;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.Field;
import java.security.SecureRandom;

public class SafetyNetHelper
        implements GoogleApiClient.ConnectionCallbacks, GoogleApiClient.OnConnectionFailedListener {

    public static final int CAUSE_SERVICE_DISCONNECTED = 0x01;
    public static final int CAUSE_NETWORK_LOST = 0x02;
    public static final int RESPONSE_ERR = 0x04;
    public static final int CONNECTION_FAIL = 0x08;

    public static final int BASIC_PASS = 0x10;
    public static final int CTS_PASS = 0x20;

    private GoogleApiClient mGoogleApiClient;
    private Activity mActivity;
    private int responseCode;
    private SafetyNetCallback cb;
    private String dexPath;
    private boolean isDarkTheme;

    public SafetyNetHelper(Activity activity, String dexPath, SafetyNetCallback cb) {
        mActivity = activity;
        this.cb = cb;
        this.dexPath = dexPath;
        responseCode = 0;

        // Get theme
        try {
            Context context = activity.getApplicationContext();
            Field theme = context.getClass().getField("isDarkTheme");
            isDarkTheme = (boolean) theme.get(context);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Entry point to start test
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
        cb.onResponse(i);
    }

    @Override
    public void onConnectionFailed(@NonNull ConnectionResult result) {
        Class<? extends Activity> clazz = mActivity.getClass();
        try {
            // Use external resources
            clazz.getMethod("swapResources", String.class, int.class).invoke(mActivity, dexPath,
                    isDarkTheme ? android.R.style.Theme_Material : android.R.style.Theme_Material_Light);
            try {
                GoogleApiAvailability.getInstance().getErrorDialog(mActivity, result.getErrorCode(), 0).show();
            } catch (Exception e) {
                e.printStackTrace();
            }
            clazz.getMethod("restoreResources").invoke(mActivity);
        } catch (Exception e) {
            e.printStackTrace();
        }
        cb.onResponse(CONNECTION_FAIL);
    }

    @Override
    public void onConnected(@Nullable Bundle bundle) {
        // Create nonce
        byte[] nonce = new byte[24];
        new SecureRandom().nextBytes(nonce);

        // Call SafetyNet
        SafetyNet.SafetyNetApi.attest(mGoogleApiClient, nonce)
            .setResultCallback(new ResultCallback<SafetyNetApi.AttestationResult>() {
                @Override
                public void onResult(@NonNull SafetyNetApi.AttestationResult result) {
                    Status status = result.getStatus();
                    try {
                        if (!status.isSuccess()) throw new JSONException("");
                        String json = new String(Base64.decode(
                                result.getJwsResult().split("\\.")[1], Base64.DEFAULT));
                        JSONObject decoded = new JSONObject(json);
                        responseCode |= decoded.getBoolean("ctsProfileMatch") ? CTS_PASS : 0;
                        responseCode |= decoded.getBoolean("basicIntegrity") ? BASIC_PASS : 0;
                    } catch (JSONException e) {
                        responseCode = RESPONSE_ERR;
                    }

                    // Disconnect
                    mGoogleApiClient.disconnect();

                    // Return results
                    cb.onResponse(responseCode);
                }
            });
    }
}
