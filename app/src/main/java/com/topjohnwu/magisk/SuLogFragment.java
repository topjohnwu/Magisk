package com.topjohnwu.magisk;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.adapters.SuLogAdapter;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.superuser.SuLogEntry;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class SuLogFragment extends Fragment {

    @BindView(R.id.empty_rv) TextView emptyRv;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    private Unbinder unbinder;
    private MagiskManager magiskManager;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu_log, menu);
        menu.findItem(R.id.menu_save).setVisible(false);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_su_log, container, false);
        unbinder = ButterKnife.bind(this, v);
        magiskManager = getApplication();

        updateList();

        return v;
    }

    private void updateList() {
        List<SuLogEntry> logs = magiskManager.suDB.getLogList();

        if (logs.size() == 0) {
            emptyRv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            recyclerView.setAdapter(new SuLogAdapter(logs).getAdapter());
            emptyRv.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_refresh:
                updateList();
                return true;
            case R.id.menu_clear:
                magiskManager.suDB.clearLogs();
                updateList();
                return true;
            default:
                return true;
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
