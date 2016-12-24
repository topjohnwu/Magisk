package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Base64;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.Status;
import com.google.android.gms.safetynet.SafetyNet;

import org.json.JSONException;
import org.json.JSONObject;

import java.security.SecureRandom;

public abstract class SafetyNetHelper
        implements GoogleApiClient.OnConnectionFailedListener, GoogleApiClient.ConnectionCallbacks {

    private GoogleApiClient mGoogleApiClient;
    protected Context mContext;

    public SafetyNetHelper(Context context) {
        mContext = context;
        mGoogleApiClient = new GoogleApiClient.Builder(mContext)
                .addApi(SafetyNet.API)
                .addConnectionCallbacks(this)
                .addOnConnectionFailedListener(this)
                .build();
    }

    @Override
    public void onConnectionFailed(@NonNull ConnectionResult result) {
        Logger.dev("SN: Google API fail");
    }

    @Override
    public void onConnected(@Nullable Bundle bundle) {
        Logger.dev("SN: Google API Connected");
        safetyNetCheck();
    }

    @Override
    public void onConnectionSuspended(int i) {
        Logger.dev("SN: Google API Suspended");
    }

    public void requestTest() {
        // Connect Google Service
        mGoogleApiClient.connect();
    }

    private void safetyNetCheck() {
        // Create nonce
        byte[] nonce = new byte[24];
        new SecureRandom().nextBytes(nonce);

        Logger.dev("SN: Check with nonce: " + Base64.encodeToString(nonce, Base64.DEFAULT));

        // Call SafetyNet
        SafetyNet.SafetyNetApi.attest(mGoogleApiClient, nonce)
                .setResultCallback(result -> {
                    Status status = result.getStatus();
                    if (status.isSuccess()) {
                        String json = new String(Base64.decode(result.getJwsResult().split("\\.")[1], Base64.DEFAULT));
                        Logger.dev("SN: Response: " + json);
                        try {
                            JSONObject decoded = new JSONObject(json);
                            handleResults(decoded.getBoolean("ctsProfileMatch") ? 1 : 0);
                        } catch (JSONException ignored) {}
                    } else {
                        Logger.dev("SN: No response");
                        handleResults(-1);
                    }
                    // Disconnect
                    mGoogleApiClient.disconnect();
                });
    }

    public abstract void handleResults(int i);
}
