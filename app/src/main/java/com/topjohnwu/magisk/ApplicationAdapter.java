package com.topjohnwu.magisk;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.Async;

import java.util.Arrays;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ApplicationAdapter extends RecyclerView.Adapter<ApplicationAdapter.ViewHolder> {

    private List<ApplicationInfo> mList;
    private List<String> mHideList;
    private Context context;
    private PackageManager packageManager;

    // Don't show in list...
    private static final String[] blackList = {
            "com.topjohnwu.magisk",
            "com.google.android.gms"
    };

    public ApplicationAdapter(List<ApplicationInfo> list, List<String> hideList) {
        mList = list;
        mHideList = hideList;
        List<String> bl = Arrays.asList(blackList);
        for (int i = 0; i < mList.size(); ++i)
            if (bl.contains(mList.get(i).packageName))
                mList.remove(i);
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View mView = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_app, parent, false);
        context = parent.getContext();
        packageManager = context.getPackageManager();
        ButterKnife.bind(this, mView);
        return new ViewHolder(mView);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        ApplicationInfo info = mList.get(position);
        holder.appIcon.setImageDrawable(info.loadIcon(packageManager));
        holder.appName.setText(info.loadLabel(packageManager));
        holder.appPackage.setText(info.packageName);
        holder.checkBox.setChecked(false);
        for (String hidePackage : mHideList) {
            if (info.packageName.contains(hidePackage)) {
                holder.checkBox.setChecked(true);
                break;
            }
        }
        holder.checkBox.setOnCheckedChangeListener((compoundButton, b) -> {
            Async.MagiskHide mh = new Async.MagiskHide();
            if (b) {
                mh.add(info.packageName);
            } else {
                mh.rm(info.packageName);
            }
        });
    }

    @Override
    public int getItemCount() {
        return mList.size();
    }

    class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.app_package) TextView appPackage;
        @BindView(R.id.checkbox) CheckBox checkBox;

        public ViewHolder(View itemView) {
            super(itemView);
            WindowManager windowmanager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            ButterKnife.bind(this, itemView);
            DisplayMetrics dimension = new DisplayMetrics();
            windowmanager.getDefaultDisplay().getMetrics(dimension);
        }
    }
}