package com.topjohnwu.magisk.superuser;

import android.content.ContentValues;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.FileObserver;
import android.text.TextUtils;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.ParallelTask;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;

import java.io.DataInputStream;
import java.io.IOException;

import butterknife.BindView;
import butterknife.ButterKnife;

public class RequestActivity extends Activity {

    @BindView(R.id.su_popup) LinearLayout suPopup;
    @BindView(R.id.timeout) Spinner timeout;
    @BindView(R.id.app_icon) ImageView appIcon;
    @BindView(R.id.app_name) TextView appNameView;
    @BindView(R.id.package_name) TextView packageNameView;
    @BindView(R.id.grant_btn) Button grant_btn;
    @BindView(R.id.deny_btn) Button deny_btn;

    private String socketPath;
    private LocalSocket socket;
    private PackageManager pm;
    private MagiskManager mm;

    private boolean hasTimeout;
    private Policy policy;
    private CountDownTimer timer;

    @Override
    public int getDarkTheme() {
        return R.style.SuRequest_Dark;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);

        pm = getPackageManager();
        mm = Utils.getMagiskManager(this);
        mm.suDB.cleanup();

        Intent intent = getIntent();
        socketPath = intent.getStringExtra("socket");
        hasTimeout = intent.getBooleanExtra("timeout", true);

        new FileObserver(socketPath) {
            @Override
            public void onEvent(int fileEvent, String path) {
                if (fileEvent == FileObserver.DELETE_SELF) {
                    finish();
                }
            }
        }.startWatching();

        new SocketManager(this).exec();
    }

    private boolean cancelTimeout() {
        timer.cancel();
        deny_btn.setText(getString(R.string.deny));
        return false;
    }

    private void showRequest() {
        switch (mm.suResponseType) {
            case Const.Value.SU_AUTO_DENY:
                handleAction(Policy.DENY, 0);
                return;
            case Const.Value.SU_AUTO_ALLOW:
                handleAction(Policy.ALLOW, 0);
                return;
            case Const.Value.SU_PROMPT:
            default:
        }

        // If not interactive, response directly
        if (policy.policy != Policy.INTERACTIVE) {
            handleAction();
            return;
        }

        setContentView(R.layout.activity_request);
        ButterKnife.bind(this);

        appIcon.setImageDrawable(policy.info.loadIcon(pm));
        appNameView.setText(policy.appName);
        packageNameView.setText(policy.packageName);

        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.allow_timeout, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        timeout.setAdapter(adapter);

        timer = new CountDownTimer(mm.suRequestTimeout * 1000, 1000) {
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

        grant_btn.setOnClickListener(v -> {
            handleAction(Policy.ALLOW);
            timer.cancel();
        });
        grant_btn.requestFocus();
        deny_btn.setOnClickListener(v -> {
            handleAction(Policy.DENY);
            timer.cancel();
        });
        suPopup.setOnClickListener(v -> cancelTimeout());
        timeout.setOnTouchListener((v, event) -> cancelTimeout());

        if (hasTimeout) {
            timer.start();
        } else {
            cancelTimeout();
        }
    }

    @Override
    public void onBackPressed() {
        if (policy != null) {
            handleAction(Policy.DENY);
        }
        finish();
    }

    void handleAction() {
        String response;
        if (policy.policy == Policy.ALLOW) {
            response = "socket:ALLOW";
        } else {
            response = "socket:DENY";
        }
        try {
            socket.getOutputStream().write((response).getBytes());
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        finish();
    }

    void handleAction(int action) {
        handleAction(action, Const.Value.timeoutList[timeout.getSelectedItemPosition()]);
    }

    void handleAction(int action, int time) {
        policy.policy = action;
        if (time >= 0) {
            policy.until = (time == 0) ? 0 : (System.currentTimeMillis() / 1000 + time * 60);
            mm.suDB.addPolicy(policy);
        }
        handleAction();
    }

    private class SocketManager extends ParallelTask<Void, Void, Boolean> {

        SocketManager(Activity context) {
            super(context);
        }

        @Override
        protected Boolean doInBackground(Void... params) {
            try {
                socket = new LocalSocket();
                socket.connect(new LocalSocketAddress(socketPath, LocalSocketAddress.Namespace.FILESYSTEM));

                DataInputStream is = new DataInputStream(socket.getInputStream());
                ContentValues payload = new ContentValues();

                while (true) {
                    int nameLen = is.readInt();
                    byte[] nameBytes = new byte[nameLen];
                    is.readFully(nameBytes);
                    String name = new String(nameBytes);
                    if (TextUtils.equals(name, "eof"))
                        break;

                    int dataLen = is.readInt();
                    byte[] dataBytes = new byte[dataLen];
                    is.readFully(dataBytes);
                    String data = new String(dataBytes);
                    payload.put(name, data);
                }

                if (payload.getAsInteger("uid") == null) {
                    return false;
                }

                int uid = payload.getAsInteger("uid");
                policy = mm.suDB.getPolicy(uid);
                if (policy == null) {
                    policy = new Policy(uid, pm);
                }
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            if (result) {
                showRequest();
            } else {
                finish();
            }
        }
    }
}
