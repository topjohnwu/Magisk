package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;

import com.topjohnwu.magisk.asyncs.MarkDownWindow;
import com.topjohnwu.magisk.components.AboutCardRow;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.Const;

import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;

public class AboutActivity extends Activity {

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.app_version_info) AboutCardRow appVersionInfo;
    @BindView(R.id.app_changelog) AboutCardRow appChangelog;
    @BindView(R.id.app_translators) AboutCardRow appTranslators;
    @BindView(R.id.app_source_code) AboutCardRow appSourceCode;
    @BindView(R.id.support_thread) AboutCardRow supportThread;
    @BindView(R.id.donation) AboutCardRow donation;

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_StatusBar_Dark;
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_about);
        ButterKnife.bind(this);

        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(view -> finish());

        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.about);
            ab.setDisplayHomeAsUpEnabled(true);
        }

        appVersionInfo.setSummary(String.format(Locale.US, "%s (%d) (%s)",
                BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE, getPackageName()));

        appChangelog.removeSummary();
        appChangelog.setOnClickListener(v -> {
            new MarkDownWindow(this, getString(R.string.app_changelog),
                    getResources().openRawResource(R.raw.changelog)).exec();
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
