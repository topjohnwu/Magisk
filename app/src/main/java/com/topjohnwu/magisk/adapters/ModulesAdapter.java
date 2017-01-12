package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.support.design.widget.Snackbar;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Shell;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesAdapter extends RecyclerView.Adapter<ModulesAdapter.ViewHolder> {

    private final List<Module> mList;

    public ModulesAdapter(List<Module> list) {
        mList = list;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_module, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        Context context = holder.itemView.getContext();
        final Module module = mList.get(position);

        holder.title.setText(module.getName());
        holder.versionName.setText(module.getVersion());
        String author = module.getAuthor();
        holder.author.setText(TextUtils.isEmpty(author) ? null : context.getString(R.string.author, author));
        holder.description.setText(module.getDescription());

        holder.checkBox.setOnCheckedChangeListener(null);
        holder.checkBox.setChecked(module.isEnabled());
        holder.checkBox.setOnCheckedChangeListener((v, isChecked) -> new Async.RootTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                if (isChecked) {
                    module.removeDisableFile();
                } else {
                    module.createDisableFile();
                }
                return null;
            }

            @Override
            protected void onPostExecute(Void v) {
                int title = isChecked ? R.string.disable_file_removed : R.string.disable_file_created;
                Snackbar.make(holder.title, title, Snackbar.LENGTH_SHORT).show();
            }
        }.exec());

        holder.delete.setOnClickListener(v -> new Async.RootTask<Void, Void, Void>() {
            private final boolean removed = module.willBeRemoved();

            @Override
            protected Void doInBackground(Void... voids) {
                if (removed) {
                    module.deleteRemoveFile();
                } else {
                    module.createRemoveFile();
                }
                return null;
            }

            @Override
            protected void onPostExecute(Void v) {
                int title = removed ? R.string.remove_file_deleted : R.string.remove_file_created;
                Snackbar.make(holder.title, title, Snackbar.LENGTH_SHORT).show();
                updateDeleteButton(holder, module);
            }
        }.exec());

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
            ButterKnife.bind(this, itemView);

            if (!Shell.rootAccess()) {
                checkBox.setEnabled(false);
                delete.setEnabled(false);
            }
        }
    }
}
