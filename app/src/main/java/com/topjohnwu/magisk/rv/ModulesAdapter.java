package com.topjohnwu.magisk.rv;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.model.Module;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesAdapter extends RecyclerView.Adapter<ModulesAdapter.ViewHolder> {

    private final List<Module> mList;
    private final ItemClickListener chboxListener;
    private final ItemClickListener deleteBtnListener;

    public ModulesAdapter(List<Module> list, ItemClickListener chboxListener, ItemClickListener deleteBtnListener) {
        this.mList = list;
        this.chboxListener = chboxListener;
        this.deleteBtnListener = deleteBtnListener;
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
        holder.checkBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                chboxListener.onItemClick(compoundButton, holder.getAdapterPosition());
            }
        });

        holder.delete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                deleteBtnListener.onItemClick(holder.delete, holder.getAdapterPosition());
                holder.warning.setVisibility(module.willBeRemoved() ? View.VISIBLE : View.GONE);
            }
        });
        holder.delete.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View view) {
                module.deleteRemoveFile();
                holder.warning.setVisibility(module.willBeRemoved() ? View.VISIBLE : View.GONE);

                return false;
            }
        });

        holder.warning.setVisibility(module.willBeRemoved() ? View.VISIBLE : View.GONE);
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
        }
    }
}