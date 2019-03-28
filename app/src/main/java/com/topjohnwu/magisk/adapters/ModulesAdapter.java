package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.uicomponents.SnackbarMaker;
import com.topjohnwu.superuser.Shell;

import java.util.List;

import butterknife.BindView;

public class ModulesAdapter extends RecyclerView.Adapter<ModulesAdapter.ViewHolder> {

    private final List<Module> mList;

    public ModulesAdapter(List<Module> list) {
        mList = list;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_module, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        Context context = holder.itemView.getContext();
        final Module module = mList.get(position);

        String version = module.getVersion();
        String author = module.getAuthor();
        String description = module.getDescription();
        String noInfo = context.getString(R.string.no_info_provided);

        holder.title.setText(module.getName());
        holder.versionName.setText(TextUtils.isEmpty(version) ? noInfo : version);
        holder.author.setText(TextUtils.isEmpty(author) ? noInfo : context.getString(R.string.author, author));
        holder.description.setText(TextUtils.isEmpty(description) ? noInfo : description);

        holder.checkBox.setOnCheckedChangeListener(null);
        holder.checkBox.setChecked(module.isEnabled());
        holder.checkBox.setOnCheckedChangeListener((v, isChecked) -> {
            int snack;
            if (isChecked) {
                module.removeDisableFile();
                snack = R.string.disable_file_removed;
            } else {
                module.createDisableFile();
                snack = R.string.disable_file_created;
            }
            SnackbarMaker.make(holder.itemView, snack, Snackbar.LENGTH_SHORT).show();
        });

        holder.delete.setOnClickListener(v -> {
            boolean removed = module.willBeRemoved();
            int snack;
            if (removed) {
                module.deleteRemoveFile();
                snack = R.string.remove_file_deleted;
            } else {
                module.createRemoveFile();
                snack = R.string.remove_file_created;
            }
            SnackbarMaker.make(holder.itemView, snack, Snackbar.LENGTH_SHORT).show();
            updateDeleteButton(holder, module);
        });

        if (module.isUpdated()) {
            holder.notice.setVisibility(View.VISIBLE);
            holder.notice.setText(R.string.update_file_created);
            holder.delete.setEnabled(false);
        } else {
            updateDeleteButton(holder, module);
        }
    }

    private void updateDeleteButton(ViewHolder holder, Module module) {
        holder.notice.setVisibility(module.willBeRemoved() ? View.VISIBLE : View.GONE);

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
        @BindView(R.id.notice) TextView notice;
        @BindView(R.id.checkbox) CheckBox checkBox;
        @BindView(R.id.author) TextView author;
        @BindView(R.id.delete) ImageView delete;

        ViewHolder(View itemView) {
            super(itemView);
            new ModulesAdapter$ViewHolder_ViewBinding(this, itemView);

            if (!Shell.rootAccess()) {
                checkBox.setEnabled(false);
                delete.setEnabled(false);
            }
        }
    }
}
