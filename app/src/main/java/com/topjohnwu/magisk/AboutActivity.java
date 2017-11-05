package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AlertDialog;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.widget.TextView;

import com.topjohnwu.magisk.components.AboutCardRow;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.utils.Const;

import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;

public class AboutActivity extends Activity {

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.app_version_info) AboutCardRow appVersionInfo;
    @BindView(R.id.app_changelog) AboutCardRow appChangelog;
    @BindView(R.id.app_developers) AboutCardRow appDevelopers;
    @BindView(R.id.app_translators) AboutCardRow appTranslators;
    @BindView(R.id.app_source_code) AboutCardRow appSourceCode;
    @BindView(R.id.support_thread) AboutCardRow supportThread;
    @BindView(R.id.donation) AboutCardRow donation;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getMagiskManager().isDarkTheme) {
            setTheme(R.style.AppTheme_Transparent_Dark);
        }
        setContentView(R.layout.activity_about);
        ButterKnife.bind(this);

        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(view -> finish());

        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.about);
            ab.setDisplayHomeAsUpEnabled(true);
        }

        appVersionInfo.setSummary(String.format(Locale.US, "%s (%d)", BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE));

        String changes = null;
        try (InputStream is = getAssets().open("changelog.html")) {
            int size = is.available();

            byte[] buffer = new byte[size];
            is.read(buffer);

            changes = new String(buffer);
        } catch (IOException ignored) {
        }

        appChangelog.removeSummary();
        if (changes == null) {
            appChangelog.setVisibility(View.GONE);
        } else {
            Spanned result;
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
                result = Html.fromHtml(changes, Html.TO_HTML_PARAGRAPH_LINES_CONSECUTIVE);
            } else {
                result = Html.fromHtml(changes);
            }
            appChangelog.setOnClickListener(v -> {
                AlertDialog d = new AlertDialogBuilder(this)
                        .setTitle(R.string.app_changelog)
                        .setMessage(result)
                        .setPositiveButton(android.R.string.ok, null)
                        .show();

                //noinspection ConstantConditions
                ((TextView) d.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
            });
        }

        appDevelopers.removeSummary();
        appDevelopers.setOnClickListener(view -> {
            Spanned result;
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
                result = Html.fromHtml(getString(R.string.app_developers_), Html.TO_HTML_PARAGRAPH_LINES_CONSECUTIVE);
            } else {
                result = Html.fromHtml(getString(R.string.app_developers_));
            }
            AlertDialog d = new AlertDialogBuilder(this)
                    .setTitle(R.string.app_developers)
                    .setMessage(result)
                    .setPositiveButton(android.R.string.ok, null)
                    .create();

            d.show();
            //noinspection ConstantConditions
            ((TextView) d.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
        });

        String translators = getString(R.string.translators);
        if (TextUtils.isEmpty(translators)) {
            appTranslators.setVisibility(View.GONE);
        } else {
            appTranslators.setSummary(translators);
        }

        appSourceCode.removeSummary();
        appSourceCode.setOnClickListener(view -> startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(Const.Url.SOURCE_CODE_URL))));

        supportThread.removeSummary();
        supportThread.setOnClickListener(view -> startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(Const.Url.XDA_THREAD))));

        donation.removeSummary();
        donation.setOnClickListener(view -> startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(Const.Url.DONATION_URL))));

        setFloating();
    }

}
