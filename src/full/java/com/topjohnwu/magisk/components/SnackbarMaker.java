package com.topjohnwu.magisk.components;

import android.app.Activity;
import android.net.Uri;
import android.support.annotation.StringRes;
import android.support.design.widget.Snackbar;
import android.view.View;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

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
        TextView text = snack.getView().findViewById(android.support.design.R.id.snackbar_text);
        text.setMaxLines(Integer.MAX_VALUE);
    }

    public static void showUri(Activity activity, Uri uri) {
        make(activity, activity.getString(R.string.internal_storage,
                "/MagiskManager/" + Utils.getNameFromUri(activity, uri)),
                Snackbar.LENGTH_LONG)
                .setAction(R.string.ok, (v)->{}).show();
    }
}
