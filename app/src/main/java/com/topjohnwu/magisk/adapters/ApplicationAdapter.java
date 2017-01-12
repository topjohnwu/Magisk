package com.topjohnwu.magisk.adapters;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.Filter;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ApplicationAdapter extends RecyclerView.Adapter<ApplicationAdapter.ViewHolder> {

    private List<ApplicationInfo> mOriginalList, mList;
    private List<String> mHideList;
    private PackageManager packageManager;
    private ApplicationFilter filter;

    public ApplicationAdapter(PackageManager packageManager) {
        mOriginalList = mList = Collections.emptyList();
        mHideList = Collections.emptyList();
        this.packageManager = packageManager;
    }

    public void setLists(List<ApplicationInfo> listApps, List<String> hideList) {
        mOriginalList = mList = Collections.unmodifiableList(listApps);
        mHideList = new ArrayList<>(hideList);
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

        holder.checkBox.setOnCheckedChangeListener(null);
        holder.checkBox.setChecked(mHideList.contains(info.packageName));
        holder.checkBox.setOnCheckedChangeListener((v, isChecked) -> {
            if (isChecked) {
                new Async.MagiskHide().add(info.packageName);
                mHideList.add(info.packageName);
            } else {
                new Async.MagiskHide().rm(info.packageName);
                mHideList.remove(info.packageName);
            }
        });
    }

    @Override
    public int getItemCount() {
        return mList.size();
    }

    public void filter(String constraint) {
        if (filter == null) {
            filter = new ApplicationFilter();
        }
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
