package com.topjohnwu.magisk;

import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;

import com.topjohnwu.magisk.asyncs.MarkDownWindow;
import com.topjohnwu.magisk.components.AboutCardRow;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.utils.Utils;

import java.util.Locale;

import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.widget.Toolbar;
import butterknife.BindView;

public class AboutActivity extends BaseActivity {

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.app_version_info) AboutCardRow appVersionInfo;
    @BindView(R.id.app_changelog) AboutCardRow appChangelog;
    @BindView(R.id.app_translators) AboutCardRow appTranslators;
    @BindView(R.id.app_source_code) AboutCardRow appSourceCode;
    @BindView(R.id.support_thread) AboutCardRow supportThread;
    @BindView(R.id.follow_twitter) AboutCardRow twitter;

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_StatusBar_Dark;
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_about);
        new AboutActivity_ViewBinding(this);

        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(view -> finish());

        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.about);
            ab.setDisplayHomeAsUpEnabled(true);
        }

        appVersionInfo.setSummary(String.format(Locale.US, "%s (%d) (%s)",
                BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE, getPackageName()));

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

        appSourceCode.setOnClickListener(v -> Utils.openLink(this, Uri.parse(Const.Url.SOURCE_CODE_URL)));
        supportThread.setOnClickListener(v -> Utils.openLink(this, Uri.parse(Const.Url.XDA_THREAD)));
        twitter.setOnClickListener(v -> Utils.openLink(this, Uri.parse(Const.Url.TWITTER_URL)));

        setFloating();
    }

}
