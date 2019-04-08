package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.widget.Toolbar;
import androidx.recyclerview.widget.RecyclerView;

import com.topjohnwu.magisk.adapters.StringListAdapter;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.tasks.FlashZip;
import com.topjohnwu.magisk.tasks.MagiskInstaller;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.CallbackList;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.internal.UiThreadHandler;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.OnClick;

public class FlashActivity extends BaseActivity {

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.button_panel) LinearLayout buttonPanel;
    @BindView(R.id.reboot) Button reboot;
    @BindView(R.id.recyclerView) RecyclerView rv;
    @BindColor(android.R.color.white) int white;

    private List<String> console, logs;

    @OnClick(R.id.reboot)
    void reboot() {
        RootUtils.reboot();
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
    public void close() {
        finish();
    }

    @Override
    public void onBackPressed() {
        // Prevent user accidentally press back button
    }

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_NoDrawer_Dark;
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
        rv.setAdapter(new ConsoleAdapter());

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
            case Const.Value.PATCH_FILE:
                new PatchFile(uri).exec();
                break;
        }
    }

    private class ConsoleAdapter extends StringListAdapter<ConsoleAdapter.ViewHolder> {

        ConsoleAdapter() {
            super(console, true);
        }

        @Override
        protected int itemLayoutRes() {
            return R.layout.list_item_console;
        }

        @NonNull
        @Override
        public ViewHolder createViewHolder(@NonNull View v) {
            return new ViewHolder(v);
        }

        class ViewHolder extends StringListAdapter.ViewHolder {

            public ViewHolder(@NonNull View itemView) {
                super(itemView);
                txt.setTextColor(white);
            }

            @Override
            protected int textViewResId() {
                return R.id.txt;
            }
        }
    }

    private class ConsoleList extends CallbackList<String> {

        ConsoleList() {
            super(new ArrayList<>());
        }

        private void updateUI() {
            rv.getAdapter().notifyItemChanged(size() - 1);
            rv.postDelayed(() -> rv.smoothScrollToPosition(size() - 1), 10);
        }

        @Override
        public void onAddElement(String s) {
            logs.add(s);
            updateUI();
        }

        @Override
        public String set(int i, String s) {
            String ret = super.set(i, s);
            UiThreadHandler.run(this::updateUI);
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
                UiThreadHandler.handler.postDelayed(Shell.su("pm uninstall " + getPackageName())::exec, 3000);
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
            return findImage() && extractZip() && patchBoot() && flashBoot();
        }
    }

    private class SecondSlot extends BaseInstaller {

        @Override
        protected boolean operations() {
            return findSecondaryImage() && extractZip() && patchBoot() && flashBoot() && postOTA();
        }
    }

    private class PatchFile extends BaseInstaller {

        private Uri uri;

        PatchFile(Uri u) {
            uri = u;
        }

        @Override
        protected boolean operations() {
            return extractZip() && handleFile(uri) && patchBoot() && storeBoot();
        }
    }

}
