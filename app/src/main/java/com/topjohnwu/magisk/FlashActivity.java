package com.topjohnwu.magisk;

import android.net.Uri;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.asyncs.FlashZip;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.AdaptiveList;
import com.topjohnwu.magisk.utils.Shell;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class FlashActivity extends Activity {

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.flash_logs) RecyclerView flashLogs;
    @BindView(R.id.button_panel) LinearLayout buttonPanel;

    private AdaptiveList<String> rootShellOutput;

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
        rootShellOutput = new AdaptiveList<>(flashLogs);
        setSupportActionBar(toolbar);
        ActionBar ab = getSupportActionBar();
        if (ab != null) {
            ab.setTitle(R.string.flashing);
        }
        setFloating();

        flashLogs.setAdapter(new FlashLogAdapter());

        // We must receive a Uri of the target zip
        Uri uri = getIntent().getData();

        new FlashZip(this, uri, rootShellOutput)
                .setCallBack(() -> buttonPanel.setVisibility(View.VISIBLE))
                .exec();
    }

    @Override
    public void onBackPressed() {
        // Prevent user accidentally press back button
    }

    private class FlashLogAdapter extends RecyclerView.Adapter<ViewHolder> {

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.list_item_flashlog, parent, false);
            return new ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(ViewHolder holder, int position) {
            holder.text.setText(rootShellOutput.get(position));
        }

        @Override
        public int getItemCount() {
            return rootShellOutput.size();
        }
    }

    public class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.textView) TextView text;

        public ViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }
    }
}
