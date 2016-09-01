package com.topjohnwu.magisk;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.ModuleRepo;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

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

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title) TextView title;

        @BindView(R.id.version_name) TextView versionName;
        @BindView(R.id.description) TextView description;

        @BindView(R.id.warning) TextView warning;

        @BindView(R.id.checkbox) CheckBox checkBox;
        @BindView(R.id.delete) ImageView delete;

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