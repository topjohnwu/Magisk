package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
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
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.Toast;

import com.ipaulpro.afilechooser.FileInfo;
import com.ipaulpro.afilechooser.utils.FileUtils;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.RepoHelper;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends Fragment {

    private static final int FETCH_ZIP_CODE = 2;
    public static List<Module> listModules = new ArrayList<>();
    public static List<Module> listModulesCache = new ArrayList<>();
    @BindView(R.id.progressBar)
    ProgressBar progressBar;
    @BindView(R.id.fab)
    FloatingActionButton fabio;
    @BindView(R.id.pager)
    ViewPager viewPager;
    @BindView(R.id.tab_layout)
    TabLayout tabLayout;
    private int viewPagePosition;
    private RepoHelper.TaskDelegate mTaskDelegate;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);

        ButterKnife.bind(this, view);
        fabio.setOnClickListener(v -> {
            Intent getContentIntent = FileUtils.createGetContentIntent(null);
            getContentIntent.setType("application/zip");
            Intent fileIntent = Intent.createChooser(getContentIntent, "Select a file");

            startActivityForResult(fileIntent, FETCH_ZIP_CODE);

        });

        new Utils.LoadModules(getActivity()).execute();
        mTaskDelegate = result -> {
            if (result.equals("OK")) {
                RefreshUI();
            }

        };

        new updateUI().execute();
        return view;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (data != null) {
            // Get the URI of the selected file
            final Uri uri = data.getData();
            Log.i("Magisk", "ModulesFragment: Uri = " + uri.toString() + " or ");
            new Utils.FlashZIP(getActivity(), uri).execute();
            try {
                // Get the file path from the URI
                FileInfo fileInfo = FileUtils.getFileInfo(getActivity(), uri);
                Toast.makeText(getActivity(),
                        "File Selected: " + fileInfo.getDisplayName() + " size: " + fileInfo.getSize(), Toast.LENGTH_LONG).show();

                if (!fileInfo.isExternal()) {

                } else {

                }
            } catch (Exception e) {
                Log.e("FileSelectorTestAc...", "File select error", e);
            }
        }

    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.menu_module, menu);

    }

    private void RefreshUI() {
        viewPagePosition = tabLayout.getSelectedTabPosition();
        listModules.clear();
        listModulesCache.clear();
        progressBar.setVisibility(View.VISIBLE);
        viewPager.setAdapter(new TabsAdapter(getChildFragmentManager()));
        tabLayout.setupWithViewPager(viewPager);
        viewPager.setCurrentItem(viewPagePosition);
        new Utils.LoadModules(getActivity()).execute();
        Collections.sort(listModules, new CustomComparator());
        Collections.sort(listModulesCache, new CustomComparator());
        new updateUI().execute();
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
                NormalModuleFragment nmf = new NormalModuleFragment();
                nmf.SetDelegate(mTaskDelegate);
                return nmf;
            } else {
                CacheModuleFragment cmf = new CacheModuleFragment();
                cmf.SetDelegate(mTaskDelegate);
                return cmf;
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