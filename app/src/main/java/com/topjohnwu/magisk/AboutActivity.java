package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.text.TextUtils;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.RowItem;

import java.io.IOException;
import java.io.InputStream;

import butterknife.BindView;
import butterknife.ButterKnife;

public class AboutActivity extends AppCompatActivity {

    private static final String SOURCE_CODE_URL = "https://github.com/topjohnwu/MagiskManager";
    private static final String XDA_THREAD = "http://forum.xda-developers.com/android/software/mod-magisk-v1-universal-systemless-t3432382";

    @BindView(R.id.toolbar) Toolbar toolbar;

    @BindView(R.id.app_version_info) RowItem appVersionInfo;
    @BindView(R.id.app_changelog) RowItem appChangelog;
    @BindView(R.id.app_developers) RowItem appDevelopers;
    @BindView(R.id.app_translators) RowItem appTranslators;
    @BindView(R.id.app_source_code) RowItem appSourceCode;
    @BindView(R.id.support_thread) RowItem supportThread;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_about);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
        }
        ButterKnife.bind(this);

        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                finish();
            }
        });

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
        if (changes == null) {
            appChangelog.setVisibility(View.GONE);
        } else {
            final String finalChanges = changes;
            appChangelog.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    AlertDialog d = new AlertDialog.Builder(AboutActivity.this)
                            .setTitle(R.string.app_changelog)
                            .setMessage(Html.fromHtml(finalChanges))
                            .setPositiveButton(android.R.string.ok, null)
                            .create();

                    d.show();

                    //noinspection ConstantConditions
                    ((TextView) d.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
                }
            });
        }

        appDevelopers.removeSummary();
        appDevelopers.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                AlertDialog d = new AlertDialog.Builder(AboutActivity.this)
                        .setTitle(R.string.app_developers)
                        .setMessage(Html.fromHtml(getString(R.string.app_developers_)))
                        .setPositiveButton(android.R.string.ok, null)
                        .create();

                d.show();
                //noinspection ConstantConditions
                ((TextView) d.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
            }
        });

        String translators = getString(R.string.translators);
        if (TextUtils.isEmpty(translators)) {
            appTranslators.setVisibility(View.GONE);
        } else {
            appTranslators.setSummary(translators);
        }

        appSourceCode.removeSummary();
        appSourceCode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(SOURCE_CODE_URL)));
            }
        });

        supportThread.removeSummary();
        supportThread.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(XDA_THREAD)));
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();

        getWindow().setStatusBarColor(getResources().getColor(R.color.primary_dark));
    }
}
