package com.topjohnwu.magisk;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.TabLayout;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.Toast;

import com.nbsp.materialfilepicker.MaterialFilePicker;
import com.nbsp.materialfilepicker.ui.FilePickerActivity;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.regex.Pattern;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends Fragment {

    public static List<Module> listModules = new ArrayList<>();
    public static List<Module> listModulesCache = new ArrayList<>();
    private static final int FILE_SELECT_CODE = 0;
    private int viewPagePosition;
    private static final int RESULT_OK = 1;

    @BindView(R.id.progressBar)
    ProgressBar progressBar;
    @BindView(R.id.fab)
    FloatingActionButton fabio;
    @BindView(R.id.pager)
    ViewPager viewPager;
    @BindView(R.id.tab_layout)
    TabLayout tabLayout;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);

        ButterKnife.bind(this, view);
        new Utils.LoadModules(getActivity(), false).execute();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        new updateUI().execute();
        setHasOptionsMenu(true);
        return view;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.menu_module, menu);
        fabio.setOnClickListener(view -> {
            openFilePicker();
        });

    }

    private void openFilePicker() {
        new MaterialFilePicker()
                .withSupportFragment(this)
                .withFilter(Pattern.compile(".*\\.zip$"))
                .withRequestCode(FILE_SELECT_CODE)
                .withHiddenFiles(true)
                .start();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.d("Magisk","WelcomeActivity: Got an OK result" + resultCode);
        if (resultCode == Activity.RESULT_OK) {
            String path = data.getStringExtra(FilePickerActivity.RESULT_FILE_PATH);
            Log.d("Magisk","ModuleFragment: Got an OK result " + path);
            if (path != null) {
                Log.d("Path: ", path);
                Toast.makeText(getActivity(), "Picked file: " + path, Toast.LENGTH_LONG).show();
                // Get the Uri of the selected file
                String filePath = data.getStringExtra(FilePickerActivity.RESULT_FILE_PATH);

                Uri uri = Uri.parse(filePath);

                path = uri.getPath();
                Log.d("Magisk","ModuleFragment: Got an OK result " + filePath + " and " + uri.toString() + " and " + path);

                String fileName = uri.getLastPathSegment();
                new Utils.FlashZIP(getActivity(), fileName, path).execute();
            }
        }
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.force_reload:
                viewPagePosition = tabLayout.getSelectedTabPosition();
                listModules.clear();
                listModulesCache.clear();
                progressBar.setVisibility(View.VISIBLE);
                viewPager.setAdapter(new TabsAdapter(getChildFragmentManager()));
                tabLayout.setupWithViewPager(viewPager);
                viewPager.setCurrentItem(viewPagePosition);
                new Utils.LoadModules(getActivity(), true).execute();
                Collections.sort(listModules,new CustomComparator());
                Collections.sort(listModulesCache,new CustomComparator());
                new updateUI().execute();
                break;
        }

        return super.onOptionsItemSelected(item);
    }

    void selectPage(int pageIndex) {
        tabLayout.setScrollPosition(pageIndex, 0f, true);
        viewPager.setCurrentItem(pageIndex);
    }

    public static class NormalModuleFragment extends BaseModuleFragment {

        @Override
        protected List<Module> listModules() {
            return listModules;
        }

    }

    public static class CacheModuleFragment extends BaseModuleFragment {

        @Override
        protected List<Module> listModules() {
            return listModulesCache;
        }

    }


    private class updateUI extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);

            progressBar.setVisibility(View.GONE);
            viewPager.setAdapter(new TabsAdapter(getChildFragmentManager()));
            tabLayout.setupWithViewPager(viewPager);
            selectPage(viewPagePosition);

        }
    }

    private class TabsAdapter extends FragmentPagerAdapter {

        String[] tabTitles = new String[]{
                getString(R.string.modules), getString(R.string.cache_modules)
        };

        public TabsAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public int getCount() {
            return tabTitles.length;
        }

        @Override
        public String getPageTitle(int position) {
            return tabTitles[position];
        }

        @Override
        public Fragment getItem(int position) {
            if (position == 0) {
                return new NormalModuleFragment();
            } else {
                return new CacheModuleFragment();
            }
        }
    }
    public class CustomComparator implements Comparator<Module> {
        @Override
        public int compare(Module o1, Module o2) {
            return o1.getName().compareTo(o2.getName());
        }
    }

}
