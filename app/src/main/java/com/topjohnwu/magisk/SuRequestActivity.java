package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.hardware.fingerprint.FingerprintManager;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.text.TextUtils;
import android.view.View;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.content.res.AppCompatResources;

import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.utils.FingerprintHelper;
import com.topjohnwu.magisk.utils.SuConnector;

import java.io.IOException;

import butterknife.BindView;

public class SuRequestActivity extends BaseActivity {
    @BindView(R.id.su_popup) LinearLayout suPopup;
    @BindView(R.id.timeout) Spinner timeout;
    @BindView(R.id.app_icon) ImageView appIcon;
    @BindView(R.id.app_name) TextView appNameView;
    @BindView(R.id.package_name) TextView packageNameView;
    @BindView(R.id.grant_btn) Button grant_btn;
    @BindView(R.id.deny_btn) Button deny_btn;
    @BindView(R.id.fingerprint) ImageView fingerprintImg;
    @BindView(R.id.warning) TextView warning;

    private SuConnector connector;
    private Policy policy;
    private CountDownTimer timer;
    private FingerprintHelper fingerprintHelper;
    private SharedPreferences timeoutPrefs;

    @Override
    public int getDarkTheme() {
        return R.style.SuRequest_Dark;
    }

    @Override
    public void finish() {
        if (timer != null)
            timer.cancel();
        if (fingerprintHelper != null)
            fingerprintHelper.cancel();
        super.finish();
    }

    @Override
    public void onBackPressed() {
        if (policy != null) {
            handleAction(Policy.DENY);
        } else {
            finish();
        }
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        lockOrientation();
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);

        PackageManager pm = getPackageManager();
        app.mDB.clearOutdated();
        timeoutPrefs = App.deContext.getSharedPreferences("su_timeout", 0);

        // Get policy
        Intent intent = getIntent();
        try {
            String socketName = intent.getStringExtra("socket");
            connector = new SuConnector(socketName) {
                @Override
                protected void onResponse() throws IOException {
                    out.writeInt(policy.policy);
                }
            };
            Bundle bundle = connector.readSocketInput();
            int uid = Integer.parseInt(bundle.getString("uid"));
            policy = app.mDB.getPolicy(uid);
            if (policy == null) {
                policy = new Policy(uid, pm);
            }
        } catch (IOException | PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            finish();
            return;
        }

        // Never allow com.topjohnwu.magisk (could be malware)
        if (TextUtils.equals(policy.packageName, BuildConfig.APPLICATION_ID)) {
            finish();
            return;
        }

        switch ((int) Config.get(Config.Key.SU_AUTO_RESPONSE)) {
            case Config.Value.SU_AUTO_DENY:
                handleAction(Policy.DENY, 0);
                return;
            case Config.Value.SU_AUTO_ALLOW:
                handleAction(Policy.ALLOW, 0);
                return;
            case Config.Value.SU_PROMPT:
            default:
        }

        // If not interactive, response directly
        if (policy.policy != Policy.INTERACTIVE) {
            handleAction();
            return;
        }

        setContentView(R.layout.activity_request);
        new SuRequestActivity_ViewBinding(this);

        appIcon.setImageDrawable(policy.info.loadIcon(pm));
        appNameView.setText(policy.appName);
        packageNameView.setText(policy.packageName);
        warning.setCompoundDrawablesRelativeWithIntrinsicBounds(
                AppCompatResources.getDrawable(this, R.drawable.ic_warning), null, null, null);

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.allow_timeout, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        timeout.setAdapter(adapter);
        timeout.setSelection(timeoutPrefs.getInt(policy.packageName, 0));

        timer = new CountDownTimer((int) Config.get(Config.Key.SU_REQUEST_TIMEOUT) * 1000, 1000) {
            @Override
            public void onTick(long millisUntilFinished) {
                deny_btn.setText(getString(R.string.deny_with_str, "(" + millisUntilFinished / 1000 + ")"));
            }
            @Override
            public void onFinish() {
                deny_btn.setText(getString(R.string.deny_with_str, "(0)"));
                handleAction(Policy.DENY);
            }
        };

        boolean useFP = FingerprintHelper.useFingerprint();

        if (useFP) {
            try {
                fingerprintHelper = new FingerprintHelper() {
                    @Override
                    public void onAuthenticationError(int errorCode, CharSequence errString) {
                        warning.setText(errString);
                    }

                    @Override
                    public void onAuthenticationHelp(int helpCode, CharSequence helpString) {
                        warning.setText(helpString);
                    }

                    @Override
                    public void onAuthenticationSucceeded(FingerprintManager.AuthenticationResult result) {
                        handleAction(Policy.ALLOW);
                    }

                    @Override
                    public void onAuthenticationFailed() {
                        warning.setText(R.string.auth_fail);
                    }
                };
                fingerprintHelper.authenticate();
            } catch (Exception e) {
                e.printStackTrace();
                useFP = false;
            }
        }

        if (!useFP) {
            grant_btn.setOnClickListener(v -> {
                handleAction(Policy.ALLOW);
                timer.cancel();
            });
            grant_btn.requestFocus();
        }

        grant_btn.setVisibility(useFP ? View.GONE : View.VISIBLE);
        fingerprintImg.setVisibility(useFP ? View.VISIBLE : View.GONE);

        deny_btn.setOnClickListener(v -> {
            handleAction(Policy.DENY);
            timer.cancel();
        });
        suPopup.setOnClickListener(v -> cancelTimeout());
        timeout.setOnTouchListener((v, event) -> cancelTimeout());
        timer.start();
    }

    private boolean cancelTimeout() {
        timer.cancel();
        deny_btn.setText(getString(R.string.deny));
        return false;
    }

    private void handleAction() {
        connector.response();
        finish();
    }

    private void handleAction(int action) {
        int pos = timeout.getSelectedItemPosition();
        timeoutPrefs.edit().putInt(policy.packageName, pos).apply();
        handleAction(action, Config.Value.TIMEOUT_LIST[pos]);
    }

    private void handleAction(int action, int time) {
        policy.policy = action;
        if (time >= 0) {
            policy.until = (time == 0) ? 0 : (System.currentTimeMillis() / 1000 + time * 60);
            app.mDB.updatePolicy(policy);
        }
        handleAction();
    }
}
