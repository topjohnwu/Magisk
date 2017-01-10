package com.topjohnwu.magisk.utils;

import android.app.AlertDialog;
import android.content.Context;
import android.webkit.WebResourceRequest;
import android.webkit.WebView;
import android.webkit.WebViewClient;

public class WebWindow {

public WebWindow(String title, String url, Context context) {
    AlertDialog.Builder alert = Utils.getAlertDialogBuilder(context);
    alert.setTitle(title);

    Logger.dev("WebView: URL = " + url);

    WebView wv = new WebView(context);
    wv.loadUrl(url);
    wv.setWebViewClient(new WebViewClient() {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
            view.loadUrl(url);
            return true;
        }
    });

    alert.setView(wv);
    alert.setNegativeButton("Close", (dialog, id) -> dialog.dismiss());
    alert.show();
}

}
