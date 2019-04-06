package com.topjohnwu.magisk.uicomponents;

import android.app.Activity;
import android.view.LayoutInflater;
import android.view.View;

import androidx.appcompat.app.AlertDialog;

import com.topjohnwu.magisk.R;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.ResponseListener;

import java.io.InputStream;
import java.util.Scanner;

import ru.noties.markwon.Markwon;
import ru.noties.markwon.html.HtmlPlugin;
import ru.noties.markwon.image.ImagesPlugin;
import ru.noties.markwon.image.svg.SvgPlugin;

public class MarkDownWindow {

    public static void show(Activity activity, String title, String url) {
        Networking.get(url).getAsString(new Listener(activity, title));
    }

    public static void show (Activity activity, String title, InputStream is) {
        try (Scanner s = new Scanner(is, "UTF-8")) {
            s.useDelimiter("\\A");
            new Listener(activity, title).onResponse(s.next());
        }
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
            Markwon markwon = Markwon.builder(activity)
                    .usePlugin(HtmlPlugin.create())
                    .usePlugin(ImagesPlugin.create(activity))
                    .usePlugin(SvgPlugin.create(activity.getResources()))
                    .build();
            AlertDialog.Builder alert = new AlertDialog.Builder(activity);
            alert.setTitle(title);
            View mv = LayoutInflater.from(activity).inflate(R.layout.markdown_window, null);
            markwon.setMarkdown(mv.findViewById(R.id.md_txt), md);
            alert.setView(mv);
            alert.setNegativeButton(R.string.close, (dialog, id) -> dialog.dismiss());
            alert.show();
        }
    }
}
