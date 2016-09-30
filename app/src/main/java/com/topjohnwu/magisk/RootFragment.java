package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class RootFragment extends Fragment {

    public SharedPreferences prefs;

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

    int statusOK = R.drawable.ic_check_circle;
    int statusError = R.drawable.ic_error;
    int statusUnknown = R.drawable.ic_help;

    @BindColor(R.color.green500) int colorOK;
    @BindColor(R.color.yellow500) int colorWarn;
    @BindColor(R.color.grey500) int colorNeutral;
    @BindColor(R.color.red500) int colorFail;

    private SharedPreferences.OnSharedPreferenceChangeListener listener;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.root_fragment, container, false);
        ButterKnife.bind(this, view);

        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        updateUI();

        rootToggle.setOnClickListener(toggle -> {
            new AsyncTask<Boolean, Void, Void>() {
                @Override
                protected Void doInBackground(Boolean... bools) {
                    Utils.toggleRoot(bools[0], getActivity());
                    return null;
                }
            }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, ((CompoundButton) toggle).isChecked());

        });

        selinuxToggle.setOnClickListener(toggle -> {

            new AsyncTask<Boolean, Void, Void>() {
                @Override
                protected Void doInBackground(Boolean... bools) {
                    Shell.su(bools[0] ? "setenforce 1" : "setenforce 0");
                    return null;
                }
                @Override
                protected void onPostExecute(Void v) {
                    super.onPostExecute(v);
                    updateUI();
                }
            }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, ((CompoundButton) toggle).isChecked());
        });

        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        listener = (pref, key) -> {
            if ((key.contains("autoRootEnable")) || (key.equals("root"))) {
                Logger.dev("RootFragmnet, keychange detected for " + key);
                updateUI();
            }

        };
        prefs.registerOnSharedPreferenceChangeListener(listener);
        updateUI();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        prefs.unregisterOnSharedPreferenceChangeListener(listener);
    }

    private void updateUI() {
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
            selinuxStatusContainer.setBackgroundColor(colorNeutral);
            selinuxStatusIcon.setImageResource(statusUnknown);

            selinuxStatus.setText(R.string.selinux_error_info);
            selinuxStatus.setTextColor(colorNeutral);
            selinuxToggle.setChecked(false);
        } else if (selinux.get(0).equals("Enforcing")) {
            selinuxStatusContainer.setBackgroundColor(colorOK);
            selinuxStatusIcon.setImageResource(statusOK);

            selinuxStatus.setText(R.string.selinux_enforcing_info);
            selinuxStatus.setTextColor(colorOK);
            selinuxToggle.setChecked(true);
        } else {
            selinuxStatusContainer.setBackgroundColor(colorFail);
            selinuxStatusIcon.setImageResource(statusError);

            selinuxStatus.setText(R.string.selinux_permissive_info);
            selinuxStatus.setTextColor(colorFail);
            selinuxToggle.setChecked(false);
        }

        if (new File("/system/framework/twframework.jar").exists()) {
            selinuxStatus.append("\n" + getString(R.string.selinux_samsung_info));
        }

        switch (Shell.rootStatus) {
            case -1:
                // Root Error
                rootStatusContainer.setBackgroundColor(colorFail);
                rootStatusIcon.setImageResource(statusUnknown);
                rootStatus.setTextColor(colorNeutral);
                rootStatus.setText(R.string.root_error);
                rootToggle.setChecked(false);
                safetyNetStatusIcon.setImageResource(statusUnknown);
                safetyNetStatus.setText(R.string.root_error_info);
                break;
            case 0:
                // Not rooted
                rootStatusContainer.setBackgroundColor(colorOK);
                rootStatusIcon.setImageResource(statusOK);
                rootStatus.setTextColor(colorOK);
                rootStatus.setText(R.string.root_none);
                rootToggle.setChecked(false);
                safetyNetStatusIcon.setImageResource(statusOK);
                safetyNetStatus.setText(R.string.root_none_info);
                break;
            case 1:
                // Proper root
                rootToggle.setEnabled(true);
                if (Utils.rootEnabled()) {
                    // Mounted
                    rootStatusContainer.setBackgroundColor(colorWarn);
                    rootStatusIcon.setImageResource(statusError);
                    rootStatus.setTextColor(colorWarn);
                    rootStatus.setText(R.string.root_enabled);
                    rootToggle.setChecked(true);
                    safetyNetStatusIcon.setImageResource(statusError);
                    safetyNetStatus.setText(R.string.root_enabled_info);
                    break;
                } else {
                    // Disabled
                    rootStatusContainer.setBackgroundColor(colorOK);
                    rootStatusIcon.setImageResource(statusOK);
                    rootStatus.setTextColor(colorOK);
                    rootStatus.setText(R.string.root_disabled);
                    rootToggle.setChecked(false);
                    safetyNetStatusIcon.setImageResource(statusOK);
                    safetyNetStatus.setText(R.string.root_disabled_info);
                    break;
                }
            case 2:
                // Improper root
                rootStatusContainer.setBackgroundColor(colorFail);
                rootStatusIcon.setImageResource(statusError);
                rootStatus.setTextColor(colorFail);
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
