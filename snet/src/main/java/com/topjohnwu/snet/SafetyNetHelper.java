package com.topjohnwu.snet;

import android.content.Context;
import android.util.Base64;
import android.util.Log;

import androidx.annotation.NonNull;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.safetynet.SafetyNet;
import com.google.android.gms.safetynet.SafetyNetApi;
import com.google.android.gms.safetynet.SafetyNetClient;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.security.SecureRandom;

public class SafetyNetHelper implements InvocationHandler,
        OnSuccessListener<SafetyNetApi.AttestationResponse>, OnFailureListener {

    private static final int RESPONSE_ERR = 0x01;
    private static final int CONNECTION_FAIL = 0x02;
    private static final int BASIC_PASS = 0x10;
    private static final int CTS_PASS = 0x20;

    private static final GoogleApiAvailability API_AVAIL = GoogleApiAvailability.getInstance();
    private static final SecureRandom RANDOM = new SecureRandom();
    private static final String TAG = "SNET";

    private final Context context;
    private final Object callback;

    public static Object get(Class<?> interfaceClass, Context context, Object cb) {
        return Proxy.newProxyInstance(SafetyNetHelper.class.getClassLoader(),
                new Class[]{interfaceClass}, new SafetyNetHelper(context, cb));
    }

    private SafetyNetHelper(Context c, Object cb) {
        context = c;
        callback = cb;
    }

    private void invokeCallback(int code) {
        Class<?> clazz = callback.getClass();
        try {
            clazz.getMethod("onResponse", int.class).invoke(callback, code);
        } catch (Exception ignored) {
        }
    }

    /* Return magic API key here :) */
    private String getApiKey() {
        return "";
    }

    private int getVersion() {
        return BuildConfig.VERSION_CODE;
    }

    private void attest() {
        int code = API_AVAIL.isGooglePlayServicesAvailable(context);
        if (code != ConnectionResult.SUCCESS) {
            Log.e(TAG, API_AVAIL.getErrorString(code));
            invokeCallback(CONNECTION_FAIL);
            return;
        }
        // Create nonce
        byte[] nonce = new byte[24];
        RANDOM.nextBytes(nonce);

        SafetyNetClient client = SafetyNet.getClient(context);
        client.attest(nonce, getApiKey()).addOnSuccessListener(this).addOnFailureListener(this);
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
            Log.e(TAG, API_AVAIL.getErrorString(errCode));
        } else {
            Log.e(TAG, "Unknown: " + e);
        }
        invokeCallback(CONNECTION_FAIL);
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) {
        switch (method.getName()) {
            case "attest":
                attest();
                break;
            case "getVersion":
                return getVersion();
        }
        return null;
    }
}
