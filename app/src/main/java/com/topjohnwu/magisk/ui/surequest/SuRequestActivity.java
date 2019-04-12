package com.topjohnwu.magisk.ui.surequest;

import android.annotation.SuppressLint;
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

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.model.entity.Policy;
import com.topjohnwu.magisk.ui.base.BaseActivity;
import com.topjohnwu.magisk.utils.FingerprintHelper;
import com.topjohnwu.magisk.utils.SuConnector;
import com.topjohnwu.magisk.utils.SuLogger;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import java9.lang.Iterables;

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

    private ActionHandler handler;
    private Policy policy;
    private SharedPreferences timeoutPrefs;

    public static final String REQUEST = "request";
    public static final String LOG = "log";
    public static final String NOTIFY = "notify";

    @Override
    public int getDarkTheme() {
        return R.style.SuRequest_Dark;
    }

    @Override
    public void onBackPressed() {
        handler.handleAction(Policy.DENY, -1);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        lockOrientation();
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);

        timeoutPrefs = App.deContext.getSharedPreferences("su_timeout", 0);
        Intent intent = getIntent();

        String action = intent.getAction();

        if (TextUtils.equals(action, REQUEST)) {
            if (!handleRequest())
                finish();
            return;
        }

        if (TextUtils.equals(action, LOG))
            SuLogger.handleLogs(intent);
        else if (TextUtils.equals(action, NOTIFY))
            SuLogger.handleNotify(intent);

        finish();
    }

    private boolean handleRequest() {
        String socketName = getIntent().getStringExtra("socket");

        if (socketName == null)
            return false;

        SuConnector connector;
        try {
            connector = new SuConnector(socketName) {
                @Override
                protected void onResponse() throws IOException {
                    out.writeInt(policy.policy);
                }
            };
            Bundle bundle = connector.readSocketInput();
            int uid = Integer.parseInt(bundle.getString("uid"));
            app.mDB.clearOutdated();
            policy = app.mDB.getPolicy(uid);
            if (policy == null) {
                policy = new Policy(uid, getPackageManager());
            }
        } catch (IOException | PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            return false;
        }
        handler = new ActionHandler() {
            @Override
            void handleAction() {
                connector.response();
                done();
            }

            @Override
            void handleAction(int action) {
                int pos = timeout.getSelectedItemPosition();
                timeoutPrefs.edit().putInt(policy.packageName, pos).apply();
                handleAction(action, Config.Value.TIMEOUT_LIST[pos]);
            }

            @Override
            void handleAction(int action, int time) {
                policy.policy = action;
                if (time >= 0) {
                    policy.until = (time == 0) ? 0
                            : (System.currentTimeMillis() / 1000 + time * 60);
                    app.mDB.updatePolicy(policy);
                }
                handleAction();
            }
        };

        // Never allow com.topjohnwu.magisk (could be malware)
        if (TextUtils.equals(policy.packageName, BuildConfig.APPLICATION_ID))
            return false;

        // If not interactive, response directly
        if (policy.policy != Policy.INTERACTIVE) {
            handler.handleAction();
            return true;
        }

        switch ((int) Config.get(Config.Key.SU_AUTO_RESPONSE)) {
            case Config.Value.SU_AUTO_DENY:
                handler.handleAction(Policy.DENY, 0);
                return true;
            case Config.Value.SU_AUTO_ALLOW:
                handler.handleAction(Policy.ALLOW, 0);
                return true;
        }

        showUI();
        return true;
    }

    @SuppressLint("ClickableViewAccessibility")
    private void showUI() {
        setContentView(R.layout.activity_request);
        new SuRequestActivity_ViewBinding(this);

        appIcon.setImageDrawable(policy.info.loadIcon(getPackageManager()));
        appNameView.setText(policy.appName);
        packageNameView.setText(policy.packageName);
        warning.setCompoundDrawablesRelativeWithIntrinsicBounds(
                AppCompatResources.getDrawable(this, R.drawable.ic_warning), null, null, null);

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.allow_timeout, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        timeout.setAdapter(adapter);
        timeout.setSelection(timeoutPrefs.getInt(policy.packageName, 0));

        CountDownTimer timer = new CountDownTimer(
                (int) Config.get(Config.Key.SU_REQUEST_TIMEOUT) * 1000, 1000) {
            @Override
            public void onTick(long remains) {
                deny_btn.setText(getString(R.string.deny) + "(" + remains / 1000 + ")");
            }
            @Override
            public void onFinish() {
                deny_btn.setText(getString(R.string.deny));
                handler.handleAction(Policy.DENY);
            }
        };
        timer.start();
        Runnable cancelTimer = () -> {
            timer.cancel();
            deny_btn.setText(getString(R.string.deny));
        };
        handler.addCancel(cancelTimer);

        boolean useFP = FingerprintHelper.useFingerprint();

        if (useFP) try {
            FingerprintHelper helper = new SuFingerprint();
            helper.authenticate();
            handler.addCancel(helper::cancel);
        } catch (Exception e) {
            e.printStackTrace();
            useFP = false;
        }

        if (!useFP) {
            grant_btn.setOnClickListener(v -> {
                handler.handleAction(Policy.ALLOW);
                timer.cancel();
            });
            grant_btn.requestFocus();
        }

        grant_btn.setVisibility(useFP ? View.GONE : View.VISIBLE);
        fingerprintImg.setVisibility(useFP ? View.VISIBLE : View.GONE);

        deny_btn.setOnClickListener(v -> {
            handler.handleAction(Policy.DENY);
            timer.cancel();
        });
        suPopup.setOnClickListener(v -> cancelTimer.run());
        timeout.setOnTouchListener((v, event) -> {
            cancelTimer.run();
            return false;
        });
    }

    private class SuFingerprint extends FingerprintHelper {

        SuFingerprint() throws Exception {}

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
            handler.handleAction(Policy.ALLOW);
        }

        @Override
        public void onAuthenticationFailed() {
            warning.setText(R.string.auth_fail);
        }
    }

    private class ActionHandler {
        private List<Runnable> cancelTasks = new ArrayList<>();

        void handleAction() {
            done();
        }

        void handleAction(int action) {
            done();
        }

        void handleAction(int action, int time) {
            done();
        }

        void addCancel(Runnable r) {
            cancelTasks.add(r);
        }

        void done() {
            Iterables.forEach(cancelTasks, Runnable::run);
            finish();
        }
    }
}
