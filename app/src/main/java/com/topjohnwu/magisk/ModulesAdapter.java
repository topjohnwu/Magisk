package com.topjohnwu.magisk;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.support.v7.app.AlertDialog;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebWindow;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesAdapter extends RecyclerView.Adapter<ModulesAdapter.ViewHolder> {

    //@BindView(R.id.expand_layout) LinearLayout expandedLayout;

    private final List<Module> mList;
    private View viewMain;
    private Context context;
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
        viewMain = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_module, parent, false);
        context = parent.getContext();
        ButterKnife.bind(this, viewMain);
        return new ViewHolder(viewMain);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        final Module module = mList.get(position);
        Log.d("Magisk", "ModulesAdapter: Trying set up bindview from list pos " + position + " and " + module.getName());
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
        @BindView(R.id.author) TextView author;
//        @BindView(R.id.updateStatus) TextView updateStatus;
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
