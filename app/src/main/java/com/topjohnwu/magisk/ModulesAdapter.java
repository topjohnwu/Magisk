package com.topjohnwu.magisk;

import android.content.Context;
import android.support.design.widget.Snackbar;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesAdapter extends RecyclerView.Adapter<ModulesAdapter.ViewHolder> {

    private final List<Module> mList;
    private View mView;
    private Context context;

    private Utils.ItemClickListener chboxListener, deleteBtnListener, unDeleteBtnListener;

    public ModulesAdapter(List<Module> list) {
        mList = list;
        chboxListener = (chk, position) -> {
            // On Checkbox change listener
            CheckBox chbox = (CheckBox) chk;

            if (!chbox.isChecked()) {
                mList.get(position).createDisableFile();
                Snackbar.make(mView, R.string.disable_file_created, Snackbar.LENGTH_SHORT).show();
            } else {
                mList.get(position).removeDisableFile();
                Snackbar.make(mView, R.string.disable_file_removed, Snackbar.LENGTH_SHORT).show();
            }
        };
        deleteBtnListener = (deleteBtn, position) -> {
            // On delete button click listener
            mList.get(position).createRemoveFile();
            Snackbar.make(mView, R.string.remove_file_created, Snackbar.LENGTH_SHORT).show();
        };
        unDeleteBtnListener = (undeleteBtn, position) -> {
            // On undelete button click listener
            mList.get(position).deleteRemoveFile();
            Snackbar.make(mView, R.string.remove_file_deleted, Snackbar.LENGTH_SHORT).show();
        };
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        mView = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_module, parent, false);
        context = parent.getContext();
        ButterKnife.bind(this, mView);
        return new ViewHolder(mView);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        final Module module = mList.get(position);
        if (module.isCache()) {
            holder.title.setText("[Cache] " + module.getName());
        } else {
            holder.title.setText(module.getName());
        }
        String author = module.getAuthor();
        String versionName = module.getVersion();
        String description = module.getDescription();
        if (versionName != null) {
            holder.versionName.setText(versionName);
        }
        if (author != null) {
            holder.author.setText(context.getString(R.string.author, author));
        }
        if (description != null) {
            holder.description.setText(description);
        }

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

    class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title) TextView title;
        @BindView(R.id.version_name) TextView versionName;
        @BindView(R.id.description) TextView description;
        @BindView(R.id.notice) TextView notice;
        @BindView(R.id.checkbox) CheckBox checkBox;
        @BindView(R.id.author) TextView author;
        @BindView(R.id.delete) ImageView delete;

        public ViewHolder(View itemView) {
            super(itemView);
            WindowManager windowmanager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            ButterKnife.bind(this, itemView);
            DisplayMetrics dimension = new DisplayMetrics();
            windowmanager.getDefaultDisplay().getMetrics(dimension);

            if (!Shell.rootAccess()) {
                checkBox.setEnabled(false);
                delete.setEnabled(false);
            }
        }
    }
}
