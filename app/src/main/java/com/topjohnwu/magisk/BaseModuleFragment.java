package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public abstract class BaseModuleFragment extends Fragment {

    @BindView(R.id.recyclerView)
    RecyclerView recyclerView;
    @BindView(R.id.empty_rv)
    TextView emptyTv;
    private View view;
    private SharedPreferences prefs;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.single_module_fragment, container, false);


        ButterKnife.bind(this, view);
        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        prefs.registerOnSharedPreferenceChangeListener(new SharedPreferences.OnSharedPreferenceChangeListener() {
            @Override
            public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String s) {
                if (s.contains("updated")) {
                    view.invalidate();
                    view.requestLayout();

                }
            }
        });
        if (listModules().size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);

            return view;
        }

        recyclerView.setAdapter(new ModulesAdapter(listModules(), new Utils.ItemClickListener() {
            @Override
            public void onItemClick(View chk, int position) {
                // On Checkbox change listener
                CheckBox chbox = (CheckBox) chk;

                if (!chbox.isChecked()) {
                    BaseModuleFragment.this.listModules().get(position).createDisableFile();
                    Snackbar.make(chk, R.string.disable_file_created, Snackbar.LENGTH_SHORT).show();
                } else {
                    BaseModuleFragment.this.listModules().get(position).removeDisableFile();
                    Snackbar.make(chk, R.string.disable_file_removed, Snackbar.LENGTH_SHORT).show();
                }
            }
        }, (deleteBtn, position) -> {
            // On delete button click listener

            listModules().get(position).createRemoveFile();
            Snackbar.make(deleteBtn, R.string.remove_file_created, Snackbar.LENGTH_SHORT).show();
        }, (undeleteBtn, position) -> {
            // On undelete button click listener

            listModules().get(position).deleteRemoveFile();
            Snackbar.make(undeleteBtn, R.string.remove_file_deleted, Snackbar.LENGTH_SHORT).show();
        }));
        return view;
    }


    protected abstract List<Module> listModules();
}
