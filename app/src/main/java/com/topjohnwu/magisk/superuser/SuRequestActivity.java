package com.topjohnwu.magisk.superuser;

import android.content.ContentValues;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.support.v7.app.AppCompatActivity;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import com.topjohnwu.magisk.Global;
import com.topjohnwu.magisk.R;

import java.io.DataInputStream;
import java.io.IOException;

import butterknife.BindView;
import butterknife.ButterKnife;

public class SuRequestActivity extends AppCompatActivity {

    private static final int[] timeoutList = {0, -1, 10, 20, 30, 60};
    private final static int SU_PROTOCOL_PARAM_MAX = 20;
    private final static int SU_PROTOCOL_NAME_MAX = 20;
    private final static int SU_PROTOCOL_VALUE_MAX = 256;

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
    private PackageInfo info;

    private int uid;
    private String appName, packageName;
    private CountDownTimer timer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);

        pm = getPackageManager();

        Intent intent = getIntent();
        socketPath = intent.getStringExtra("socket");

        new SocketManager().execute();
    }

    void updateUI() throws Throwable {
        setContentView(R.layout.activity_request);
        ButterKnife.bind(this);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.timeout, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        timeout.setAdapter(adapter);
        appIcon.setImageDrawable(info.applicationInfo.loadIcon(pm));
        appNameView.setText(appName);
        packageNameView.setText(packageName);

        timer = new CountDownTimer(Global.Configs.suRequestTimeout * 1000, 1000) {
            @Override
            public void onTick(long millisUntilFinished) {
                deny_btn.setText(getString(R.string.deny, "(" + millisUntilFinished / 1000 + ")"));
            }
            @Override
            public void onFinish() {
                deny_btn.setText(getString(R.string.deny, "(0)"));
                handleAction(false, -1);
            }
        };

        grant_btn.setOnClickListener(v -> handleAction(true, timeoutList[timeout.getSelectedItemPosition()]));
        deny_btn.setOnClickListener(v -> handleAction(false, timeoutList[timeout.getSelectedItemPosition()]));
        suPopup.setOnClickListener((v) -> {
            timer.cancel();
            deny_btn.setText(getString(R.string.deny, ""));
        });
        timeout.setOnTouchListener((v, event) -> {
            timer.cancel();
            deny_btn.setText(getString(R.string.deny, ""));
            return false;
        });

        timer.start();

    }

    void handleAction(boolean action, int timeout) {

        try {
            socket.getOutputStream().write((action ? "socket:ALLOW" : "socket:DENY").getBytes());
        } catch (Exception ignored) {}

        if (timeout >= 0) {
            Policy policy = new Policy();
            policy.uid = uid;
            policy.packageName = packageName;
            policy.appName = appName;
            policy.until = (timeout == 0) ? 0 : (System.currentTimeMillis() / 1000 + timeout * 60);
            policy.policy = action ? Policy.ALLOW : Policy.DENY;
            policy.logging = true;
            policy.notification = true;
            new SuDatabaseHelper(this).addPolicy(policy);
        }

        finish();
    }

    private class SocketManager extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            try{
                socket = new LocalSocket();
                socket.connect(new LocalSocketAddress(socketPath, LocalSocketAddress.Namespace.FILESYSTEM));

                DataInputStream is = new DataInputStream(socket.getInputStream());

                ContentValues payload = new ContentValues();

                for (int i = 0; i < SU_PROTOCOL_PARAM_MAX; i++) {

                    int nameLen = is.readInt();
                    if (nameLen > SU_PROTOCOL_NAME_MAX)
                        throw new IllegalArgumentException("name length too long: " + nameLen);

                    byte[] nameBytes = new byte[nameLen];
                    is.readFully(nameBytes);

                    String name = new String(nameBytes);

                    int dataLen = is.readInt();
                    if (dataLen > SU_PROTOCOL_VALUE_MAX)
                        throw new IllegalArgumentException(name + " data length too long: " + dataLen);

                    byte[] dataBytes = new byte[dataLen];
                    is.readFully(dataBytes);

                    String data = new String(dataBytes);

                    payload.put(name, data);

                    if ("eof".equals(name))
                        break;
                }

                uid = payload.getAsInteger("uid");

            }catch (IOException e) {
                e.printStackTrace();
                return false;
            }
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            try {
                if (!result) throw new Throwable();
                String[] pkgs = pm.getPackagesForUid(uid);
                if (pkgs != null && pkgs.length > 0) {
                    info = pm.getPackageInfo(pkgs[0], 0);
                    packageName = pkgs[0];
                    appName = info.applicationInfo.loadLabel(pm).toString();
                    updateUI();
                }
                else
                    throw new Throwable();
            } catch (Throwable e) {
                handleAction(false, -1);
            }
        }
    }
}
