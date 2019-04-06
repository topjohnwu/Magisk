package com.topjohnwu.magisk.adapters;

import android.app.Activity;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.widget.SwitchCompat;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.database.MagiskDB;
import com.topjohnwu.magisk.dialogs.CustomAlertDialog;
import com.topjohnwu.magisk.dialogs.FingerprintAuthDialog;
import com.topjohnwu.magisk.uicomponents.ArrowExpandable;
import com.topjohnwu.magisk.uicomponents.Expandable;
import com.topjohnwu.magisk.uicomponents.ExpandableViewHolder;
import com.topjohnwu.magisk.uicomponents.SnackbarMaker;
import com.topjohnwu.magisk.utils.FingerprintHelper;

import java.util.List;

import butterknife.BindView;

public class PolicyAdapter extends RecyclerView.Adapter<PolicyAdapter.ViewHolder> {

    private List<Policy> policyList;
    private MagiskDB dbHelper;
    private PackageManager pm;
    private boolean[] expandList;

    public PolicyAdapter(List<Policy> list, MagiskDB db, PackageManager pm) {
        policyList = list;
        expandList = new boolean[policyList.size()];
        dbHelper = db;
        this.pm = pm;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_policy, parent, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        Policy policy = policyList.get(position);

        holder.settings.setExpanded(expandList[position]);
        holder.trigger.setOnClickListener(view -> {
            if (holder.settings.isExpanded()) {
                holder.settings.collapse();
                expandList[position] = false;
            } else {
                holder.settings.expand();
                expandList[position] = true;
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
            if (FingerprintHelper.useFingerprint()) {
                holder.masterSwitch.setChecked(!isChecked);
                new FingerprintAuthDialog((Activity) v.getContext(), () -> {
                    holder.masterSwitch.setChecked(isChecked);
                    r.run();
                }).show();
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
        holder.delete.setOnClickListener(v -> {
            DialogInterface.OnClickListener l = (dialog, which) -> {
                policyList.remove(position);
                notifyItemRemoved(position);
                notifyItemRangeChanged(position, policyList.size());
                SnackbarMaker.make(holder.itemView, v.getContext().getString(R.string.su_snack_revoke, policy.appName),
                        Snackbar.LENGTH_SHORT).show();
                dbHelper.deletePolicy(policy);
            };
            if (FingerprintHelper.useFingerprint()) {
                new FingerprintAuthDialog((Activity) v.getContext(),
                        () -> l.onClick(null, 0)).show();
            } else {
                new CustomAlertDialog((Activity) v.getContext())
                        .setTitle(R.string.su_revoke_title)
                        .setMessage(v.getContext().getString(R.string.su_revoke_msg, policy.appName))
                        .setPositiveButton(R.string.yes, l)
                        .setNegativeButton(R.string.no_thanks, null)
                        .setCancelable(true)
                        .show();
            }
        });
    }

    @Override
    public int getItemCount() {
        return policyList.size();
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.package_name) TextView packageName;
        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.master_switch) SwitchCompat masterSwitch;
        @BindView(R.id.notification_switch) SwitchCompat notificationSwitch;
        @BindView(R.id.logging_switch) SwitchCompat loggingSwitch;
        @BindView(R.id.expand_layout) ViewGroup expandLayout;
        @BindView(R.id.arrow) ImageView arrow;
        @BindView(R.id.trigger) View trigger;
        @BindView(R.id.delete) ImageView delete;
        @BindView(R.id.more_info) ImageView moreInfo;

        Expandable settings;

        public ViewHolder(View itemView) {
            super(itemView);
            new PolicyAdapter$ViewHolder_ViewBinding(this, itemView);
            settings = new ArrowExpandable(new ExpandableViewHolder(expandLayout), arrow);
        }
    }
}
