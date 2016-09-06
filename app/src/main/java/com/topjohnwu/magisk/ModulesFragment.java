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
import android.support.v4.widget.SwipeRefreshLayout;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends Fragment {

    public static List<Module> listModules = new ArrayList<>();
    public static List<Module> listModulesCache = new ArrayList<>();
    public static List<Repo> listModulesDownload = new ArrayList<>();
    private static final int FILE_SELECT_CODE = 0;
    private TabsAdapter ta;
    private File input;
    private SwipeRefreshLayout mSwipeRefreshLayout;

    @BindView(R.id.progressBar) ProgressBar progressBar;
    @BindView(R.id.fab) FloatingActionButton fabio;
    @BindView(R.id.pager) ViewPager viewPager;
    @BindView(R.id.tab_layout) TabLayout tabLayout;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);

        ButterKnife.bind(this, view);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        if (prefs.contains("hasCachedRepos")) {
            new Utils.LoadModules(getActivity(), false).execute();
        } else {
            new Utils.LoadModules(getActivity(), true).execute();
        }

        new updateUI().execute();
        setHasOptionsMenu(true);
        return view;
    }

    public void updateThisShit() {
        new Utils.LoadModules(getActivity(), true).execute();
        new updateUI().execute();
        setHasOptionsMenu(true);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.menu_module, menu);
        fabio.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                Intent intent = new Intent();
                intent.setType("*/zip");
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent,FILE_SELECT_CODE);
            }


        });

    }



    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case FILE_SELECT_CODE:
                if (resultCode == Activity.RESULT_OK) {
                    // Get the Uri of the selected file
                    Uri uri = data.getData();
                    String path = uri.getPath();
                    String fileName = uri.getLastPathSegment();
                    new Utils.FlashZIP(getActivity(), fileName, path).execute();

                }
                break;
        }}

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.force_reload:
                listModules.clear();
                listModulesCache.clear();
                listModulesDownload.clear();
                progressBar.setVisibility(View.VISIBLE);
                ta = new TabsAdapter(getChildFragmentManager());
                viewPager.setAdapter(ta);
                tabLayout.setupWithViewPager(viewPager);
                new Utils.LoadModules(getActivity(),true).execute();
                new updateUI().execute();
                break;
        }

        return super.onOptionsItemSelected(item);
    }

    public void redrawLayout() {
        listModules.clear();
        listModulesCache.clear();
        listModulesDownload.clear();
        progressBar.setVisibility(View.VISIBLE);
        ta = new TabsAdapter(getChildFragmentManager());
        viewPager.setAdapter(ta);
        tabLayout.setupWithViewPager(viewPager);
        new Utils.LoadModules(getActivity(),false).execute();
        new updateUI().execute();

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
    public static class DownloadModuleFragment extends BaseRepoFragment {

        @Override
        protected List<Repo> listRepos() {
            return listModulesDownload;
        }

    }


    public class updateUI extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);

            progressBar.setVisibility(View.GONE);
            ta = new TabsAdapter(getChildFragmentManager());
            viewPager.setAdapter(ta);
            tabLayout.setupWithViewPager(viewPager);
        }
    }

    private class TabsAdapter extends FragmentPagerAdapter {

        String[] tabTitles = new String[]{
                getString(R.string.modules), getString(R.string.cache_modules) ,"Download"
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
            } else if (position == 1) {
                return new CacheModuleFragment();
            } else {
                return new DownloadModuleFragment();
            }
        }
    }

}
