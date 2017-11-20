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
import android.widget.Toast;

import com.topjohnwu.magisk.asyncs.FlashZip;
import com.topjohnwu.magisk.asyncs.InstallMagisk;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.container.CallbackList;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Shell;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class FlashActivity extends Activity {

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.txtLog) TextView flashLogs;
    @BindView(R.id.button_panel) public LinearLayout buttonPanel;
    @BindView(R.id.reboot) public Button reboot;
    @BindView(R.id.scrollView) ScrollView sv;

    private List<String> logs;

    @OnClick(R.id.no_thanks)
    void dismiss() {
        finish();
    }

    @OnClick(R.id.reboot)
    void reboot() {
        Shell.su_raw("/system/bin/reboot");
    }

    @OnClick(R.id.save_logs)
    void saveLogs() {
        Calendar now = Calendar.getInstance();
        String filename = String.format(Locale.US,
                "install_log_%04d%02d%02d_%02d:%02d:%02d.log",
                now.get(Calendar.YEAR), now.get(Calendar.MONTH) + 1,
                now.get(Calendar.DAY_OF_MONTH), now.get(Calendar.HOUR_OF_DAY),
                now.get(Calendar.MINUTE), now.get(Calendar.SECOND));

        File logFile = new File(Const.EXTERNAL_PATH + "/logs", filename);
        logFile.getParentFile().mkdirs();
        try (FileWriter writer = new FileWriter(logFile)) {
            for (String s : logs) {
                writer.write(s);
                writer.write('\n');
            }
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }
        MagiskManager.toast(logFile.getPath(), Toast.LENGTH_LONG);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_flash);
        ButterKnife.bind(this);
        setSupportActionBar(toolbar);
        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.flashing);
        }
        setFloating();
        setFinishOnTouchOutside(false);
        if (!Shell.rootAccess())
            reboot.setVisibility(View.GONE);

        logs = new ArrayList<>();
        List<String> console = new CallbackList<String>() {
            @Override
            public synchronized void onAddElement(String e) {
                logs.add(e);
                flashLogs.setText(TextUtils.join("\n", this));
                sv.postDelayed(() -> sv.fullScroll(ScrollView.FOCUS_DOWN), 10);
            }
        };

        // We must receive a Uri of the target zip
        Intent intent = getIntent();
        Uri uri = intent.getData();

        boolean keepEnc = intent.getBooleanExtra(Const.Key.FLASH_SET_ENC, false);
        boolean keepVerity = intent.getBooleanExtra(Const.Key.FLASH_SET_VERITY, false);

        switch (intent.getStringExtra(Const.Key.FLASH_ACTION)) {
            case Const.Value.FLASH_ZIP:
                new FlashZip(this, uri, console, logs).exec();
                break;
            case Const.Value.PATCH_BOOT:
                new InstallMagisk(this, console, logs, uri, keepEnc, keepVerity, (Uri) intent.getParcelableExtra(Const.Key.FLASH_SET_BOOT))
                        .exec();
                break;
            case Const.Value.FLASH_MAGISK:
                new InstallMagisk(this, console, logs, uri, keepEnc, keepVerity, intent.getStringExtra(Const.Key.FLASH_SET_BOOT))
                        .exec();
                break;
        }
    }

    @Override
    public void onBackPressed() {
        // Prevent user accidentally press back button
    }
}
