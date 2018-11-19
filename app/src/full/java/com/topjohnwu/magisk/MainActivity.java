package com.topjohnwu.magisk;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import com.google.android.material.navigation.NavigationView;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.fragments.LogFragment;
import com.topjohnwu.magisk.fragments.MagiskFragment;
import com.topjohnwu.magisk.fragments.MagiskHideFragment;
import com.topjohnwu.magisk.fragments.ModulesFragment;
import com.topjohnwu.magisk.fragments.ReposFragment;
import com.topjohnwu.magisk.fragments.SettingsFragment;
import com.topjohnwu.magisk.fragments.SuperuserFragment;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.appcompat.widget.Toolbar;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;
import butterknife.BindView;

public class MainActivity extends BaseActivity
        implements NavigationView.OnNavigationItemSelectedListener, Topic.Subscriber {

    private final Handler mDrawerHandler = new Handler();
    private int mDrawerItem;
    private static boolean fromShortcut = false;

    @BindView(R.id.toolbar) public Toolbar toolbar;
    @BindView(R.id.drawer_layout) DrawerLayout drawer;
    @BindView(R.id.nav_view) NavigationView navigationView;

    private float toolbarElevation;

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_Dark;
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        if (!mm.hasInit) {
            startActivity(new Intent(this, Data.classMap.get(SplashActivity.class)));
            finish();
        }

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        new MainActivity_ViewBinding(this);

        setSupportActionBar(toolbar);

        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(this, drawer, toolbar, R.string.magisk, R.string.magisk) {
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
            String section = getIntent().getStringExtra(Const.Key.OPEN_SECTION);
            fromShortcut = section != null;
            navigate(section);
        }

        navigationView.setNavigationItemSelectedListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        checkHideSection();
    }

    @Override
    public void onBackPressed() {
        if (drawer.isDrawerOpen(navigationView)) {
            drawer.closeDrawer(navigationView);
        } else if (mDrawerItem != R.id.magisk && !fromShortcut) {
            navigate(R.id.magisk);
        } else {
            finish();
        }
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull final MenuItem menuItem) {
        mDrawerHandler.removeCallbacksAndMessages(null);
        mDrawerHandler.postDelayed(() -> navigate(menuItem.getItemId()), 250);
        drawer.closeDrawer(navigationView);
        return true;
    }

    @Override
    public void onPublish(int topic, Object[] result) {
        recreate();
    }

    public void checkHideSection() {
        Menu menu = navigationView.getMenu();
        menu.findItem(R.id.magiskhide).setVisible(Shell.rootAccess() &&
                mm.prefs.getBoolean(Const.Key.MAGISKHIDE, false));
        menu.findItem(R.id.modules).setVisible(Shell.rootAccess() && Data.magiskVersionCode >= 0);
        menu.findItem(R.id.downloads).setVisible(Download.checkNetworkStatus(this)
                && Shell.rootAccess() && Data.magiskVersionCode >= 0);
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
                case "donation":
                    itemId = R.id.donation;
                    break;
            }
        }
        navigate(itemId);
    }

    public void navigate(int itemId) {
        int bak = mDrawerItem;
        mDrawerItem = itemId;
        navigationView.setCheckedItem(itemId);
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
            case R.id.magiskhide:
                displayFragment(new MagiskHideFragment(), true);
                break;
            case R.id.log:
                displayFragment(new LogFragment(), false);
                break;
            case R.id.settings:
                displayFragment(new SettingsFragment(), true);
                break;
            case R.id.app_about:
                startActivity(new Intent(this, Data.classMap.get(AboutActivity.class)));
                mDrawerItem = bak;
                break;
            case R.id.donation:
                startActivity(new Intent(this, Data.classMap.get(DonationActivity.class)));
                mDrawerItem = bak;
                break;
        }
    }

    private void displayFragment(@NonNull Fragment navFragment, boolean setElevation) {
        supportInvalidateOptionsMenu();
        getSupportFragmentManager()
                .beginTransaction()
                .setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
                .replace(R.id.content_frame, navFragment)
                .commitNow();
        toolbar.setElevation(setElevation ? toolbarElevation : 0);
    }
}
