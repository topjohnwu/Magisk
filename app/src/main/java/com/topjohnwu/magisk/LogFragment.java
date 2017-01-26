package com.topjohnwu.magisk;


import android.app.Fragment;
import android.app.FragmentManager;
import android.os.Bundle;
import android.support.design.widget.TabLayout;
import android.support.v13.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class LogFragment extends Fragment {

    private Unbinder unbinder;

    @BindView(R.id.container) ViewPager viewPager;
    @BindView(R.id.tab) TabLayout tab;


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_log, container, false);
        unbinder = ButterKnife.bind(this, v);

        ViewPagerAdapter adapter = new ViewPagerAdapter(getChildFragmentManager());

        adapter.addTab(new MagiskLogFragment(), getString(R.string.magisk));

        if (Global.Info.isSuClient) {
            adapter.addTab(new SuLogFragment(), getString(R.string.superuser));
            tab.setupWithViewPager(viewPager);
            tab.setVisibility(View.VISIBLE);
        }

        viewPager.setAdapter(adapter);

        return v;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }

    private class ViewPagerAdapter extends FragmentPagerAdapter {

        List<Fragment> fragmentList;
        List<String> titleList;

        public ViewPagerAdapter(FragmentManager fm) {
            super(fm);
            fragmentList = new ArrayList<>();
            titleList = new ArrayList<>();
        }

        @Override
        public Fragment getItem(int position) {
            return fragmentList.get(position);
        }

        @Override
        public int getCount() {
            return fragmentList.size();
        }

        @Override
        public CharSequence getPageTitle(int position) {
            return titleList.get(position);
        }

        public void addTab(Fragment fragment, String title) {
            fragmentList.add(fragment);
            titleList.add(title);
        }
    }



}
