package com.topjohnwu.magisk.utils;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.FragmentActivity;
import android.util.Base64;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.Status;
import com.google.android.gms.safetynet.SafetyNet;
import com.topjohnwu.magisk.R;

import org.json.JSONException;
import org.json.JSONObject;

import java.security.SecureRandom;

public abstract class SafetyNetHelper
        implements GoogleApiClient.OnConnectionFailedListener, GoogleApiClient.ConnectionCallbacks {

    private GoogleApiClient mGoogleApiClient;
    private Result ret;
    protected FragmentActivity mActivity;

    public SafetyNetHelper(FragmentActivity activity) {
        ret = new Result();
        mActivity = activity;
        mGoogleApiClient = new GoogleApiClient.Builder(activity)
                .enableAutoManage(activity, this)
                .addApi(SafetyNet.API)
                .addConnectionCallbacks(this)
                .build();
    }

    // Entry point to start test
    public void requestTest() {
        // Connect Google Service
        mGoogleApiClient.connect();
    }

    @Override
    public void onConnectionFailed(@NonNull ConnectionResult result) {
        Logger.dev("SN: Google API fail");
        ret.errmsg = result.getErrorMessage();
        handleResults(ret);
    }

    @Override
    public void onConnectionSuspended(int i) {
        Logger.dev("SN: Google API Suspended");
        switch (i) {
            case CAUSE_NETWORK_LOST:
                ret.errmsg = mActivity.getString(R.string.safetyNet_network_loss);
                break;
            case CAUSE_SERVICE_DISCONNECTED:
                ret.errmsg = mActivity.getString(R.string.safetyNet_service_disconnected);
                break;
        }
        handleResults(ret);
    }

    @Override
    public void onConnected(@Nullable Bundle bundle) {
        Logger.dev("SN: Google API Connected");
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
                            ret.ctsProfile = decoded.getBoolean("ctsProfileMatch");
                            ret.basicIntegrity = decoded.getBoolean("basicIntegrity");
                            ret.failed = false;
                        } catch (JSONException e) {
                            ret.errmsg = mActivity.getString(R.string.safetyNet_res_invalid);
                        }
                    } else {
                        Logger.dev("SN: No response");
                        ret.errmsg = mActivity.getString(R.string.safetyNet_no_response);
                    }
                    // Disconnect
                    mGoogleApiClient.stopAutoManage(mActivity);
                    mGoogleApiClient.disconnect();
                    handleResults(ret);
                });
    }

    // Callback function to save the results
    public abstract void handleResults(Result result);

    public static class Result {
        public boolean failed = true;
        public String errmsg;
        public boolean ctsProfile = false;
        public boolean basicIntegrity = false;
    }
}
