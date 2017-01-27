package com.topjohnwu.magisk;


import android.app.Fragment;
import android.os.Bundle;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.adapters.SuLogAdapter;
import com.topjohnwu.magisk.superuser.SuLogDatabaseHelper;
import com.topjohnwu.magisk.superuser.SuLogEntry;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class SuLogFragment extends Fragment {

    @BindView(R.id.empty_rv) TextView emptyRv;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    private Unbinder unbinder;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_su_log, container, false);
        unbinder = ButterKnife.bind(this, v);

        SuLogDatabaseHelper dbHelper = new SuLogDatabaseHelper(getActivity());
        List<SuLogEntry> logs = dbHelper.getLogList();

        if (logs.size() == 0) {
            emptyRv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            recyclerView.setAdapter(new SuLogAdapter(logs).getAdapter());
            emptyRv.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
        }

        return v;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
