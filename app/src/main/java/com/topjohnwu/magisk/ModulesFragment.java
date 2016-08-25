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
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;
import java.util.concurrent.ExecutionException;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends Fragment {

    @BindView(R.id.progressBar) ProgressBar progressBar;
    @BindView(R.id.pager) ViewPager viewPager;
    @BindView(R.id.tab_layout) TabLayout tabLayout;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);
        ButterKnife.bind(this, view);

        new CheckFolders().execute();

        setHasOptionsMenu(true);
        return view;
    }

    public static class NormalModuleFragment extends BaseModuleFragment {

        @Override
        protected List<Module> listModules() {
            return Utils.listModules;
        }

    }

    public static class CacheModuleFragment extends BaseModuleFragment {

        @Override
        protected List<Module> listModules() {
            return Utils.listModulesCache;
        }

    }

    private class CheckFolders extends AsyncTask<Void, Integer, Void> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            progressBar.setVisibility(View.VISIBLE);
        }

        @Override
        protected Void doInBackground(Void... voids) {
            // Ensure initialize is done
            try {
                Utils.initialize.get();
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
