package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;
import android.widget.Toast;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.SafetyNetHelper;
import com.topjohnwu.magisk.utils.Utils;

public class SplashActivity extends AppCompatActivity {

    private SharedPreferences prefs;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        prefs = PreferenceManager.getDefaultSharedPreferences(getApplication());
        if (prefs.getString("theme", "").equals("Dark")) {
            setTheme(R.style.AppTheme_dh);
        }

        Logger.devLog = prefs.getBoolean("developer_logging", false);
        Logger.logShell = prefs.getBoolean("shell_logging", false);

        // Initialize prefs
        prefs.edit()
                .putBoolean("module_done", false)
                .putBoolean("repo_done", false)
                .putBoolean("update_check_done", false)
                .putBoolean("magiskhide", Utils.itemExist(false, "/magisk/.core/magiskhide/enable"))
                .putBoolean("busybox", Utils.commandExists("busybox"))
                .putBoolean("hosts", Utils.itemExist(false, "/magisk/.core/hosts"))
                .apply();

        // Simple POC for checking SN status
        new SafetyNetHelper(getApplicationContext()) {
            @Override
            public void handleResults(int i) {
                switch (i) {
                    case -1:
                        Toast.makeText(mContext, "SN: Error", Toast.LENGTH_LONG).show();
                        break;
                    case 0:
                        Toast.makeText(mContext, "SN: Fail", Toast.LENGTH_LONG).show();
                        break;
                    case 1:
                        Toast.makeText(mContext, "SN: Success", Toast.LENGTH_LONG).show();
                        break;
                }
            }
        }.requestTest();

        new Async.CheckUpdates(prefs).exec();

        new Async.LoadModules(prefs) {
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                new Async.LoadRepos(getApplicationContext()).exec();
                // Start main activity
                Intent intent = new Intent(getApplicationContext(), MainActivity.class);
                startActivity(intent);
                finish();
            }
        }.exec();

    }
}
