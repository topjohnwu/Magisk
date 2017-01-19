package com.topjohnwu.magisk;

import android.Manifest;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.design.widget.NavigationView;
import android.support.v4.app.ActivityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import com.topjohnwu.magisk.utils.CallbackHandler;
import com.topjohnwu.magisk.utils.Shell;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener, CallbackHandler.EventListener {

    private static final String SELECTED_ITEM_ID = "SELECTED_ITEM_ID";

    private final Handler mDrawerHandler = new Handler();
    private SharedPreferences prefs;

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.drawer_layout) DrawerLayout drawer;
    @BindView(R.id.nav_view) public NavigationView navigationView;

    private int mSelectedId = R.id.status;
    private float toolbarElevation;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {

        prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

        if (Global.Configs.isDarkTheme) {
            setTheme(R.style.AppTheme_dh);
        }
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                && Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
        }

        setSupportActionBar(toolbar);

        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close) {
            @Override
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
                super.onDrawerSlide(drawerView, 0); // this disables the arrow @ completed tate
            }

            @Override
            public void onDrawerSlide(View drawerView, float slideOffset) {
                super.onDrawerSlide(drawerView, 0); // this disables the animation
            }
        };

        toolbarElevation = toolbar.getElevation();

        drawer.addDrawerListener(toggle);
        toggle.syncState();

        if (savedInstanceState == null) {
            navigate(mSelectedId, true);
            navigationView.setCheckedItem(mSelectedId);
        } else {
            mSelectedId = savedInstanceState.getInt(SELECTED_ITEM_ID);
        }

        navigationView.setNavigationItemSelectedListener(this);
        CallbackHandler.register(Global.Events.reloadMainActivity, this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        CallbackHandler.register(Global.Events.updateCheckDone, this);
        if (Global.Events.updateCheckDone.isTriggered) {
            onTrigger(Global.Events.updateCheckDone);
        }
        checkHideSection();
    }

    @Override
    protected void onPause() {
        CallbackHandler.unRegister(Global.Events.updateCheckDone, this);
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        CallbackHandler.unRegister(Global.Events.reloadMainActivity, this);
        super.onDestroy();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt(SELECTED_ITEM_ID, mSelectedId);
    }

    @Override
    public void onBackPressed() {
        if (drawer.isDrawerOpen(navigationView)) {
            drawer.closeDrawer(navigationView);
        } else {
            finish();
        }
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull final MenuItem menuItem) {
        mSelectedId = menuItem.getItemId();
        mDrawerHandler.removeCallbacksAndMessages(null);
        mDrawerHandler.postDelayed(() -> navigate(menuItem.getItemId(), false), 250);
        drawer.closeDrawer(navigationView);
        return true;
    }

    @Override
    public void onTrigger(CallbackHandler.Event event) {
        if (event == Global.Events.updateCheckDone) {
            Menu menu = navigationView.getMenu();
            menu.findItem(R.id.install).setVisible(Global.Info.remoteMagiskVersion > 0 &&
                    Shell.rootAccess());
        } else if (event == Global.Events.reloadMainActivity) {
            recreate();
        }
    }

    private void checkHideSection() {
        Menu menu = navigationView.getMenu();
        if (Shell.rootAccess()) {
            menu.findItem(R.id.magiskhide).setVisible(
                    Global.Info.magiskVersion >= 8 && prefs.getBoolean("magiskhide", false));
            menu.findItem(R.id.modules).setVisible(Global.Info.magiskVersion >= 4);
            menu.findItem(R.id.downloads).setVisible(Global.Info.magiskVersion >= 4);
            menu.findItem(R.id.log).setVisible(true);
            menu.findItem(R.id.superuser).setVisible(Global.Info.isSuClient);
        }
    }

    public void navigate(int itemId, boolean now) {
        toolbar.setElevation(toolbarElevation);
        switch (itemId) {
            case R.id.status:
                displayFragment(new StatusFragment(), "status", now);
                break;
            case R.id.install:
                displayFragment(new InstallFragment(), "install", now);
                break;
            case R.id.superuser:
                displayFragment(new SuperuserFragment(), "superuser", now);
                break;
            case R.id.modules:
                displayFragment(new ModulesFragment(), "modules", now);
                break;
            case R.id.downloads:
                displayFragment(new ReposFragment(), "downloads", now);
                break;
            case R.id.magiskhide:
                displayFragment(new MagiskHideFragment(), "magiskhide", now);
                break;
            case R.id.log:
                displayFragment(new LogFragment(), "log", now);
                toolbar.setElevation(0);
                break;
            case R.id.settings:
                startActivity(new Intent(this, SettingsActivity.class));
                break;
            case R.id.app_about:
                startActivity(new Intent(this, AboutActivity.class));
                break;
        }
    }

    private void displayFragment(@NonNull Fragment navFragment, String tag, boolean now) {
        FragmentTransaction transaction = getFragmentManager().beginTransaction();
        if (!now) {
            transaction.setCustomAnimations(android.R.animator.fade_in, android.R.animator.fade_out);
        }
        transaction.replace(R.id.content_frame, navFragment, tag).commit();
    }
}
