package com.topjohnwu.magisk;

import android.Manifest;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.design.widget.NavigationView;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.Shell;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends Activity
        implements NavigationView.OnNavigationItemSelectedListener, CallbackEvent.Listener<Void> {

    public static final String SECTION = "section";

    private final Handler mDrawerHandler = new Handler();
    private SharedPreferences prefs;
    private int mDrawerItem;

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.drawer_layout) DrawerLayout drawer;
    @BindView(R.id.nav_view) public NavigationView navigationView;

    private float toolbarElevation;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {

        prefs = getApplicationContext().prefs;

        if (getApplicationContext().isDarkTheme) {
            setTheme(R.style.AppTheme_Dark);
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

        if (savedInstanceState != null)
            navigate(savedInstanceState.getInt(SECTION, R.id.status));
        else
            navigate(getIntent().getStringExtra(SECTION));

        navigationView.setNavigationItemSelectedListener(this);
        getApplicationContext().reloadMainActivity.register(this);

    }

    @Override
    protected void onResume() {
        super.onResume();
        checkHideSection();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt(SECTION, mDrawerItem);
    }

    @Override
    protected void onDestroy() {
        getApplicationContext().reloadMainActivity.unRegister(this);
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (drawer.isDrawerOpen(navigationView))
            drawer.closeDrawer(navigationView);
        else
            finish();
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull final MenuItem menuItem) {
        mDrawerHandler.removeCallbacksAndMessages(null);
        mDrawerHandler.postDelayed(() -> navigate(menuItem.getItemId()), 250);
        drawer.closeDrawer(navigationView);
        return true;
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        recreate();
    }

    private void checkHideSection() {
        Menu menu = navigationView.getMenu();
        if (Shell.rootAccess()) {
            menu.findItem(R.id.magiskhide).setVisible(
                    getApplicationContext().magiskVersion >= 8 && prefs.getBoolean("magiskhide", false));
            menu.findItem(R.id.modules).setVisible(getApplicationContext().magiskVersion >= 4);
            menu.findItem(R.id.downloads).setVisible(getApplicationContext().magiskVersion >= 4);
            menu.findItem(R.id.log).setVisible(true);
            menu.findItem(R.id.superuser).setVisible(getApplicationContext().isSuClient);
        }
        menu.findItem(R.id.install).setVisible(getApplicationContext().remoteMagiskVersion > 0);
    }

    public void navigate(String item) {
        int itemId = R.id.status;
        if (item != null) {
            switch (item) {
                case "status":
                    itemId = R.id.status;
                    break;
                case "install":
                    itemId = R.id.install;
                    break;
                case "superuser":
                    itemId = R.id.superuser;
                    break;
                case "modules":
                    itemId = R.id.modules;
                    break;
                case "downloads":
                    itemId = R.id.downloads;
                    break;
                case "magiskhide":
                    itemId = R.id.magiskhide;
                    break;
                case "log":
                    itemId = R.id.log;
                    break;
                case "settings":
                    itemId = R.id.settings;
                    break;
                case "about":
                    itemId = R.id.app_about;
                    break;
            }
        }
        navigate(itemId);
    }

    public void navigate(int itemId) {
        mDrawerItem = itemId;
        navigationView.setCheckedItem(itemId);
        switch (itemId) {
            case R.id.status:
                displayFragment(new StatusFragment(), "status", true);
                break;
            case R.id.install:
                displayFragment(new InstallFragment(), "install", true);
                break;
            case R.id.superuser:
                displayFragment(new SuperuserFragment(), "superuser", true);
                break;
            case R.id.modules:
                displayFragment(new ModulesFragment(), "modules", true);
                break;
            case R.id.downloads:
                displayFragment(new ReposFragment(), "downloads", true);
                break;
            case R.id.magiskhide:
                displayFragment(new MagiskHideFragment(), "magiskhide", true);
                break;
            case R.id.log:
                displayFragment(new LogFragment(), "log", false);
                break;
            case R.id.settings:
                startActivity(new Intent(this, SettingsActivity.class));
                break;
            case R.id.app_about:
                startActivity(new Intent(this, AboutActivity.class));
                break;
        }
    }

    private void displayFragment(@NonNull Fragment navFragment, String tag, boolean setElevation) {
        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
        supportInvalidateOptionsMenu();
        transaction.setCustomAnimations(android.R.anim.fade_in, android.R.anim.fade_out);
        transaction.replace(R.id.content_frame, navFragment, tag).commitNow();
        if (setElevation) toolbar.setElevation(toolbarElevation);
        else toolbar.setElevation(0);
    }
}
