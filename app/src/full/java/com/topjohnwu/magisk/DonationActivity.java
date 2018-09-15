package com.topjohnwu.magisk;

import android.net.Uri;
import android.os.Bundle;

import com.topjohnwu.magisk.components.AboutCardRow;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.utils.Utils;

import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.widget.Toolbar;

public class DonationActivity extends BaseActivity {

    Toolbar toolbar;
    AboutCardRow paypal;
    AboutCardRow patreon;

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_StatusBar_Dark;
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_donation);
        ViewBinder.bind(this);

        setSupportActionBar(toolbar);
        toolbar.setNavigationOnClickListener(view -> finish());

        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.donation);
            ab.setDisplayHomeAsUpEnabled(true);
        }

        paypal.setOnClickListener(v -> Utils.openLink(this, Uri.parse(Const.Url.PAYPAL_URL)));
        patreon.setOnClickListener(v -> Utils.openLink(this, Uri.parse(Const.Url.PATREON_URL)));
    }
}
