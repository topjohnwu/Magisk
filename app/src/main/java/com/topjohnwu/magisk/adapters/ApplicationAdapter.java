package com.topjohnwu.magisk.adapters;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.support.design.widget.Snackbar;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.Filter;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.MagiskHide;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ApplicationAdapter extends RecyclerView.Adapter<ApplicationAdapter.ViewHolder> {

    public static final List<String> BLACKLIST =  Arrays.asList(
            "android",
            "com.topjohnwu.magisk",
            "com.google.android.gms"
    );

    private static final List<String> SNLIST =  Arrays.asList(
            "com.google.android.apps.walletnfcrel",
            "com.nianticlabs.pokemongo"
    );

    private List<ApplicationInfo> mOriginalList, mList;
    private List<String> mHideList;
    private PackageManager packageManager;
    private ApplicationFilter filter;

    public ApplicationAdapter(PackageManager packageManager) {
        mOriginalList = mList = Collections.emptyList();
        mHideList = Collections.emptyList();
        this.packageManager = packageManager;
        filter = new ApplicationFilter();
    }

    public void setLists(List<ApplicationInfo> listApps, List<String> hideList) {
        mOriginalList = mList = listApps;
        mHideList = hideList;
        notifyDataSetChanged();
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View mView = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_app, parent, false);
        return new ViewHolder(mView);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        ApplicationInfo info = mList.get(position);

        holder.appIcon.setImageDrawable(info.loadIcon(packageManager));
        holder.appName.setText(info.loadLabel(packageManager));
        holder.appPackage.setText(info.packageName);

        // Remove all listeners
        holder.itemView.setOnClickListener(null);
        holder.checkBox.setOnCheckedChangeListener(null);

        if (SNLIST.contains(info.packageName)) {
            holder.checkBox.setChecked(true);
            holder.checkBox.setEnabled(false);
            holder.itemView.setOnClickListener(v ->
                SnackbarMaker.make(holder.itemView,
                        R.string.safetyNet_hide_notice, Snackbar.LENGTH_LONG).show()
            );
        } else {
            holder.checkBox.setEnabled(true);
            holder.checkBox.setChecked(mHideList.contains(info.packageName));
            holder.checkBox.setOnCheckedChangeListener((v, isChecked) -> {
                if (isChecked) {
                    new MagiskHide().add(info.packageName);
                    mHideList.add(info.packageName);
                } else {
                    new MagiskHide().rm(info.packageName);
                    mHideList.remove(info.packageName);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return mList.size();
    }

    public void filter(String constraint) {
        filter.filter(constraint);
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.app_package) TextView appPackage;
        @BindView(R.id.checkbox) CheckBox checkBox;

        ViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }
    }

    private class ApplicationFilter extends Filter {

        @Override
        protected FilterResults performFiltering(CharSequence constraint) {
            List<ApplicationInfo> filteredApps;
            if (constraint == null || constraint.length() == 0) {
                filteredApps = mOriginalList;
            } else {
                filteredApps = new ArrayList<>();
                String filter = constraint.toString().toLowerCase();
                for (ApplicationInfo info : mOriginalList) {
                    if (Utils.lowercaseContains(info.loadLabel(packageManager), filter)
                            || Utils.lowercaseContains(info.packageName, filter)) {
                        filteredApps.add(info);
                    }
                }
            }

            FilterResults results = new FilterResults();
            results.values = filteredApps;
            results.count = filteredApps.size();
            return results;
        }

        @SuppressWarnings("unchecked")
        @Override
        protected void publishResults(CharSequence constraint, FilterResults results) {
            mList = (List<ApplicationInfo>) results.values;
            notifyDataSetChanged();
        }
    }
}
