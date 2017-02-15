package com.topjohnwu.magisk.components;

import android.content.Context;
import android.support.v7.app.AlertDialog;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;

import us.feras.mdv.MarkdownView;

public class MarkDownWindow {

    public MarkDownWindow(String title, String url, Context context) {
        AlertDialog.Builder alert = new AlertDialogBuilder(context);
        alert.setTitle(title);
    
        Logger.dev("WebView: URL = " + url);

        MarkdownView md = new MarkdownView(context);
        md.loadMarkdownFile(url);

        alert.setView(md);
        alert.setNegativeButton(R.string.close, (dialog, id) -> dialog.dismiss());
        alert.show();
    }

}
