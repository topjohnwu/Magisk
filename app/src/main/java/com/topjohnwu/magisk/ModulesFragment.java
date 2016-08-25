package com.topjohnwu.magisk;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.design.widget.TabLayout;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends Fragment {

    private static final String MAGISK_PATH = "/magisk";
    private static final String MAGISK_CACHE_PATH = "/cache/magisk";

    private static List<Module> listModules = new ArrayList<>();
    private static List<Module> listModulesCache = new ArrayList<>();

    public static loadModules loadMod;

    @BindView(R.id.progressBar) ProgressBar progressBar;
    @BindView(R.id.pager) ViewPager viewPager;
    @BindView(R.id.tab_layout) TabLayout tabLayout;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);
        ButterKnife.bind(this, view);

        new updateUI().execute();

        setHasOptionsMenu(true);
        return view;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.force_reload:
                loadMod = new loadModules();
                loadMod.execute();
                new updateUI().execute();
                break;
        }

        return super.onOptionsItemSelected(item);
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

    public static class loadModules extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {

            listModules.clear();
            listModulesCache.clear();

            listModules.clear();
            listModulesCache.clear();
            List<String> magisk = Utils.getModList(MAGISK_PATH);
            List<String> magiskCache = Utils.getModList(MAGISK_CACHE_PATH);
            if (!magisk.isEmpty()) {
                for (String mod : magisk) {
                    listModules.add(new Module(mod));
                }
            }

            if (!magiskCache.isEmpty()) {
                for (String mod : magiskCache) {
                    listModulesCache.add(new Module(mod));
                }
            }

            return null;
        }
    }

    private class updateUI extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            // Ensure loadMod is done
            try {
                loadMod.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }

            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);

            progressBar.setVisibility(View.GONE);

            viewPager.setAdapter(new TabsAdapter(getChildFragmentManager()));
            tabLayout.setupWithViewPager(viewPager);
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
}
