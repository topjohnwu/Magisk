package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import com.topjohnwu.magisk.asyncs.FlashZip;
import com.topjohnwu.magisk.asyncs.InstallMagisk;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.container.AdaptiveList;
import com.topjohnwu.magisk.utils.Shell;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class FlashActivity extends Activity {

    public static final String SET_ACTION = "action";
    public static final String SET_BOOT = "boot";
    public static final String SET_ENC = "enc";
    public static final String SET_VERITY = "verity";

    public static final String FLASH_ZIP = "flash";
    public static final String PATCH_BOOT = "patch";
    public static final String FLASH_MAGISK = "magisk";

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.txtLog) TextView flashLogs;
    @BindView(R.id.button_panel) LinearLayout buttonPanel;
    @BindView(R.id.reboot) Button reboot;
    @BindView(R.id.scrollView) ScrollView sv;

    @OnClick(R.id.no_thanks)
    public void dismiss() {
        finish();
    }

    @OnClick(R.id.reboot)
    public void reboot() {
        Shell.su_raw("reboot");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_flash);
        ButterKnife.bind(this);
        AdaptiveList<String> rootShellOutput = new AdaptiveList<String>() {
            @Override
            public synchronized void updateView() {
                flashLogs.setText(TextUtils.join("\n", this));
                sv.postDelayed(() -> sv.fullScroll(ScrollView.FOCUS_DOWN), 10);
            }
        };
        setSupportActionBar(toolbar);
        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.flashing);
        }
        setFloating();
        setFinishOnTouchOutside(false);
        if (!Shell.rootAccess())
            reboot.setVisibility(View.GONE);

        // We must receive a Uri of the target zip
        Intent intent = getIntent();
        Uri uri = intent.getData();

        boolean keepEnc = intent.getBooleanExtra(SET_ENC, false);
        boolean keepVerity = intent.getBooleanExtra(SET_VERITY, false);

        switch (getIntent().getStringExtra(SET_ACTION)) {
            case FLASH_ZIP:
                new FlashZip(this, uri, rootShellOutput)
                        .setCallBack(() -> buttonPanel.setVisibility(View.VISIBLE))
                        .exec();
                break;
            case PATCH_BOOT:
                new InstallMagisk(this, rootShellOutput, uri, keepEnc, keepVerity, (Uri) intent.getParcelableExtra(SET_BOOT))
                        .setCallBack(() -> buttonPanel.setVisibility(View.VISIBLE))
                        .exec();
                break;
            case FLASH_MAGISK:
                String boot = intent.getStringExtra(SET_BOOT);
                if (getMagiskManager().remoteMagiskVersionCode < 1370) {
                    // Use legacy installation method
                    Shell.su_raw(
                            "echo \"BOOTIMAGE=" + boot + "\" > /dev/.magisk",
                            "echo \"KEEPFORCEENCRYPT=" + keepEnc + "\" >> /dev/.magisk",
                            "echo \"KEEPVERITY=" + keepVerity + "\" >> /dev/.magisk"
                    );
                    new FlashZip(this, uri, rootShellOutput)
                            .setCallBack(() -> buttonPanel.setVisibility(View.VISIBLE))
                            .exec();
                } else {
                    // Use new installation method
                    new InstallMagisk(this, rootShellOutput, uri, keepEnc, keepVerity, boot)
                            .setCallBack(() -> buttonPanel.setVisibility(View.VISIBLE))
                            .exec();
                }
                break;
        }
    }

    @Override
    public void onBackPressed() {
        // Prevent user accidentally press back button
    }
}
