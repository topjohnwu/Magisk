package com.topjohnwu.magisk.ui.base;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;

import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ui.MainActivity;
import com.topjohnwu.magisk.ui.home.MagiskFragment;
import com.topjohnwu.magisk.ui.log.LogFragment;
import com.topjohnwu.magisk.ui.module.ModulesFragment;
import com.topjohnwu.magisk.ui.settings.SettingsFragment;
import com.topjohnwu.magisk.ui.superuser.SuperuserFragment;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;
import butterknife.BindView;

public class NavigationFragment extends BaseFragment implements
        BottomNavigationView.OnNavigationItemSelectedListener {

    private static final String KEY_CURRENT_PAGE = "currentPage";
    private static final String KEY_FROM_SHORTCUTS = "fromShortcuts";

    @BindView(R.id.bottom_nav) BottomNavigationView bottomNavigationView;

    private boolean fromShortcuts = false;
    private int currentPage = R.id.magisk;

    private Fragment magiskFragment = new MagiskFragment();
    private Fragment superuserFragment = new SuperuserFragment();
    private Fragment modulesFragment = new ModulesFragment();
    private Fragment logFragment = new LogFragment();
    private Fragment settingsFragment = new SettingsFragment();

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Bundle args = getArguments();
        if (savedInstanceState != null) {
            fromShortcuts = savedInstanceState.getBoolean(KEY_FROM_SHORTCUTS);
            currentPage = savedInstanceState.getInt(KEY_CURRENT_PAGE);
        } else if (args != null) {
            String section = args.getString(Const.Key.OPEN_SECTION);
            fromShortcuts = section != null;
            currentPage = getItemId(section);
        }
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_navigation, container, false);
        unbinder = new NavigationFragment_ViewBinding(this, v);
        return v;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        bottomNavigationView.setOnNavigationItemSelectedListener(this);
        bottomNavigationView.setSelectedItemId(currentPage);

        navigate(currentPage);
        checkHideSection();
    }

    public void checkHideSection() {
        if (bottomNavigationView == null) {
            return;
        }

        boolean superuserVisible = Utils.showSuperUser();
        boolean modulesVisible = Shell.rootAccess() && Config.magiskVersionCode >= 0;
        boolean logVisible = Shell.rootAccess();

        Menu menu = bottomNavigationView.getMenu();
        menu.findItem(R.id.superuser).setVisible(superuserVisible);
        menu.findItem(R.id.modules).setVisible(modulesVisible);
        menu.findItem(R.id.log).setVisible(logVisible);
    }

    @Override
    public boolean onBackPressed() {
        if (!fromShortcuts && currentPage != R.id.magisk) {
            bottomNavigationView.setSelectedItemId(R.id.magisk);
            return true;
        }
        return false;
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull MenuItem menuItem) {
        int itemId = menuItem.getItemId();
        if (itemId != currentPage) {
            currentPage = itemId;
            fromShortcuts = false;
            navigate(itemId);
        }
        return true;
    }

    private int getItemId(@Nullable String section) {
        if (section == null) {
            return R.id.magisk;
        }
        switch (section) {
            case "superuser":
                return R.id.superuser;
            case "modules":
                return R.id.modules;
            case "log":
                return R.id.log;
            case "settings":
                return R.id.settings;
            default:
                return R.id.magisk;
        }
    }

    private void navigate(int itemId) {
        ((MainActivity) requireActivity()).setElevation(itemId != R.id.log);
        switch (itemId) {
            case R.id.magisk:
                displayFragment(magiskFragment);
                break;
            case R.id.superuser:
                displayFragment(superuserFragment);
                break;
            case R.id.modules:
                displayFragment(modulesFragment);
                break;
            case R.id.log:
                displayFragment(logFragment);
                break;
            case R.id.settings:
                displayFragment(settingsFragment);
                break;
        }
    }

    private void displayFragment(Fragment fragment) {
        requireActivity().invalidateOptionsMenu();
        getChildFragmentManager()
                .beginTransaction()
                .setTransition(FragmentTransaction.TRANSIT_FRAGMENT_FADE)
                .replace(R.id.content_frame, fragment)
                .commitNow();
    }

    @Override
    public void onSaveInstanceState(@NonNull Bundle outState) {
        super.onSaveInstanceState(outState);

        outState.putBoolean(KEY_FROM_SHORTCUTS, fromShortcuts);
        outState.putInt(KEY_CURRENT_PAGE, currentPage);
    }

    public static Fragment newInstance(String section) {
        Bundle args = new Bundle();
        args.putString(Const.Key.OPEN_SECTION, section);

        Fragment fragment = new NavigationFragment();
        fragment.setArguments(args);
        return fragment;
    }
}
