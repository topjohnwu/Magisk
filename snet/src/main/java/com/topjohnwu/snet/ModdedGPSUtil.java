package com.topjohnwu.snet;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.util.Log;

import com.google.android.gms.common.GooglePlayServicesUtil;
import com.google.android.gms.internal.zzlu;

/* Decompiled and modified from GooglePlayServiceUtil.class */
public class ModdedGPSUtil {

    private static final String TAG = "GooglePlayServicesUtil";
    static String dexPath;

    static Dialog getErrorDialog(int errCode, final Activity activity, final int requestCode) {
        SwapResContext ctx = new SwapResContext(activity, dexPath);
        Resources res = ctx.getResources();
        if (zzlu.zzQ(ctx) && errCode == 2) {
            errCode = 42;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(activity);

        builder.setMessage(GooglePlayServicesUtil.zze(ctx, errCode));

        String btnMsg = GooglePlayServicesUtil.zzf(ctx, errCode);
        if (btnMsg != null) {
            final Intent intent = GooglePlayServicesUtil.zzan(errCode);
            builder.setPositiveButton(btnMsg, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int i) {
                    PackageManager pm = activity.getPackageManager();
                    if (intent != null && intent.resolveActivity(pm) != null)
                        activity.startActivityForResult(intent, requestCode);
                    dialog.dismiss();
                }
            });
        }

        switch(errCode) {
            case 0:
                return null;
            case 1:
                return builder.setTitle(res.getString(com.google.android.gms.R.string.common_google_play_services_install_title)).create();
            case 2:
                return builder.setTitle(res.getString(com.google.android.gms.R.string.common_google_play_services_update_title)).create();
            case 3:
                return builder.setTitle(res.getString(com.google.android.gms.R.string.common_google_play_services_enable_title)).create();
            case 4:
            case 6:
                return builder.create();
            case 5:
                Log.e(TAG, "An invalid account was specified when connecting. Please provide a valid account.");
                return builder.setTitle(res.getString(com.google.android.gms.R.string.common_google_play_services_invalid_account_title)).create();
            case 7:
                Log.e(TAG, "Network error occurred. Please retry request later.");
                return builder.setTitle(res.getString(com.google.android.gms.R.string.common_google_play_services_network_error_title)).create();
            case 8:
                Log.e(TAG, "Internal error occurred. Please see logs for detailed information");
                return builder.create();
            case 9:
                Log.e(TAG, "Google Play services is invalid. Cannot recover.");
                return builder.setTitle(res.getString(com.google.android.gms.R.string.common_google_play_services_unsupported_title)).create();
            case 10:
                Log.e(TAG, "Developer error occurred. Please see logs for detailed information");
                return builder.create();
            case 11:
                Log.e(TAG, "The application is not licensed to the user.");
                return builder.create();
            case 12:
            case 13:
            case 14:
            case 15:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:
            case 40:
            case 41:
            default:
                Log.e(TAG, "Unexpected error code " + errCode);
                return builder.create();
            case 16:
                Log.e(TAG, "One of the API components you attempted to connect to is not available.");
                return builder.create();
            case 17:
                Log.e(TAG, "The specified account could not be signed in.");
                return builder.setTitle(res.getString(com.google.android.gms.R.string.common_google_play_services_sign_in_failed_title)).create();
            case 42:
                return builder.setTitle(res.getString(com.google.android.gms.R.string.common_android_wear_update_title)).create();
        }
    }

    public static class SwapResContext extends ContextWrapper {

        private AssetManager asset;
        private Resources resources;

        public SwapResContext(Context base, String apk) {
            super(base);
            try {
                asset = AssetManager.class.newInstance();
                AssetManager.class.getMethod("addAssetPath", String.class).invoke(asset, apk);
            } catch (Exception e) {
                e.printStackTrace();
            }
            Resources res = base.getResources();
            resources = new Resources(asset, res.getDisplayMetrics(), res.getConfiguration());
        }

        @Override
        public Resources getResources() {
            return resources;
        }

        @Override
        public AssetManager getAssets() {
            return asset;
        }
    }
}
