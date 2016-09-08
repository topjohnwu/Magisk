package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public abstract class BaseModuleFragment extends Fragment {

    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyTv;
    

    private SharedPreferences prefs;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.single_module_fragment, container, false);


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

        recyclerView.setAdapter(new ModulesAdapter(listModules(), (chk, position) -> {
            // On Checkbox change listener
            CheckBox chbox = (CheckBox) chk;

            if (!chbox.isChecked()) {
                listModules().get(position).createDisableFile();
                Snackbar.make(chk, R.string.disable_file_created, Snackbar.LENGTH_SHORT).show();
            } else {
                listModules().get(position).removeDisableFile();
                Snackbar.make(chk, R.string.disable_file_removed, Snackbar.LENGTH_SHORT).show();
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

    public class ModulesAdapter extends RecyclerView.Adapter<ModulesAdapter.ViewHolder> {

        private final List<Module> mList;
        private final Utils.ItemClickListener chboxListener;
        private final Utils.ItemClickListener deleteBtnListener;
        private final Utils.ItemClickListener unDeleteBtnListener;

        public ModulesAdapter(List<Module> list, Utils.ItemClickListener chboxListener, Utils.ItemClickListener deleteBtnListener, Utils.ItemClickListener undeleteBtnListener) {
            this.mList = list;
            this.chboxListener = chboxListener;
            this.deleteBtnListener = deleteBtnListener;
            this.unDeleteBtnListener = undeleteBtnListener;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_module, parent, false);

            return new ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(final ViewHolder holder, int position) {
            final Module module = mList.get(position);
            Log.d("Magisk","ModulesAdapter: Trying set up bindview from list pos " + position + " and " + module.getName() );

            holder.title.setText(module.getName());
            holder.versionName.setText(module.getVersion());
            holder.description.setText(module.getDescription());

            holder.checkBox.setChecked(module.isEnabled());
            holder.checkBox.setOnCheckedChangeListener((compoundButton, b) -> chboxListener.onItemClick(compoundButton, holder.getAdapterPosition()));

            holder.delete.setOnClickListener(view -> {
                if (module.willBeRemoved()) {
                    unDeleteBtnListener.onItemClick(holder.delete, holder.getAdapterPosition());
                } else {
                    deleteBtnListener.onItemClick(holder.delete, holder.getAdapterPosition());
                }

                updateDeleteButton(holder, module);
            });

            updateDeleteButton(holder, module);
        }

        private void updateDeleteButton(ViewHolder holder, Module module) {
            holder.warning.setVisibility(module.willBeRemoved() ? View.VISIBLE : View.GONE);

            if (module.willBeRemoved()) {
                holder.delete.setImageResource(R.drawable.ic_undelete);
            } else {
                holder.delete.setImageResource(R.drawable.ic_delete);
            }
        }

        @Override
        public int getItemCount() {
            return mList.size();
        }

        class ViewHolder extends RecyclerView.ViewHolder {

            @BindView(R.id.title) TextView title;

            @BindView(R.id.version_name) TextView versionName;
            @BindView(R.id.description) TextView description;

            @BindView(R.id.warning) TextView warning;

            @BindView(R.id.checkbox) CheckBox checkBox;
            @BindView(R.id.delete)
            ImageView delete;

            public ViewHolder(View itemView) {
                super(itemView);
                ButterKnife.bind(this, itemView);
                if (!Shell.rootAccess()) {
                    checkBox.setEnabled(false);
                    delete.setEnabled(false);
                }
            }
        }
    }
}
