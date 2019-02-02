package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.text.TextUtils;

import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.tasks.CheckUpdates;
import com.topjohnwu.magisk.tasks.UpdateRepos;
import com.topjohnwu.magisk.uicomponents.Notifications;
import com.topjohnwu.magisk.uicomponents.Shortcuts;
import com.topjohnwu.magisk.utils.AppUtils;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;

public class SplashActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Dynamic detect all locales
        LocaleManager.loadAvailableLocales(R.string.app_changelog);

        String pkg = Config.get(Config.Key.SU_MANAGER);
        if (pkg != null && getPackageName().equals(BuildConfig.APPLICATION_ID)) {
            Config.remove(Config.Key.SU_MANAGER);
            Shell.su("pm uninstall " + pkg).submit();
        }
        if (TextUtils.equals(pkg, getPackageName())) {
            try {
                // We are the manager, remove com.topjohnwu.magisk as it could be malware
                getPackageManager().getApplicationInfo(BuildConfig.APPLICATION_ID, 0);
                Shell.su("pm uninstall " + BuildConfig.APPLICATION_ID).submit();
            } catch (PackageManager.NameNotFoundException ignored) {}
        }

        // Magisk working as expected
        if (Shell.rootAccess() && Config.magiskVersionCode > 0) {
            // Load modules
            Utils.loadModules();
        }

        // Set default configs
        Config.initialize();

        // Create notification channel on Android O
        Notifications.setup(this);

        // Schedule periodic update checks
        AppUtils.scheduleUpdateCheck();

        // Setup shortcuts
        Shortcuts.setup(this);

        if (Networking.checkNetworkStatus(this)) {
            // Repo update check
            new UpdateRepos().exec();
        }

        app.init = true;

        Intent intent = new Intent(this, ClassMap.get(MainActivity.class));
        intent.putExtra(Const.Key.OPEN_SECTION, getIntent().getStringExtra(Const.Key.OPEN_SECTION));
        intent.putExtra(BaseActivity.INTENT_PERM, getIntent().getStringExtra(BaseActivity.INTENT_PERM));
        startActivity(intent);
        finish();
    }
}
