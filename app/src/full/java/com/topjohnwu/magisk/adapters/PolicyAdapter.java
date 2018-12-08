package com.topjohnwu.magisk.adapters;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.TextView;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.CustomAlertDialog;
import com.topjohnwu.magisk.components.ExpandableView;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.database.MagiskDB;
import com.topjohnwu.magisk.utils.FingerprintHelper;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import butterknife.BindView;

public class PolicyAdapter extends RecyclerView.Adapter<PolicyAdapter.ViewHolder> {

    private List<Policy> policyList;
    private MagiskDB dbHelper;
    private PackageManager pm;
    private Set<Policy> expandList = new HashSet<>();

    public PolicyAdapter(List<Policy> list, MagiskDB db, PackageManager pm) {
        policyList = list;
        dbHelper = db;
        this.pm = pm;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_policy, parent, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        Policy policy = policyList.get(position);

        holder.setExpanded(expandList.contains(policy));

        holder.itemView.setOnClickListener(view -> {
            if (holder.isExpanded()) {
                holder.collapse();
                expandList.remove(policy);
            } else {
                holder.expand();
                expandList.add(policy);
            }
        });

        holder.appName.setText(policy.appName);
        holder.packageName.setText(policy.packageName);
        holder.appIcon.setImageDrawable(policy.info.loadIcon(pm));

        holder.notificationSwitch.setOnCheckedChangeListener(null);
        holder.loggingSwitch.setOnCheckedChangeListener(null);

        holder.masterSwitch.setChecked(policy.policy == Policy.ALLOW);
        holder.notificationSwitch.setChecked(policy.notification);
        holder.loggingSwitch.setChecked(policy.logging);

        holder.masterSwitch.setOnClickListener(v -> {
            boolean isChecked = holder.masterSwitch.isChecked();
            Runnable r = () -> {
                if ((isChecked && policy.policy == Policy.DENY) ||
                        (!isChecked && policy.policy == Policy.ALLOW)) {
                    policy.policy = isChecked ? Policy.ALLOW : Policy.DENY;
                    String message = v.getContext().getString(
                            isChecked ? R.string.su_snack_grant : R.string.su_snack_deny, policy.appName);
                    SnackbarMaker.make(holder.itemView, message, Snackbar.LENGTH_SHORT).show();
                    dbHelper.updatePolicy(policy);
                }
            };
            if (FingerprintHelper.useFingerPrint()) {
                holder.masterSwitch.setChecked(!isChecked);
                FingerprintHelper.showAuthDialog((Activity) v.getContext(), () -> {
                    holder.masterSwitch.setChecked(isChecked);
                    r.run();
                });
            } else {
                r.run();
            }
        });
        holder.notificationSwitch.setOnCheckedChangeListener((v, isChecked) -> {
            if ((isChecked && !policy.notification) ||
                    (!isChecked && policy.notification)) {
                policy.notification = isChecked;
                String message = v.getContext().getString(
                        isChecked ? R.string.su_snack_notif_on : R.string.su_snack_notif_off, policy.appName);
                SnackbarMaker.make(holder.itemView, message, Snackbar.LENGTH_SHORT).show();
                dbHelper.updatePolicy(policy);
            }
        });
        holder.loggingSwitch.setOnCheckedChangeListener((v, isChecked) -> {
            if ((isChecked && !policy.logging) ||
                    (!isChecked && policy.logging)) {
                policy.logging = isChecked;
                String message = v.getContext().getString(
                        isChecked ? R.string.su_snack_log_on : R.string.su_snack_log_off, policy.appName);
                SnackbarMaker.make(holder.itemView, message, Snackbar.LENGTH_SHORT).show();
                dbHelper.updatePolicy(policy);
            }
        });
        holder.delete.setOnClickListener(v -> new CustomAlertDialog((Activity) v.getContext())
                .setTitle(R.string.su_revoke_title)
                .setMessage(v.getContext().getString(R.string.su_revoke_msg, policy.appName))
                .setPositiveButton(R.string.yes, (dialog, which) -> {
                    policyList.remove(position);
                    notifyItemRemoved(position);
                    notifyItemRangeChanged(position, policyList.size());
                    SnackbarMaker.make(holder.itemView, v.getContext().getString(R.string.su_snack_revoke, policy.appName),
                            Snackbar.LENGTH_SHORT).show();
                    dbHelper.deletePolicy(policy);
                })
                .setNegativeButton(R.string.no_thanks, null)
                .setCancelable(true)
                .show());

        // Hide for now
        holder.moreInfo.setVisibility(View.GONE);
    }

    @Override
    public int getItemCount() {
        return policyList.size();
    }

    static class ViewHolder extends RecyclerView.ViewHolder implements ExpandableView {

        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.package_name) TextView packageName;
        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.master_switch) Switch masterSwitch;
        @BindView(R.id.notification_switch) Switch notificationSwitch;
        @BindView(R.id.logging_switch) Switch loggingSwitch;
        @BindView(R.id.expand_layout) ViewGroup expandLayout;

        @BindView(R.id.delete) ImageView delete;
        @BindView(R.id.more_info) ImageView moreInfo;

        private Container container = new Container();

        public ViewHolder(View itemView) {
            super(itemView);
            new PolicyAdapter$ViewHolder_ViewBinding(this, itemView);
            container.expandLayout = expandLayout;
            setupExpandable();
        }

        @Override
        public Container getContainer() {
            return container;
        }
    }
}
