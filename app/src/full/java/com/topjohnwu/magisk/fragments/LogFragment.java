package com.topjohnwu.magisk.fragments;


import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.google.android.material.tabs.TabLayout;
import com.topjohnwu.magisk.MainActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.adapters.TabFragmentAdapter;
import com.topjohnwu.magisk.components.BaseFragment;

import androidx.viewpager.widget.ViewPager;
import butterknife.BindView;

public class LogFragment extends BaseFragment {

    @BindView(R.id.container) ViewPager viewPager;
    @BindView(R.id.tab) TabLayout tab;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_log, container, false);
        unbinder = new LogFragment_ViewBinding(this, v);

        ((MainActivity) requireActivity()).toolbar.setElevation(0);

        TabFragmentAdapter adapter = new TabFragmentAdapter(getChildFragmentManager());

        adapter.addTab(new SuLogFragment(), getString(R.string.superuser));
        adapter.addTab(new MagiskLogFragment(), getString(R.string.magisk));
        tab.setupWithViewPager(viewPager);
        tab.setVisibility(View.VISIBLE);

        viewPager.setAdapter(adapter);

        return v;
    }
}
