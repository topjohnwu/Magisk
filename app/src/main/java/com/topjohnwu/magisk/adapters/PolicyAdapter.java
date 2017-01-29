package com.topjohnwu.magisk.adapters;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.pm.PackageManager;
import android.support.design.widget.Snackbar;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.superuser.Policy;
import com.topjohnwu.magisk.superuser.SuDatabaseHelper;
import com.topjohnwu.magisk.utils.Utils;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import butterknife.BindView;
import butterknife.ButterKnife;

public class PolicyAdapter extends RecyclerView.Adapter<PolicyAdapter.ViewHolder> {

    private List<Policy> policyList;
    private SuDatabaseHelper dbHelper;
    private PackageManager pm;
    private Set<Policy> expandList = new HashSet<>();

    public PolicyAdapter(List<Policy> list, SuDatabaseHelper db, PackageManager pm) {
        policyList = list;
        dbHelper = db;
        this.pm = pm;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_policy, parent, false);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        Policy policy = policyList.get(position);
        try {
            holder.setExpanded(expandList.contains(policy));

            holder.itemView.setOnClickListener(view -> {
                if (holder.mExpanded) {
                    holder.collapse();
                    expandList.remove(policy);
                } else {
                    holder.expand();
                    expandList.add(policy);
                }
            });

            holder.appName.setText(policy.appName);
            holder.packageName.setText(policy.packageName);
            holder.appIcon.setImageDrawable(pm.getPackageInfo(policy.packageName, 0).applicationInfo.loadIcon(pm));
            holder.masterSwitch.setOnCheckedChangeListener((v, isChecked) -> {
                if ((isChecked && policy.policy == Policy.DENY) ||
                        (!isChecked && policy.policy == Policy.ALLOW)) {
                    policy.policy = isChecked ? Policy.ALLOW : Policy.DENY;
                    String message = v.getContext().getString(
                            isChecked ? R.string.su_snack_grant : R.string.su_snack_deny, policy.appName);
                    Snackbar.make(holder.itemView, message, Snackbar.LENGTH_SHORT).show();
                    dbHelper.addPolicy(policy);
                }
            });
            holder.notificationSwitch.setOnCheckedChangeListener((v, isChecked) -> {
                if ((isChecked && !policy.notification) ||
                        (!isChecked && policy.notification)) {
                    policy.notification = isChecked;
                    String message = v.getContext().getString(
                            isChecked ? R.string.su_snack_notif_on : R.string.su_snack_notif_off, policy.appName);
                    Snackbar.make(holder.itemView, message, Snackbar.LENGTH_SHORT).show();
                    dbHelper.addPolicy(policy);
                }
            });
            holder.loggingSwitch.setOnCheckedChangeListener((v, isChecked) -> {
                if ((isChecked && !policy.logging) ||
                        (!isChecked && policy.logging)) {
                    policy.logging = isChecked;
                    String message = v.getContext().getString(
                            isChecked ? R.string.su_snack_log_on : R.string.su_snack_log_off, policy.appName);
                    Snackbar.make(holder.itemView, message, Snackbar.LENGTH_SHORT).show();
                    dbHelper.addPolicy(policy);
                }
            });
            holder.delete.setOnClickListener(v -> Utils.getAlertDialogBuilder(v.getContext())
                    .setTitle(R.string.su_revoke_title)
                    .setMessage(v.getContext().getString(R.string.su_revoke_msg, policy.appName))
                    .setPositiveButton(R.string.yes, (dialog, which) -> {
                        policyList.remove(position);
                        notifyItemRemoved(position);
                        notifyItemRangeChanged(position, policyList.size());
                        Snackbar.make(holder.itemView, v.getContext().getString(R.string.su_snack_revoke, policy.appName),
                                Snackbar.LENGTH_SHORT).show();
                        dbHelper.deletePolicy(policy.uid);
                    })
                    .setNegativeButton(R.string.no_thanks, null)
                    .setCancelable(true)
                    .show());
            holder.masterSwitch.setChecked(policy.policy == Policy.ALLOW);
            holder.notificationSwitch.setChecked(policy.notification);
            holder.loggingSwitch.setChecked(policy.logging);

            // Hide for now
            holder.moreInfo.setVisibility(View.GONE);

        } catch (PackageManager.NameNotFoundException e) {
            policyList.remove(position);
            dbHelper.deletePolicy(policy.uid);
            onBindViewHolder(holder, position);
        }
    }

    @Override
    public int getItemCount() {
        return policyList.size();
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.package_name) TextView packageName;
        @BindView(R.id.expand_layout) LinearLayout expandLayout;
        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.master_switch) Switch masterSwitch;
        @BindView(R.id.notification_switch) Switch notificationSwitch;
        @BindView(R.id.logging_switch) Switch loggingSwitch;

        @BindView(R.id.delete) ImageView delete;
        @BindView(R.id.more_info) ImageView moreInfo;

        private ValueAnimator mAnimator;
        private boolean mExpanded = false;
        private static int expandHeight = 0;

        ViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
            expandLayout.getViewTreeObserver().addOnPreDrawListener(
                    new ViewTreeObserver.OnPreDrawListener() {

                        @Override
                        public boolean onPreDraw() {
                            if (expandHeight == 0) {
                                final int widthSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                                final int heightSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                                expandLayout.measure(widthSpec, heightSpec);
                                expandHeight = expandLayout.getMeasuredHeight();
                            }

                            expandLayout.getViewTreeObserver().removeOnPreDrawListener(this);
                            expandLayout.setVisibility(View.GONE);
                            mAnimator = slideAnimator(0, expandHeight);
                            return true;
                        }

                    });
        }

        private void setExpanded(boolean expanded) {
            mExpanded = expanded;
            ViewGroup.LayoutParams layoutParams = expandLayout.getLayoutParams();
            layoutParams.height = expanded ? expandHeight : 0;
            expandLayout.setLayoutParams(layoutParams);
            expandLayout.setVisibility(expanded ? View.VISIBLE : View.GONE);
        }

        private void expand() {
            expandLayout.setVisibility(View.VISIBLE);
            mAnimator.start();
            mExpanded = true;
        }

        private void collapse() {
            if (!mExpanded) return;
            int finalHeight = expandLayout.getHeight();
            ValueAnimator mAnimator = slideAnimator(finalHeight, 0);
            mAnimator.addListener(new Animator.AnimatorListener() {
                @Override
                public void onAnimationEnd(Animator animator) {
                    expandLayout.setVisibility(View.GONE);
                }

                @Override
                public void onAnimationStart(Animator animator) {}

                @Override
                public void onAnimationCancel(Animator animator) {}

                @Override
                public void onAnimationRepeat(Animator animator) {}
            });
            mAnimator.start();
            mExpanded = false;
        }

        private ValueAnimator slideAnimator(int start, int end) {

            ValueAnimator animator = ValueAnimator.ofInt(start, end);

            animator.addUpdateListener(valueAnimator -> {
                int value = (Integer) valueAnimator.getAnimatedValue();
                ViewGroup.LayoutParams layoutParams = expandLayout.getLayoutParams();
                layoutParams.height = value;
                expandLayout.setLayoutParams(layoutParams);
            });
            return animator;
        }

    }
}
