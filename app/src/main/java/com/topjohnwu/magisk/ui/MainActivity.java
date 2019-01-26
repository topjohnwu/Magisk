package com.topjohnwu.magisk.ui;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.appcompat.widget.Toolbar;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;

import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ui.base.BaseActivity;
import com.topjohnwu.magisk.ui.home.MagiskFragment;
import com.topjohnwu.magisk.ui.log.LogFragment;
import com.topjohnwu.magisk.ui.module.ModulesFragment;
import com.topjohnwu.magisk.ui.module.ReposFragment;
import com.topjohnwu.magisk.ui.settings.SettingsFragment;
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import butterknife.BindView;

public class MainActivity extends BaseActivity
        implements BottomNavigationView.OnNavigationItemSelectedListener,
        PreferenceFragmentCompat.OnPreferenceStartFragmentCallback {

    private int mDrawerItem;
    private static boolean fromShortcut = false;

    @BindView(R.id.toolbar) public Toolbar toolbar;
    @BindView(R.id.bottom_nav) BottomNavigationView bottomNavigationView;

    private float toolbarElevation;

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_Dark;
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        if (!SplashActivity.DONE) {
            startActivity(new Intent(this, ClassMap.get(SplashActivity.class)));
            finish();
        }

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        new MainActivity_ViewBinding(this);
        checkHideSection();
        setSupportActionBar(toolbar);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            toolbarElevation = toolbar.getElevation();
        }

        if (savedInstanceState == null) {
            String section = getIntent().getStringExtra(Const.Key.OPEN_SECTION);
            fromShortcut = section != null;
            navigate(section);
        }

        bottomNavigationView.setOnNavigationItemSelectedListener(this);
    }

    @Override
    public void onBackPressed() {
        int entryCount = getSupportFragmentManager().getBackStackEntryCount();
        if (entryCount > 0) {
            getSupportFragmentManager().popBackStack();
            setHasBack(entryCount > 1);
        } else if (mDrawerItem != R.id.magisk && !fromShortcut) {
            navigate(R.id.magisk);
        } else {
            finish();
        }
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull final MenuItem menuItem) {
        navigate(menuItem.getItemId());
        return true;
    }

    public void checkHideSection() {
        Menu menu = bottomNavigationView.getMenu();
        menu.findItem(R.id.modules).setVisible(Shell.rootAccess() && Config.magiskVersionCode >= 0);
        menu.findItem(R.id.log).setVisible(Shell.rootAccess());
        menu.findItem(R.id.superuser).setVisible(Utils.showSuperUser());
    }

    public void navigate(String item) {
        int itemId = R.id.magisk;
        if (item != null) {
            switch (item) {
                case "superuser":
                    itemId = R.id.superuser;
                    break;
                case "modules":
                    itemId = R.id.modules;
                    break;
                case "downloads":
                    itemId = R.id.downloads;
                    break;
                case "log":
                    itemId = R.id.log;
                    break;
                case "settings":
                    itemId = R.id.settings;
                    break;
            }
        }
        navigate(itemId);
    }

    public void navigate(int itemId) {
        if (mDrawerItem == itemId) return;
        mDrawerItem = itemId;
        bottomNavigationView.setSelectedItemId(itemId);
        switch (itemId) {
            case R.id.magisk:
                fromShortcut = false;
                displayFragment(new MagiskFragment(), true);
                break;
            case R.id.superuser:
                displayFragment(new SuperuserFragment(), true);
                break;
            case R.id.modules:
                displayFragment(new ModulesFragment(), true);
                break;
            case R.id.downloads:
                displayFragment(new ReposFragment(), true);
                break;
            case R.id.log:
                displayFragment(new LogFragment(), false);
                break;
            case R.id.settings:
                displayFragment(new SettingsFragment(), true);
                break;
        }
    }

    private void displayFragment(@NonNull Fragment navFragment, boolean setElevation) {
        supportInvalidateOptionsMenu();
        getSupportFragmentManager()
                .beginTransaction()
                .setTransition(FragmentTransaction.TRANSIT_FRAGMENT_FADE)
                .replace(R.id.content_frame, navFragment)
                .commitNow();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            toolbar.setElevation(setElevation ? toolbarElevation : 0);
        }
    }

    @Override
    public boolean onPreferenceStartFragment(PreferenceFragmentCompat caller, Preference pref) {
        addFragment(Fragment.instantiate(this, pref.getFragment()));
        return true;
    }

    public void addFragment(@NonNull Fragment fragment) {
        getSupportFragmentManager()
                .beginTransaction()
                .setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
                .replace(R.id.content_frame, fragment)
                .addToBackStack(null)
                .commit();
        setHasBack(true);
    }

    private void setHasBack(boolean show) {
        getSupportActionBar().setDisplayShowHomeEnabled(show);
        getSupportActionBar().setDisplayHomeAsUpEnabled(show);
        bottomNavigationView.setVisibility(show ? View.GONE : View.VISIBLE);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
