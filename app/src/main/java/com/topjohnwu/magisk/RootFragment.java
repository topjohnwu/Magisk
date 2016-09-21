package com.topjohnwu.magisk;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.app.Fragment;
import android.support.v4.content.LocalBroadcastManager;
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

import com.topjohnwu.magisk.receivers.Receiver;
import com.topjohnwu.magisk.receivers.RootFragmentReceiver;
import com.topjohnwu.magisk.services.MonitorService;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class RootFragment extends Fragment implements Receiver{

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
    @BindColor(R.color.red500)
    int red500;
    @BindColor(R.color.green500)
    int green500;
    @BindColor(R.color.grey500)
    int grey500;
    @BindColor(R.color.accent)
    int accent;
    int statusOK = R.drawable.ic_check_circle;
    int statusAuto = R.drawable.ic_autoroot;
    int statusError = R.drawable.ic_error;
    int statusUnknown = R.drawable.ic_help;

    private boolean autoRootStatus;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.root_fragment, container, false);
        ButterKnife.bind(this, view);
        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        if (prefs.contains("autoRootEnable")) {
            autoRootStatus = prefs.getBoolean("autoRootEnable", false);
            rootToggle.setEnabled(false);
        } else {
            autoRootStatus = false;
            rootToggle.setEnabled(true);
        }
        autoRootToggle.setChecked(autoRootStatus);
        new updateUI().execute();

        rootToggle.setOnClickListener(toggle -> {
            Utils.toggleRoot(((CompoundButton) toggle).isChecked());
            new updateUI().execute();
        });

        autoRootToggle.setOnClickListener(toggle -> {
            ToggleAutoRoot(autoRootToggle.isChecked());
            new updateUI().execute();

        });

        selinuxToggle.setOnClickListener(toggle -> {
            Shell.su(((CompoundButton) toggle).isChecked() ? "setenforce 1" : "setenforce 0");
            new updateUI().execute();
        });

//        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(mYourBroadcastReceiver,
//                new IntentFilter("com.magisk.UPDATEUI"));

        return view;
    }

//    private final BroadcastReceiver mYourBroadcastReceiver = new RootFragmentReceiver(Re) {
//        @Override
//        public void onReceive(Context context, Intent intent) {
//
//            Log.d("Magisk", "RootFragment: UpdateRF called and fired");
//            new updateUI().execute();
//        }
//
//
//    };

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        // Check which request we're responding to
        Log.d("Magisk", "Got result: " + requestCode + " and " + resultCode);
        if (requestCode == 100) {
            // Make sure the request was successful
            if (resultCode == Activity.RESULT_OK) {
                Log.d("Magisk", "Got result code OK for permissions");

            } else {
                autoRootToggle.setEnabled(false);
                Toast.makeText(getActivity(), "Auto-root disabled, permissions required.", Toast.LENGTH_LONG).show();

            }

        } else if (requestCode == 420) {
            Log.d("Magisk", "Got result code OK for UI update.");
            new updateUI().execute();
        }
    }

    private void ToggleAutoRoot(boolean toggleState) {
        autoRootStatus = toggleState;
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("autoRootEnable", (toggleState));
        editor.apply();
        if (toggleState) {
            if (!Utils.hasServicePermission(getActivity())) {
                Intent intent = new Intent(android.provider.Settings.ACTION_ACCESSIBILITY_SETTINGS);
                startActivityForResult(intent, 100);
            }
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
        new updateUI().execute();
    }

    @Override
    public void onResult() {

    }

    public class updateUI extends AsyncTask<Void, Void, Void> {

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
                autoRootToggleView.setVisibility(View.VISIBLE);
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
                    if (autoRootStatus) {
                        rootStatusContainer.setBackgroundColor(green500);
                        rootStatusIcon.setImageResource(statusAuto);
                        rootStatusIcon.setColorFilter(Color.WHITE, PorterDuff.Mode.SRC_ATOP);
                        rootStatus.setTextColor(green500);
                        rootStatus.setText(R.string.root_auto_unmounted);
                        rootToggle.setEnabled(false);
                        safetyNetStatusIcon.setImageResource(statusOK);
                        safetyNetStatus.setText(R.string.root_auto_unmounted_info);
                        break;
                    } else {
                        rootToggle.setEnabled(true);
                        if (Utils.rootEnabled()) {
                            // Mounted
                            rootStatusContainer.setBackgroundColor(accent);
                            rootStatusIcon.setImageResource(statusError);
                            rootStatus.setTextColor(accent);
                            rootStatus.setText(R.string.root_enabled);
                            rootToggle.setChecked(true);
                            safetyNetStatusIcon.setImageResource(statusError);
                            safetyNetStatus.setText(R.string.root_enabled_info);
                            break;
                        } else {
                            // Disabled
                            rootStatusContainer.setBackgroundColor(green500);
                            rootStatusIcon.setImageResource(statusOK);
                            rootStatus.setTextColor(green500);
                            rootStatus.setText(R.string.root_disabled);
                            rootToggle.setChecked(false);
                            safetyNetStatusIcon.setImageResource(statusOK);
                            safetyNetStatus.setText(R.string.root_disabled_info);
                            break;
                        }
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
                    autoRootToggleView.setVisibility(View.GONE);
                    rootToggleView.setVisibility(View.GONE);
                    selinuxToggleView.setVisibility(View.GONE);
                    break;
            }
        }
    }

}
