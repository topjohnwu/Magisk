package com.topjohnwu.magisk.uicomponents;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.topjohnwu.magisk.R;

import butterknife.BindView;
import butterknife.Unbinder;

public class UpdateCardHolder {

    @BindView(R.id.status_icon) public ImageView statusIcon;
    @BindView(R.id.progress) public ProgressBar progress;
    @BindView(R.id.status) public TextView status;
    @BindView(R.id.current_version) public TextView currentVersion;
    @BindView(R.id.latest_version) public TextView latestVersion;
    @BindView(R.id.additional) public TextView additional;
    @BindView(R.id.install) public Button install;

    public View itemView;
    public Unbinder unbinder;

    public UpdateCardHolder(LayoutInflater inflater, ViewGroup root) {
        itemView = inflater.inflate(R.layout.update_card, root, false);
        unbinder = new UpdateCardHolder_ViewBinding(this, itemView);
    }

    public void setClickable(View.OnClickListener listener) {
        itemView.setClickable(true);
        itemView.setFocusable(true);
        itemView.setOnClickListener(listener);
    }

    public void setValid(boolean valid) {
        progress.setVisibility(View.GONE);
        statusIcon.setVisibility(View.VISIBLE);
        if (valid) {
            install.setVisibility(View.VISIBLE);
            latestVersion.setVisibility(View.VISIBLE);
        } else {
            install.setVisibility(View.GONE);
            latestVersion.setVisibility(View.GONE);
        }
    }

    public void reset() {
        progress.setVisibility(View.VISIBLE);
        statusIcon.setVisibility(View.INVISIBLE);
        latestVersion.setVisibility(View.GONE);
        install.setVisibility(View.GONE);
        status.setText(R.string.checking_for_updates);
    }
}
