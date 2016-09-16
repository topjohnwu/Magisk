package com.topjohnwu.magisk;

import android.Manifest;
import android.app.AppOpsManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.support.annotation.IdRes;
import android.support.annotation.NonNull;
import android.support.design.widget.NavigationView;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.MenuItem;
import android.view.View;

import com.topjohnwu.magisk.module.RepoHelper;
import com.topjohnwu.magisk.utils.Utils;

import butterknife.BindView;
import butterknife.ButterKnife;

public class WelcomeActivity extends AppCompatActivity implements NavigationView.OnNavigationItemSelectedListener {

    private static final String SELECTED_ITEM_ID = "SELECTED_ITEM_ID";

    private final Handler mDrawerHandler = new Handler();

    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.drawer_layout)
    DrawerLayout drawer;
    @BindView(R.id.nav_view)
    NavigationView navigationView;

    @IdRes
    private int mSelectedId = R.id.magisk;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_welcome);
        ButterKnife.bind(this);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
        }

        // Startups
        PreferenceManager.setDefaultValues(this, R.xml.defaultpref, false);
        if (!Utils.isMyServiceRunning(MonitorService.class, getApplicationContext())) {
            Intent myIntent = new Intent(getApplication(), MonitorService.class);
            getApplication().startService(myIntent);
        }
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                && Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
        }
        if (!hasPermission()) {
            startActivityForResult(new Intent(Settings.ACTION_USAGE_ACCESS_SETTINGS), 100);
        }

        new Utils.Initialize(this).execute();
        new Utils.CheckUpdates(this).execute();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        RepoHelper.TaskDelegate delegate = result -> {
            //Do a thing here when we get a result we want
        };
        if (!prefs.contains("hasCachedRepos")) {
            new Utils.LoadModules(this, true).execute();
            new Utils.LoadRepos(this, true, delegate).execute();

        } else {
            new Utils.LoadModules(getApplication(), false).execute();
            new Utils.LoadRepos(this, false, delegate).execute();

        }

        setSupportActionBar(toolbar);

        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close) {
            @Override
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
                super.onDrawerSlide(drawerView, 0); // this disables the arrow @ completed state
            }

            @Override
            public void onDrawerSlide(View drawerView, float slideOffset) {
                super.onDrawerSlide(drawerView, 0); // this disables the animation
            }
        };

        drawer.addDrawerListener(toggle);
        toggle.syncState();

        //noinspection ResourceType
        mSelectedId = savedInstanceState == null ? mSelectedId : savedInstanceState.getInt(SELECTED_ITEM_ID);
        navigationView.setCheckedItem(mSelectedId);

        if (savedInstanceState == null) {
            mDrawerHandler.removeCallbacksAndMessages(null);
            mDrawerHandler.postDelayed(() -> navigate(mSelectedId), 250);
        }

        navigationView.setNavigationItemSelectedListener(this);
        if (getIntent().hasExtra("relaunch")) {
            navigate(R.id.root);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        outState.putInt(SELECTED_ITEM_ID, mSelectedId);
    }

    @Override
    public void onBackPressed() {
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull final MenuItem menuItem) {
        mSelectedId = menuItem.getItemId();
        mDrawerHandler.removeCallbacksAndMessages(null);
        mDrawerHandler.postDelayed(() -> navigate(menuItem.getItemId()), 250);

        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    private boolean hasPermission() {
        AppOpsManager appOps = (AppOpsManager)
                getSystemService(Context.APP_OPS_SERVICE);
        int mode = appOps.checkOpNoThrow(AppOpsManager.OPSTR_GET_USAGE_STATS,
                android.os.Process.myUid(), getPackageName());
        return mode == AppOpsManager.MODE_ALLOWED;

    }

    private void navigate(final int itemId) {
        Fragment navFragment = null;
        String tag = "";
        switch (itemId) {
            case R.id.magisk:
                setTitle(R.string.magisk);
                tag = "magisk";
                navFragment = new MagiskFragment();
                break;
            case R.id.root:
                setTitle(R.string.root);
                tag = "root";
                navFragment = new RootFragment();
                break;
            case R.id.autoroot:
                setTitle(R.string.auto_root);
                tag = "ic_autoroot";
                navFragment = new AutoRootFragment();
                break;
            case R.id.modules:
                setTitle(R.string.modules);
                tag = "modules";
                navFragment = new ModulesFragment();
                break;
            case R.id.downloads:
                setTitle(R.string.downloads);
                tag = "downloads";
                navFragment = new ReposFragment();
                break;
            case R.id.log:
                setTitle(R.string.log);
                tag = "log";
                navFragment = new LogFragment();
                break;
            case R.id.app_about:
                startActivity(new Intent(this, AboutActivity.class));
                return;
        }

        if (navFragment != null) {
            FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
            transaction.setCustomAnimations(R.anim.fade_in, R.anim.fade_out);
            try {
                toolbar.setElevation(navFragment instanceof ModulesFragment ? 0 : 10);

                transaction.replace(R.id.content_frame, navFragment, tag).commit();
            } catch (IllegalStateException ignored) {
            }
        }
    }
}