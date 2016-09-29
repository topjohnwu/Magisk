package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.Logger;

import java.io.IOException;
import java.io.InputStream;

import butterknife.BindView;
import butterknife.ButterKnife;

public class   AboutActivity extends AppCompatActivity {

    private static final String SOURCE_CODE_URL = "https://github.com/topjohnwu/MagiskManager";
    private static final String XDA_THREAD = "http://forum.xda-developers.com/showthread.php?t=3432382";
    private static final String DONATION_URL = "http://topjohnwu.github.io/donate";

    private AlertDialog.Builder builder;
    @BindView(R.id.toolbar) Toolbar toolbar;

    @BindView(R.id.app_version_info) RowItem appVersionInfo;
    @BindView(R.id.app_changelog) RowItem appChangelog;
    @BindView(R.id.app_developers) RowItem appDevelopers;
    @BindView(R.id.app_translators) RowItem appTranslators;
    @BindView(R.id.app_source_code) RowItem appSourceCode;
    @BindView(R.id.support_thread) RowItem supportThread;
    @BindView(R.id.donation) RowItem donation;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String theme = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getString("theme", "");
        Logger.dev("AboutActivity: Theme is " + theme);
        if (theme.equals("Dark")) {
            setTheme(R.style.AppTheme_dh);
        }
        setContentView(R.layout.activity_about);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
        }
        ButterKnife.bind(this);

        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(view -> finish());

        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.about);
            ab.setDisplayHomeAsUpEnabled(true);
        }

        appVersionInfo.setSummary(BuildConfig.VERSION_NAME);

        String changes = null;
        try {
            InputStream is = getAssets().open("changelog.html");
            int size = is.available();

            byte[] buffer = new byte[size];
            is.read(buffer);
            is.close();

            changes = new String(buffer);
        } catch (IOException ignored) {
        }

        appChangelog.removeSummary();
        if (theme.equals("Dark")) {
            builder = new AlertDialog.Builder(this,R.style.AlertDialog_dh);
        } else {
            builder = new AlertDialog.Builder(this);
        }
        if (changes == null) {
            appChangelog.setVisibility(View.GONE);
        } else {
            Spanned result;
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
                result = Html.fromHtml(changes,Html.TO_HTML_PARAGRAPH_LINES_CONSECUTIVE);
            } else {
                result = Html.fromHtml(changes);
            }
            appChangelog.setOnClickListener(v -> {
                AlertDialog d = builder
                        .setTitle(R.string.app_changelog)
                        .setMessage(result)
                        .setPositiveButton(android.R.string.ok, null)
                        .create();

                d.show();

                //noinspection ConstantConditions
                ((TextView) d.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
            });
        }

        appDevelopers.removeSummary();
        appDevelopers.setOnClickListener(view -> {
            Spanned result;
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
                result = Html.fromHtml(getString(R.string.app_developers_),Html.TO_HTML_PARAGRAPH_LINES_CONSECUTIVE);
            } else {
                result = Html.fromHtml(getString(R.string.app_developers_));
            }
            AlertDialog d = builder
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
        appSourceCode.setOnClickListener(view -> startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(SOURCE_CODE_URL))));

        supportThread.removeSummary();
        supportThread.setOnClickListener(view -> startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(XDA_THREAD))));

        donation.removeSummary();
        donation.setOnClickListener(view -> startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(DONATION_URL))));

        setFloating();
    }



    public void setFloating() {
        boolean isTablet = getResources().getBoolean(R.bool.isTablet);
        if (isTablet) {
            WindowManager.LayoutParams params = getWindow().getAttributes();
            params.height = getResources().getDimensionPixelSize(R.dimen.floating_height);
            params.width = getResources().getDimensionPixelSize(R.dimen.floating_width);
            params.alpha = 1.0f;
            params.dimAmount = 0.6f;
            params.flags |= 2;
            getWindow().setAttributes(params);
            setFinishOnTouchOutside(true);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

           setTitle("About");


            }
}
