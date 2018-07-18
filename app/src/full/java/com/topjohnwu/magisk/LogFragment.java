package com.topjohnwu.magisk;


import android.os.Bundle;
import android.support.design.widget.TabLayout;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.topjohnwu.magisk.adapters.TabFragmentAdapter;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.utils.Const;

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

        ((MainActivity) getActivity()).toolbar.setElevation(0);

        TabFragmentAdapter adapter = new TabFragmentAdapter(getChildFragmentManager());

        if (!(Const.USER_ID > 0 && getApplication().multiuserMode == Const.Value.MULTIUSER_MODE_OWNER_MANAGED)) {
            adapter.addTab(new SuLogFragment(), getString(R.string.superuser));
        }
        adapter.addTab(new MagiskLogFragment(), getString(R.string.magisk));
        tab.setupWithViewPager(viewPager);
        tab.setVisibility(View.VISIBLE);

        viewPager.setAdapter(adapter);

        return v;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }

}
