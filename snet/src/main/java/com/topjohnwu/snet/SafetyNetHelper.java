package com.topjohnwu.snet;

import android.content.Context;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Base64;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.ResultCallback;
import com.google.android.gms.common.api.Status;
import com.google.android.gms.safetynet.SafetyNet;
import com.google.android.gms.safetynet.SafetyNetApi;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.Method;
import java.security.SecureRandom;

public class SafetyNetHelper
        implements GoogleApiClient.ConnectionCallbacks, GoogleApiClient.OnConnectionFailedListener {

    public static final int CONNECTION_FAIL = -1;

    public static final int CAUSE_SERVICE_DISCONNECTED = 0x00001;
    public static final int CAUSE_NETWORK_LOST = 0x00010;
    public static final int RESPONSE_ERR = 0x00100;

    public static final int BASIC_PASS = 0x01000;
    public static final int CTS_PASS = 0x10000;

    private GoogleApiClient mGoogleApiClient;
    private Context mActivity;
    private int responseCode;
    private SafetyNetCallback cb;

    public SafetyNetHelper(Context context, SafetyNetCallback cb) {
        mActivity = context;
        this.cb = cb;
        responseCode = 0;
    }

    // Entry point to start test
    public void attest() {
        // Connect Google Service
        GoogleApiClient.Builder builder = new GoogleApiClient.Builder(mActivity);
        try {
            // Use reflection to workaround FragmentActivity crap
            Class<?> clazz = Class.forName("com.google.android.gms.common.api.GoogleApiClient$Builder");
            for (Method m : clazz.getMethods()) {
                if (m.getName().equals("enableAutoManage")) {
                    m.invoke(builder, mActivity, this);
                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        mGoogleApiClient = builder.addApi(SafetyNet.API).addConnectionCallbacks(this).build();
        mGoogleApiClient.connect();
    }

    @Override
    public void onConnectionSuspended(int i) {
        cb.onResponse(i);
    }

    @Override
    public void onConnectionFailed(@NonNull ConnectionResult connectionResult) {
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
                        cb.onResponse(RESPONSE_ERR);
                        return;
                    }
                    // Disconnect
                    try {
                        // Use reflection to workaround FragmentActivity crap
                        Class<?> clazz = Class.forName("com.google.android.gms.common.api.GoogleApiClient");
                        for (Method m : clazz.getMethods()) {
                            if (m.getName().equals("stopAutoManage")) {
                                m.invoke(mGoogleApiClient, mActivity, this);
                                break;
                            }
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    mGoogleApiClient.disconnect();

                    // Return results
                    cb.onResponse(responseCode);
                }
            });
    }
}
