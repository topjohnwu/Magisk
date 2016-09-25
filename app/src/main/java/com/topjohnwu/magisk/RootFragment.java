package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.topjohnwu.magisk.services.MonitorService;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class RootFragment extends Fragment {

    public SharedPreferences prefs;
    @BindView(R.id.progressBar)
    ProgressBar progressBar;
    @BindView(R.id.rootSwitchView)
    View rootToggleView;
    @BindView(R.id.autoRootSwitchView)
    View autoRootToggleView;
    @BindView(R.id.selinuxSwitchView)
    View selinuxToggleView;
    @BindView(R.id.rootStatusView)
    View rootStatusView;
    @BindView(R.id.safetynetStatusView)
    View safetynetStatusView;
    @BindView(R.id.selinuxStatusView)
    View selinuxStatusView;
    @BindView(R.id.root_toggle)
    Switch rootToggle;
    @BindView(R.id.auto_root_toggle)
    Switch autoRootToggle;
    @BindView(R.id.selinux_toggle)
    Switch selinuxToggle;
    @BindView(R.id.root_status_container)
    View rootStatusContainer;
    @BindView(R.id.root_status_icon)
    ImageView rootStatusIcon;
    @BindView(R.id.root_status)
    TextView rootStatus;
    @BindView(R.id.selinux_status_container)
    View selinuxStatusContainer;
    @BindView(R.id.selinux_status_icon)
    ImageView selinuxStatusIcon;
    @BindView(R.id.selinux_status)
    TextView selinuxStatus;
    @BindView(R.id.safety_net_status)
    TextView safetyNetStatus;
    @BindView(R.id.safety_net_icon)
    ImageView safetyNetStatusIcon;
    int statusOK = R.drawable.ic_check_circle;
    int statusAuto = R.drawable.ic_autoroot;
    int statusError = R.drawable.ic_error;
    int statusUnknown = R.drawable.ic_help;

    private int colorOK, colorFail, colorNeutral, colorWarn;
    private boolean autoRootStatus;
    private SharedPreferences.OnSharedPreferenceChangeListener listener;
    private View view;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.root_fragment, container, false);
        ButterKnife.bind(this, view);
        int[] attrs0 = {R.attr.ColorOK};
        int[] attrs1 = {R.attr.ColorFail};
        int[] attrs2 = {R.attr.ColorNeutral};
        int[] attrs3 = {R.attr.ColorWarn};
        TypedArray ta0 = getActivity().obtainStyledAttributes(attrs0);
        TypedArray ta1 = getActivity().obtainStyledAttributes(attrs1);
        TypedArray ta2 = getActivity().obtainStyledAttributes(attrs2);
        TypedArray ta3 = getActivity().obtainStyledAttributes(attrs3);
        colorOK = ta0.getColor(0, Color.GRAY);
        colorFail = ta1.getColor(0, Color.GRAY);
        colorNeutral = ta2.getColor(0, Color.GRAY);
        colorWarn = ta2.getColor(0, Color.GRAY);
        ta0.recycle();
        ta1.recycle();
        ta2.recycle();
        ta3.recycle();
        autoRootStatus = Utils.autoToggleEnabled(getActivity());

        if (autoRootStatus) {
            if (!Utils.hasServicePermission(getActivity())) {
                autoRootStatus = false;
            }
        }
        rootToggle.setEnabled(!autoRootStatus);
        autoRootToggle.setChecked(autoRootStatus);
        new updateUI().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);

        rootToggle.setOnClickListener(toggle -> Utils.toggleRoot(((CompoundButton) toggle).isChecked(), getActivity()));

        autoRootToggle.setOnClickListener(toggle -> {
                    if (!Utils.hasServicePermission(getActivity())) {
                        Intent intent = new Intent(android.provider.Settings.ACTION_ACCESSIBILITY_SETTINGS);
                        Toast.makeText(getActivity(), "Please enable accessibility access for Magisk's auto-toggle feature to work.", Toast.LENGTH_LONG).show();
                        startActivityForResult(intent, 100);
                    } else {
                        ToggleAutoRoot(autoRootToggle.isChecked());

                    }
                }

        );

        selinuxToggle.setOnClickListener(toggle -> {
            Shell.su(((CompoundButton) toggle).isChecked() ? "setenforce 1" : "setenforce 0");
            new updateUI().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        });

        return view;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (null != listener) {
            prefs.unregisterOnSharedPreferenceChangeListener(listener);
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        // Check which request we're responding to
        Log.d("Magisk", "Got result: " + requestCode + " and " + resultCode);
        if (requestCode == 100) {
            if (Utils.hasServicePermission(getActivity())) {
                ToggleAutoRoot(true);
                Snackbar.make(view, getActivity().getString(R.string.auto_toggle) + " has been enabled.", Snackbar.LENGTH_LONG).show();

            } else {
                autoRootToggle.setChecked(false);
                Snackbar.make(view, getActivity().getString(R.string.auto_toggle) + " disabled, permissions required.", Snackbar.LENGTH_LONG).show();
            }

        }
    }

    private void ToggleAutoRoot(boolean toggleState) {
        autoRootStatus = toggleState;
        Utils.toggleAutoRoot(toggleState, getActivity());
        if (toggleState) {
            Intent myIntent = new Intent(getActivity(), MonitorService.class);
            getActivity().startService(myIntent);
            rootToggle.setEnabled(false);
            boolean boo = Utils.isMyServiceRunning(MonitorService.class, getActivity());
            if (boo) {
                Intent myServiceIntent = new Intent(getActivity(), MonitorService.class);
                getActivity().startService(myServiceIntent);
            }
        } else {
            Intent myIntent = new Intent(getActivity(), MonitorService.class);
            getActivity().stopService(myIntent);
            rootToggle.setEnabled(true);
        }

    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle("Root");
        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        listener = (prefs1, key) -> {

            if ((key.contains("autoRootEnable")) | (key.equals("root"))) {
                Logger.dh("RootFragmnet, keychange detected for " + key);
                new updateUI().execute();
            }

        };

        prefs.registerOnSharedPreferenceChangeListener(listener);
        new updateUI().execute();

    }

    public class updateUI extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            // Make sure static block invoked
            Shell.rootAccess();
            // Set up Tile on UI Refresh
            if (PreferenceManager.getDefaultSharedPreferences(getActivity()).getBoolean("enable_quicktile", false)) {
                Utils.SetupQuickSettingsTile(getActivity());
            }
            autoRootStatus = Utils.autoToggleEnabled(getActivity());
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);
            autoRootToggle.setChecked(autoRootStatus);
            progressBar.setVisibility(View.GONE);
            rootStatusView.setVisibility(View.VISIBLE);
            safetynetStatusView.setVisibility(View.VISIBLE);
            selinuxStatusView.setVisibility(View.VISIBLE);

            if (Shell.rootAccess()) {
                rootToggleView.setVisibility(View.VISIBLE);
                autoRootToggleView.setVisibility(View.VISIBLE);
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
                    if (autoRootStatus) {
                        rootStatusContainer.setBackgroundColor(colorOK);
                        rootStatusIcon.setImageResource(statusAuto);
                        rootStatusIcon.setColorFilter(Color.WHITE, PorterDuff.Mode.SRC_ATOP);
                        rootStatus.setTextColor(colorOK);
                        rootStatus.setText(R.string.root_auto_unmounted);
                        rootToggle.setEnabled(false);
                        autoRootToggle.setChecked(true);
                        safetyNetStatusIcon.setImageResource(statusOK);
                        safetyNetStatus.setText(R.string.root_auto_unmounted_info);
                        break;
                    } else {
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
                    autoRootToggleView.setVisibility(View.GONE);
                    rootToggleView.setVisibility(View.GONE);
                    selinuxToggleView.setVisibility(View.GONE);
                    break;
            }
        }
    }

}
