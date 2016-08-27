package com.topjohnwu.magisk;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.Shell;

import java.io.File;
import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class RootFragment extends Fragment {

    @BindView(R.id.progressBar) ProgressBar progressBar;

    @BindView(R.id.rootSwitchView) View rootToggleView;
    @BindView(R.id.selinuxSwitchView) View selinuxToggleView;
    @BindView(R.id.rootStatusView) View rootStatusView;
    @BindView(R.id.safetynetStatusView) View safetynetStatusView;
    @BindView(R.id.selinuxStatusView) View selinuxStatusView;

    @BindView(R.id.root_toggle) Switch rootToggle;
    @BindView(R.id.selinux_toggle) Switch selinuxToggle;

    @BindView(R.id.root_status_container) View rootStatusContainer;
    @BindView(R.id.root_status_icon) ImageView rootStatusIcon;
    @BindView(R.id.root_status) TextView rootStatus;

    @BindView(R.id.selinux_status_container) View selinuxStatusContainer;
    @BindView(R.id.selinux_status_icon) ImageView selinuxStatusIcon;
    @BindView(R.id.selinux_status) TextView selinuxStatus;

    @BindView(R.id.safety_net_status) TextView safetyNetStatus;
    @BindView(R.id.safety_net_icon) ImageView safetyNetStatusIcon;

    @BindColor(R.color.red500) int red500;
    @BindColor(R.color.green500) int green500;
    @BindColor(R.color.grey500) int grey500;
    @BindColor(R.color.accent) int accent;

    int statusOK = R.drawable.ic_check_circle;
    int statusError = R.drawable.ic_error;
    int statusUnknown = R.drawable.ic_help;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.root_fragment, container, false);
        ButterKnife.bind(this, view);

        new updateUI().execute();

        rootToggle.setOnClickListener(toggle -> {
            Shell.su(((CompoundButton) toggle).isChecked() ? "setprop magisk.root 1" : "setprop magisk.root 0");
            new Handler().postDelayed(() -> new updateUI().execute(), 1000);
        });

        selinuxToggle.setOnClickListener(toggle -> {
            Shell.su(((CompoundButton) toggle).isChecked() ? "setenforce 1" : "setenforce 0");
            new Handler().postDelayed(() -> new updateUI().execute(), 1000);
        });

        return view;
    }

    private class updateUI extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            // Make sure static block invoked
            Shell.rootAccess();
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);

            progressBar.setVisibility(View.GONE);

            rootStatusView.setVisibility(View.VISIBLE);
            safetynetStatusView.setVisibility(View.VISIBLE);
            selinuxStatusView.setVisibility(View.VISIBLE);

            if (Shell.rootAccess()) {
                rootToggleView.setVisibility(View.VISIBLE);
                selinuxToggleView.setVisibility(View.VISIBLE);
            }

            List<String> selinux = Shell.sh("getenforce");

            if (selinux.isEmpty()) {
                selinuxStatusContainer.setBackgroundColor(grey500);
                selinuxStatusIcon.setImageResource(statusUnknown);

                selinuxStatus.setText(R.string.selinux_error_info);
                selinuxStatus.setTextColor(grey500);
                selinuxToggle.setChecked(false);
            } else if (selinux.get(0).equals("Enforcing")) {
                selinuxStatusContainer.setBackgroundColor(green500);
                selinuxStatusIcon.setImageResource(statusOK);

                selinuxStatus.setText(R.string.selinux_enforcing_info);
                selinuxStatus.setTextColor(green500);
                selinuxToggle.setChecked(true);
            } else {
                selinuxStatusContainer.setBackgroundColor(red500);
                selinuxStatusIcon.setImageResource(statusError);

                selinuxStatus.setText(R.string.selinux_permissive_info);
                selinuxStatus.setTextColor(red500);
                selinuxToggle.setChecked(false);
            }

            if (new File("/system/framework/twframework.jar").exists()) {
                selinuxStatus.append("\n" + getString(R.string.selinux_samsung_info));
            }

            switch (Shell.rootStatus) {
                case -1:
                    // Root Error
                    rootStatusContainer.setBackgroundColor(grey500);
                    rootStatusIcon.setImageResource(statusUnknown);
                    rootStatus.setTextColor(grey500);
                    rootStatus.setText(R.string.root_error);
                    rootToggle.setChecked(false);
                    safetyNetStatusIcon.setImageResource(statusUnknown);
                    safetyNetStatus.setText(R.string.root_error_info);
                    break;
                case 0:
                    // Not rooted
                    rootStatusContainer.setBackgroundColor(green500);
                    rootStatusIcon.setImageResource(statusOK);
                    rootStatus.setTextColor(green500);
                    rootStatus.setText(R.string.root_none);
                    rootToggle.setChecked(false);
                    safetyNetStatusIcon.setImageResource(statusOK);
                    safetyNetStatus.setText(R.string.root_none_info);
                    break;
                case 1:
                    // Proper root
                    if (new File("/system/xbin/su").exists()) {
                        // Mounted
                        rootStatusContainer.setBackgroundColor(accent);
                        rootStatusIcon.setImageResource(statusError);
                        rootStatus.setTextColor(accent);
                        rootStatus.setText(R.string.root_mounted);
                        rootToggle.setChecked(true);
                        safetyNetStatusIcon.setImageResource(statusError);
                        safetyNetStatus.setText(R.string.root_mounted_info);
                        break;
                    } else {
                        // Not Mounted
                        rootStatusContainer.setBackgroundColor(green500);
                        rootStatusIcon.setImageResource(statusOK);
                        rootStatus.setTextColor(green500);
                        rootStatus.setText(R.string.root_unmounted);
                        rootToggle.setChecked(false);
                        safetyNetStatusIcon.setImageResource(statusOK);
                        safetyNetStatus.setText(R.string.root_unmounted_info);
                        break;
                    }
                case 2:
                    // Improper root
                    rootStatusContainer.setBackgroundColor(red500);
                    rootStatusIcon.setImageResource(statusError);
                    rootStatus.setTextColor(red500);
                    rootStatus.setText(R.string.root_system);
                    rootToggle.setChecked(true);
                    safetyNetStatusIcon.setImageResource(statusError);
                    safetyNetStatus.setText(R.string.root_system_info);

                    rootToggleView.setVisibility(View.GONE);
                    selinuxToggleView.setVisibility(View.GONE);
                    break;
            }
        }
    }
}
