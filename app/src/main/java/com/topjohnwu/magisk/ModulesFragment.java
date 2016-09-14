package com.topjohnwu.magisk;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.provider.DocumentsContract;
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

    public static List<Module> listModules = new ArrayList<>();
    public static List<Module> listModulesCache = new ArrayList<>();
    private int viewPagePosition;

    @BindView(R.id.progressBar)
    ProgressBar progressBar;
    @BindView(R.id.fab)
    FloatingActionButton fabio;
    @BindView(R.id.pager)
    ViewPager viewPager;
    @BindView(R.id.tab_layout)
    TabLayout tabLayout;
    private RepoHelper.TaskDelegate mTaskDelegate;
    private static final int RQS_OPEN_DOCUMENT_TREE = 2;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);

        ButterKnife.bind(this, view);
        fabio.setOnClickListener(v -> {
            Intent fileintent = new Intent(Intent.ACTION_GET_CONTENT);
            fileintent.setType("application/zip");
            startActivityForResult(fileintent, RQS_OPEN_DOCUMENT_TREE);


        });

        new Utils.LoadModules(getActivity(), false).execute();
        mTaskDelegate = result -> {
            if (result.equals("OK")) {
                Log.d("Magisk", "ModulesFragment: We dun got the result, hur hur.");
                RefreshUI();
            }

        };

        new updateUI().execute();
        return view;
    }


    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        String file = "";
        if (resultCode == Activity.RESULT_OK && requestCode == RQS_OPEN_DOCUMENT_TREE) {
            if (isExternalStorageDocument(data.getData())) {
                final String docId = DocumentsContract.getDocumentId(data.getData());
                final String[] split = docId.split(":");
                final String type = split[0];

                if ("primary".equalsIgnoreCase(type)) {
                    file = Environment.getExternalStorageDirectory() + "/" + split[1];
                }

                // TODO handle non-primary volumes
            }
            String shit = data.getDataString();

            Log.d("Magisk", "ModulesFragment: Got a result, " + shit + " and " + data.getData().getAuthority() + " and " + file);

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
        new Utils.LoadModules(getActivity(), true).execute();
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
