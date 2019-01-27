package com.topjohnwu.magisk.ui;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.MenuItem;

import androidx.annotation.NonNull;
import androidx.appcompat.widget.Toolbar;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;

import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ui.base.BaseActivity;
import com.topjohnwu.magisk.ui.base.BaseFragment;
import com.topjohnwu.magisk.ui.base.NavigationFragment;

import butterknife.BindView;

public class MainActivity extends BaseActivity
        implements PreferenceFragmentCompat.OnPreferenceStartFragmentCallback {

    @BindView(R.id.toolbar) public Toolbar toolbar;

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
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.content_frame, NavigationFragment.newInstance(section), "main")
                    .commit();
        }
    }

    @Override
    public void onBackPressed() {
        Fragment fragment = getSupportFragmentManager().findFragmentById(R.id.content_frame);
        if (fragment instanceof BaseFragment && ((BaseFragment) fragment).onBackPressed()) {
            return;
        }
        int entryCount = getSupportFragmentManager().getBackStackEntryCount();
        if (entryCount > 0) {
            getSupportFragmentManager().popBackStack();
            setHasBack(entryCount > 1);
        } else {
            finish();
        }
    }

    public void checkHideSection() {
        Fragment fragment = getSupportFragmentManager().findFragmentByTag("main");
        if (fragment instanceof NavigationFragment) {
            ((NavigationFragment) fragment).checkHideSection();
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
    }

    public void setElevation(boolean hasElevation) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            toolbar.setElevation(hasElevation ? toolbarElevation : 0);
        }
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
