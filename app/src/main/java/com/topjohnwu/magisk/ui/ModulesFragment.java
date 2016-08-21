package com.topjohnwu.magisk.ui;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.design.widget.TabLayout;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.model.Module;
import com.topjohnwu.magisk.ui.utils.Utils;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends android.support.v4.app.Fragment {

    private static final String MAGISK_PATH = "/magisk";
    private static final String MAGISK_CACHE_PATH = "/cache/magisk";

    private static List<Module> listModulesNoCache = new ArrayList<>();
    private static List<Module> listModulesCache = new ArrayList<>();

    @BindView(R.id.progressBar) ProgressBar progressBar;
    @BindView(R.id.pager) ViewPager viewPager;
    @BindView(R.id.tab_layout) TabLayout tabLayout;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        listModulesCache.clear();
        listModulesNoCache.clear();
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);
        ButterKnife.bind(this, view);

        viewPager.setAdapter(new TabsAdapter(getChildFragmentManager()));
        tabLayout.setupWithViewPager(viewPager);

        new CheckFolders().execute();

        return view;
    }

    public static class NoCacheModuleFragment extends BaseModuleFragment {

        @Override
        protected List<Module> listModules() {
            return listModulesNoCache;
        }

    }

    public static class CacheModuleFragment extends BaseModuleFragment {

        @Override
        protected List<Module> listModules() {
            return listModulesCache;
        }

    }

    private class CheckFolders extends AsyncTask<Void, Integer, Boolean> {

        @Override
        protected Boolean doInBackground(Void... voids) {
            File[] magisk = new File(MAGISK_PATH).listFiles(new FileFilter() {
                @Override
                public boolean accept(File file) {
                    return file.isDirectory();
                }
            });

            Utils.executeCommand("chmod 777 /cache");

            File[] magiskCache = new File(MAGISK_CACHE_PATH).listFiles(new FileFilter() {
                @Override
                public boolean accept(File file) {
                    return file.isDirectory();
                }
            });

            if (magisk != null) {
                for (File mod : magisk) {
                    Module m = new Module(mod);
                    if (m.isValid()) {
                        try {
                            m.parse();
                            listModulesNoCache.add(m);
                        } catch (Exception ignored) {
                        }
                    }
                }
            }

            if (magiskCache != null) {
                for (File mod : magiskCache) {
                    Module m = new Module(mod);
                    if (m.isValid()) {
                        try {
                            m.parse();
                            listModulesCache.add(m);
                        } catch (Exception ignored) {
                        }
                    }
                }
            }

            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            super.onPostExecute(result);

            progressBar.setVisibility(View.GONE);
        }
    }

    private class TabsAdapter extends FragmentPagerAdapter {

        String[] tabTitles = new String[]{
                "_no_cache", "_cache"
                // TODO stringify
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
                return new NoCacheModuleFragment();
            } else {
                return new CacheModuleFragment();
            }
        }
    }
}
