package com.topjohnwu.magisk.uicomponents;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.LayoutInflater;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;

import com.caverock.androidsvg.SVG;
import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.ResponseListener;
import com.topjohnwu.signing.ByteArrayStream;
import com.topjohnwu.superuser.ShellUtils;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.concurrent.Callable;

import ru.noties.markwon.Markwon;
import ru.noties.markwon.SpannableConfiguration;
import ru.noties.markwon.spans.AsyncDrawable;

public class MarkDownWindow {

    private static final SpannableConfiguration config = SpannableConfiguration.builder(App.self)
            .asyncDrawableLoader(new Loader()).build();

    public static void show(Activity activity, String title, String url) {
        Networking.get(url).getAsString(new Listener(activity, title));
    }

    public static void show (Activity activity, String title, InputStream is) {
        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            ShellUtils.pump(is, baos);
            new Listener(activity, title).onResponse(baos.toString());
        } catch (IOException ignored) {}
    }

    static class Listener implements ResponseListener<String> {

        Activity activity;
        String title;

        Listener(Activity a, String t) {
            activity = a;
            title = t;
        }

        @Override
        public void onResponse(String md) {
            AlertDialog.Builder alert = new AlertDialog.Builder(activity);
            alert.setTitle(title);
            View mv = LayoutInflater.from(activity).inflate(R.layout.markdown_window, null);
            Markwon.setMarkdown(mv.findViewById(R.id.md_txt), config, md);
            alert.setView(mv);
            alert.setNegativeButton(R.string.close, (dialog, id) -> dialog.dismiss());
            alert.show();
        }
    }

    static class Loader implements AsyncDrawable.Loader {

        @Override
        public void load(@NonNull String url, @NonNull AsyncDrawable asyncDrawable) {
            App.THREAD_POOL.submit((Callable<?>) () -> {
                InputStream is = Networking.get(url).execForInputStream().getResult();
                if (is == null)
                    return null;
                ByteArrayStream buf = new ByteArrayStream();
                buf.readFrom(is);
                // First try default drawables
                Drawable drawable = Drawable.createFromStream(buf.getInputStream(), "");
                if (drawable == null) {
                    // SVG
                    SVG svg = SVG.getFromInputStream(buf.getInputStream());
                    int width = Utils.dpInPx((int) svg.getDocumentWidth());
                    int height = Utils.dpInPx((int) svg.getDocumentHeight());
                    final Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_4444);
                    final Canvas canvas = new Canvas(bitmap);
                    float density = App.self.getResources().getDisplayMetrics().density;
                    canvas.scale(density, density);
                    svg.renderToCanvas(canvas);
                    drawable = new BitmapDrawable(App.self.getResources(), bitmap);
                }
                drawable.setBounds(0, 0, drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight());
                asyncDrawable.setResult(drawable);
                return null;
            });
        }

        @Override
        public void cancel(@NonNull String url) {}
    }
}
