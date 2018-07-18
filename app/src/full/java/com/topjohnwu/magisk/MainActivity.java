package com.topjohnwu.magisk;

import android.content.Intent;
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
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MainActivity extends Activity
        implements NavigationView.OnNavigationItemSelectedListener, Topic.Subscriber {

    private final Handler mDrawerHandler = new Handler();
    private int mDrawerItem;
    private boolean fromShortcut = true;

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.drawer_layout) DrawerLayout drawer;
    @BindView(R.id.nav_view) public NavigationView navigationView;

    private float toolbarElevation;

    @Override
    public int getDarkTheme() {
        return R.style.AppTheme_Dark;
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {

        MagiskManager mm = getMagiskManager();

        if (!mm.hasInit) {
            Intent intent = new Intent(this, SplashActivity.class);
            String section = getIntent().getStringExtra(Const.Key.OPEN_SECTION);
            if (section != null) {
                intent.putExtra(Const.Key.OPEN_SECTION, section);
            }
            startActivity(intent);
            finish();
        }

        String perm = getIntent().getStringExtra(Const.Key.INTENT_PERM);
        if (perm != null) {
            ActivityCompat.requestPermissions(this, new String[] { perm }, 0);
        }

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);

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

        if (savedInstanceState == null)
            navigate(getIntent().getStringExtra(Const.Key.OPEN_SECTION));

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
    public void onTopicPublished(Topic topic) {
        recreate();
    }

    @Override
    public Topic[] getSubscription() {
        return new Topic[] { getMagiskManager().reloadActivity };
    }

    public void checkHideSection() {
        MagiskManager mm = getMagiskManager();
        Menu menu = navigationView.getMenu();
        menu.findItem(R.id.magiskhide).setVisible(
                Shell.rootAccess() && mm.magiskVersionCode >= Const.MAGISK_VER.UNIFIED
                        && mm.prefs.getBoolean(Const.Key.MAGISKHIDE, false));
        menu.findItem(R.id.modules).setVisible(!mm.prefs.getBoolean(Const.Key.COREONLY, false) &&
                Shell.rootAccess() && mm.magiskVersionCode >= 0);
        menu.findItem(R.id.downloads).setVisible(!mm.prefs.getBoolean(Const.Key.COREONLY, false)
                && Utils.checkNetworkStatus() && Shell.rootAccess() && mm.magiskVersionCode >= 0);
        menu.findItem(R.id.log).setVisible(Shell.rootAccess());
        menu.findItem(R.id.superuser).setVisible(Shell.rootAccess() &&
                !(Const.USER_ID > 0 && mm.multiuserMode == Const.Value.MULTIUSER_MODE_OWNER_MANAGED));
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
                startActivity(new Intent(this, SettingsActivity.class));
                mDrawerItem = bak;
                break;
            case R.id.app_about:
                startActivity(new Intent(this, AboutActivity.class));
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
