package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.net.Uri;
import android.view.View;
import android.widget.TextView;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

import androidx.annotation.StringRes;

public class SnackbarMaker {

    public static Snackbar make(Activity activity, CharSequence text, int duration) {
        View view = activity.findViewById(android.R.id.content);
        return make(view, text, duration);
    }

    public static Snackbar make(Activity activity, @StringRes int resId, int duration) {
        return make(activity, activity.getString(resId), duration);
    }

    public static Snackbar make(View view, CharSequence text, int duration) {
        Snackbar snack = Snackbar.make(view, text, duration);
        setup(snack);
        return snack;
    }

    public static Snackbar make(View view, @StringRes int resId, int duration) {
        Snackbar snack = Snackbar.make(view, resId, duration);
        setup(snack);
        return snack;
    }

    private static void setup(Snackbar snack) {
        TextView text = snack.getView().findViewById(com.google.android.material.R.id.snackbar_text);
        text.setMaxLines(Integer.MAX_VALUE);
    }

    public static void showUri(Activity activity, Uri uri) {
        make(activity, activity.getString(R.string.internal_storage,
                "/Download/" + Utils.getNameFromUri(activity, uri)),
                Snackbar.LENGTH_LONG)
                .setAction(R.string.ok, (v)->{}).show();
    }
}
