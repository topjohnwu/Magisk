package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.support.design.widget.Snackbar;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.Filter;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.ParallelTask;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ApplicationAdapter extends RecyclerView.Adapter<ApplicationAdapter.ViewHolder> {

    private List<ApplicationInfo> mOriginalList, mList;
    private List<String> mHideList;
    private PackageManager pm;
    private ApplicationFilter filter;
    private Topic magiskHideDone;

    public ApplicationAdapter(Context context) {
        mOriginalList = mList = Collections.emptyList();
        mHideList = Collections.emptyList();
        filter = new ApplicationFilter();
        pm = context.getPackageManager();
        magiskHideDone = Utils.getMagiskManager(context).magiskHideDone;
        new LoadApps().exec();
    }

    private boolean lowercaseContains(CharSequence string, CharSequence nonNullLowercaseSearch) {
        return !TextUtils.isEmpty(string) && string.toString().toLowerCase().contains(nonNullLowercaseSearch);
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View mView = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_app, parent, false);
        return new ViewHolder(mView);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        ApplicationInfo info = mList.get(position);

        holder.appIcon.setImageDrawable(info.loadIcon(pm));
        holder.appName.setText(info.loadLabel(pm));
        holder.appPackage.setText(info.packageName);

        // Remove all listeners
        holder.itemView.setOnClickListener(null);
        holder.checkBox.setOnCheckedChangeListener(null);

        if (Const.SN_DEFAULTLIST.contains(info.packageName)) {
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
                    Shell.su_raw("magiskhide --add " + info.packageName);
                    mHideList.add(info.packageName);
                } else {
                    Shell.su_raw("magiskhide --rm " + info.packageName);
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

    public void refresh() {
        new LoadApps().exec();
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.package_name) TextView appPackage;
        @BindView(R.id.checkbox) CheckBox checkBox;

        ViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }
    }

    private class ApplicationFilter extends Filter {

        @Override
        protected FilterResults performFiltering(CharSequence constraint) {
            if (constraint == null || constraint.length() == 0) {
                mList = mOriginalList;
            } else {
                mList = new ArrayList<>();
                String filter = constraint.toString().toLowerCase();
                for (ApplicationInfo info : mOriginalList) {
                    if (lowercaseContains(info.loadLabel(pm), filter)
                            || lowercaseContains(info.packageName, filter)) {
                        mList.add(info);
                    }
                }
            }
            return null;
        }

        @Override
        protected void publishResults(CharSequence constraint, FilterResults results) {
            notifyDataSetChanged();
        }
    }

    private class LoadApps extends ParallelTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            mOriginalList = pm.getInstalledApplications(0);
            for (Iterator<ApplicationInfo> i = mOriginalList.iterator(); i.hasNext(); ) {
                ApplicationInfo info = i.next();
                if (Const.HIDE_BLACKLIST.contains(info.packageName) || !info.enabled) {
                    i.remove();
                }
            }
            Collections.sort(mOriginalList, (a, b) -> a.loadLabel(pm).toString().toLowerCase()
                    .compareTo(b.loadLabel(pm).toString().toLowerCase()));
            mHideList = Shell.su("magiskhide --ls");
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            magiskHideDone.publish(false);
        }
    }
}
