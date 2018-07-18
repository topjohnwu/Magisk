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

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class SuLogFragment extends Fragment {

    @BindView(R.id.empty_rv) TextView emptyRv;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    private Unbinder unbinder;
    private MagiskManager mm;
    private SuLogAdapter adapter;

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
        mm = getApplication();
        adapter = new SuLogAdapter(mm.mDB);
        recyclerView.setAdapter(adapter);

        updateList();

        return v;
    }

    private void updateList() {
        adapter.notifyDBChanged();

        if (adapter.getSectionCount() == 0) {
            emptyRv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
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
                mm.mDB.clearLogs();
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
