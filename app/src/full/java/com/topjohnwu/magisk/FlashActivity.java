package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.tasks.FlashZip;
import com.topjohnwu.core.tasks.MagiskInstaller;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.superuser.CallbackList;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.widget.Toolbar;
import butterknife.BindView;
import butterknife.OnClick;

public class FlashActivity extends BaseActivity {

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.txtLog) TextView flashLogs;
    @BindView(R.id.button_panel) public LinearLayout buttonPanel;
    @BindView(R.id.reboot) public Button reboot;
    @BindView(R.id.scrollView) ScrollView sv;

    private List<String> console, logs;

    @OnClick(R.id.reboot)
    void reboot() {
        Shell.su("/system/bin/reboot").submit();
    }

    @OnClick(R.id.save_logs)
    void saveLogs() {
        runWithExternalRW(() -> {
            Calendar now = Calendar.getInstance();
            String filename = String.format(Locale.US,
                    "magisk_install_log_%04d%02d%02d_%02d%02d%02d.log",
                    now.get(Calendar.YEAR), now.get(Calendar.MONTH) + 1,
                    now.get(Calendar.DAY_OF_MONTH), now.get(Calendar.HOUR_OF_DAY),
                    now.get(Calendar.MINUTE), now.get(Calendar.SECOND));

            File logFile = new File(Const.EXTERNAL_PATH, filename);
            try (FileWriter writer = new FileWriter(logFile)) {
                for (String s : logs) {
                    writer.write(s);
                    writer.write('\n');
                }
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }
            Utils.toast(logFile.getPath(), Toast.LENGTH_LONG);
        });
    }

    @OnClick(R.id.close)
    @Override
    public void finish() {
        super.finish();
    }

    @Override
    public void onBackPressed() {
        // Prevent user accidentally press back button
    }

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_StatusBar_Dark;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_flash);
        new FlashActivity_ViewBinding(this);

        setSupportActionBar(toolbar);
        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.flashing);
        }
        setFloating();
        setFinishOnTouchOutside(false);
        if (!Shell.rootAccess())
            reboot.setVisibility(View.GONE);

        logs = Collections.synchronizedList(new ArrayList<>());
        console = new ConsoleList();

        Intent intent = getIntent();
        Uri uri = intent.getData();

        switch (intent.getStringExtra(Const.Key.FLASH_ACTION)) {
            case Const.Value.FLASH_ZIP:
                new FlashModule(uri).exec();
                break;
            case Const.Value.UNINSTALL:
                new Uninstall(uri).exec();
                break;
            case Const.Value.FLASH_MAGISK:
                new DirectInstall().exec();
                break;
            case Const.Value.FLASH_INACTIVE_SLOT:
                new SecondSlot().exec();
                break;
            case Const.Value.PATCH_BOOT:
                new PatchBoot(uri).exec();
                break;
        }
    }

    private class ConsoleList extends CallbackList<String> {

        ConsoleList() {
            super(new ArrayList<>());
        }

        private void updateUI() {
            flashLogs.setText(TextUtils.join("\n", this));
            sv.postDelayed(() -> sv.fullScroll(ScrollView.FOCUS_DOWN), 10);
        }

        @Override
        public void onAddElement(String s) {
            logs.add(s);
            updateUI();
        }

        @Override
        public String set(int i, String s) {
            String ret = super.set(i, s);
            App.mainHandler.post(this::updateUI);
            return ret;
        }
    }

    private class FlashModule extends FlashZip {

        FlashModule(Uri uri) {
            super(uri, console, logs);
        }

        @Override
        protected void onResult(boolean success) {
            if (success) {
                Utils.loadModules();
            } else {
                console.add("! Installation failed");
                reboot.setVisibility(View.GONE);
            }
            buttonPanel.setVisibility(View.VISIBLE);
        }
    }

    private class Uninstall extends FlashModule {

        Uninstall(Uri uri) {
            super(uri);
        }

        @Override
        protected void onResult(boolean success) {
            if (success)
                App.mainHandler.postDelayed(Shell.su("pm uninstall " + getPackageName())::exec, 3000);
            else
                super.onResult(false);
        }
    }

    private abstract class BaseInstaller extends MagiskInstaller {
        BaseInstaller() {
            super(console, logs);
        }

        @Override
        protected void onResult(boolean success) {
            if (success) {
                console.add("- All done!");
            } else {
                Shell.sh("rm -rf " + installDir).submit();
                console.add("! Installation failed");
                reboot.setVisibility(View.GONE);
            }
            buttonPanel.setVisibility(View.VISIBLE);
        }
    }

    private class DirectInstall extends BaseInstaller {

        @Override
        protected boolean operations() {
            console.add("- Detecting target image");
            srcBoot = ShellUtils.fastCmd("find_boot_image", "echo \"$BOOTIMAGE\"");
            if (srcBoot.isEmpty()) {
                console.add("! Unable to detect target image");
                return false;
            }
            return extractZip() && patchBoot() && flashBoot();
        }
    }

    private class SecondSlot extends BaseInstaller {

        @Override
        protected boolean operations() {
            String slot = ShellUtils.fastCmd("echo $SLOT");
            String target = (TextUtils.equals(slot, "_a") ? "_b" : "_a");
            console.add("- Target slot: " + target);
            console.add("- Detecting target image");
            srcBoot = ShellUtils.fastCmd(
                    "SLOT=" + target,
                    "find_boot_image",
                    "SLOT=" + slot,
                    "echo \"$BOOTIMAGE\""
            );
            if (srcBoot.isEmpty()) {
                console.add("! Unable to detect target image");
                return false;
            }
            return extractZip() && patchBoot() && flashBoot() && postOTA();
        }
    }

    private class PatchBoot extends BaseInstaller {

        private Uri uri;

        PatchBoot(Uri u) {
            uri = u;
        }

        @Override
        protected boolean operations() {
            return copyBoot(uri) && extractZip() && patchBoot() && storeBoot();
        }
    }

}
