package com.topjohnwu.magisk.adapters;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.animation.Animation;
import android.view.animation.RotateAnimation;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.thoughtbot.expandablerecyclerview.ExpandableRecyclerViewAdapter;
import com.thoughtbot.expandablerecyclerview.models.ExpandableGroup;
import com.thoughtbot.expandablerecyclerview.viewholders.ChildViewHolder;
import com.thoughtbot.expandablerecyclerview.viewholders.GroupViewHolder;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.superuser.SuLogEntry;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import butterknife.BindView;
import butterknife.ButterKnife;

public class SuLogAdapter {

    private ExpandableAdapter adapter;
    private Set<SuLogEntry> expandList = new HashSet<>();

    public SuLogAdapter(List<SuLogEntry> list) {

        // Separate the logs with date
        Map<String, List<SuLogEntry>> logEntryMap = new LinkedHashMap<>();
        List<SuLogEntry> group;
        for (SuLogEntry log : list) {
            String date = log.getDateString();
            group = logEntryMap.get(date);
            if (group == null) {
                group = new ArrayList<>();
                logEntryMap.put(date, group);
            }
            group.add(log);
        }

        // Then format them into expandable groups
        List<LogGroup> logEntryGroups = new ArrayList<>();
        for (Map.Entry<String, List<SuLogEntry>> entry : logEntryMap.entrySet()) {
            logEntryGroups.add(new LogGroup(entry.getKey(), entry.getValue()));
        }
        adapter = new ExpandableAdapter(logEntryGroups);

    }

    public RecyclerView.Adapter getAdapter() {
        return adapter;
    }

    private class ExpandableAdapter
            extends ExpandableRecyclerViewAdapter<LogGroupViewHolder, LogViewHolder> {

        ExpandableAdapter(List<? extends ExpandableGroup> groups) {
            super(groups);
            expandableList.expandedGroupIndexes[0] = true;
        }

        @Override
        public LogGroupViewHolder onCreateGroupViewHolder(ViewGroup parent, int viewType) {
            View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_sulog_group, parent, false);
            return new LogGroupViewHolder(v);
        }

        @Override
        public LogViewHolder onCreateChildViewHolder(ViewGroup parent, int viewType) {
            View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_sulog, parent, false);
            return new LogViewHolder(v);
        }

        @Override
        public void onBindChildViewHolder(LogViewHolder holder, int flatPosition, ExpandableGroup group, int childIndex) {
            Context context = holder.itemView.getContext();
            SuLogEntry logEntry = (SuLogEntry) group.getItems().get(childIndex);
            holder.setExpanded(expandList.contains(logEntry));
            holder.itemView.setOnClickListener(view -> {
                if (holder.mExpanded) {
                    holder.collapse();
                    expandList.remove(logEntry);
                } else {
                    holder.expand();
                    expandList.add(logEntry);
                }
            });
            holder.appName.setText(logEntry.appName);
            holder.action.setText(context.getString(logEntry.action ? R.string.grant : R.string.deny));
            holder.command.setText(logEntry.command);
            holder.fromPid.setText(String.valueOf(logEntry.fromPid));
            holder.toUid.setText(String.valueOf(logEntry.toUid));
            holder.time.setText(logEntry.getTimeString());
        }

        @Override
        public void onBindGroupViewHolder(LogGroupViewHolder holder, int flatPosition, ExpandableGroup group) {
            holder.date.setText(group.getTitle());
            if (isGroupExpanded(flatPosition)) holder.expand();
        }
    }

    private class LogGroup extends ExpandableGroup<SuLogEntry> {
        LogGroup(String title, List<SuLogEntry> items) {
            super(title, items);
        }
    }

    static class LogGroupViewHolder extends GroupViewHolder {

        @BindView(R.id.date) TextView date;
        @BindView(R.id.arrow) ImageView arrow;

        public LogGroupViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }

        @Override
        public void expand() {
            RotateAnimation rotate =
                    new RotateAnimation(360, 180, Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
            rotate.setDuration(300);
            rotate.setFillAfter(true);
            arrow.setAnimation(rotate);
        }

        @Override
        public void collapse() {
            RotateAnimation rotate =
                    new RotateAnimation(180, 360, Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
            rotate.setDuration(300);
            rotate.setFillAfter(true);
            arrow.setAnimation(rotate);
        }
    }

    static class LogViewHolder extends ChildViewHolder {

        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.action) TextView action;
        @BindView(R.id.time) TextView time;
        @BindView(R.id.fromPid) TextView fromPid;
        @BindView(R.id.toUid) TextView toUid;
        @BindView(R.id.command) TextView command;
        @BindView(R.id.expand_layout) LinearLayout expandLayout;

        private ValueAnimator mAnimator;
        private boolean mExpanded = false;
        private static int expandHeight = 0;

        public LogViewHolder(View itemView) {
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
