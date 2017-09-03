package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.asyncs.FlashZip;
import com.topjohnwu.magisk.asyncs.InstallMagisk;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.AdaptiveList;
import com.topjohnwu.magisk.utils.Shell;

import java.util.List;

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
    @BindView(R.id.flash_logs) RecyclerView flashLogs;
    @BindView(R.id.button_panel) LinearLayout buttonPanel;
    @BindView(R.id.reboot) Button reboot;

    @OnClick(R.id.no_thanks)
    public void dismiss() {
        finish();
    }

    @OnClick(R.id.reboot)
    public void reboot() {
        getShell().su_raw("reboot");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_flash);
        ButterKnife.bind(this);
        AdaptiveList<String> rootShellOutput = new AdaptiveList<>(flashLogs);
        setSupportActionBar(toolbar);
        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.flashing);
        }
        setFloating();
        if (!Shell.rootAccess())
            reboot.setVisibility(View.GONE);

        flashLogs.setAdapter(new FlashLogAdapter(rootShellOutput));

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
                    getShell().su_raw(
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

    private static class FlashLogAdapter extends RecyclerView.Adapter<ViewHolder> {

        private List<String> mList;

        FlashLogAdapter(List<String> list) {
            mList = list;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.list_item_flashlog, parent, false);
            return new ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(ViewHolder holder, int position) {
            holder.text.setText(mList.get(position));
        }

        @Override
        public int getItemCount() {
            return mList.size();
        }
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.textView) TextView text;

        public ViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }
    }
}
