package com.topjohnwu.magisk.adapters;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.receivers.RepoDlReceiver;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebWindow;

import java.util.HashSet;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

    private List<Repo> mUpdateRepos, mInstalledRepos, mOthersRepos;
    private HashSet<Repo> expandList = new HashSet<>();

    public ReposAdapter(List<Repo> update, List<Repo> installed, List<Repo> others) {
        mUpdateRepos = update;
        mInstalledRepos = installed;
        mOthersRepos = others;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_repo, parent, false);
        ButterKnife.bind(this, v);
        return new ViewHolder(v);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        Context context = holder.itemView.getContext();
        Repo repo = getItem(position);

        holder.title.setText(repo.getName());
        String author = repo.getAuthor();
        String versionName = repo.getVersion();
        String description = repo.getDescription();
        if (versionName != null) {
            holder.versionName.setText(versionName);
        }
        if (author != null) {
            holder.author.setText(context.getString(R.string.author, author));
        }
        if (description != null) {
            holder.description.setText(description);
        }

        holder.setExpanded(expandList.contains(repo));

        holder.itemView.setOnClickListener(view -> {
            if (holder.mExpanded) {
                holder.collapse();
                expandList.remove(repo);
            } else {
                holder.expand();
                expandList.add(repo);
            }
        });
        holder.changeLog.setOnClickListener(view -> {
            if (!TextUtils.isEmpty(repo.getLogUrl())) {
                new WebWindow(context.getString(R.string.changelog), repo.getLogUrl(), context);
            }
        });
        holder.updateImage.setOnClickListener(view -> {
            String filename = repo.getName() + "-" + repo.getVersion() + ".zip";
            Utils.getAlertDialogBuilder(context)
                    .setTitle(context.getString(R.string.repo_install_title, repo.getName()))
                    .setMessage(context.getString(R.string.repo_install_msg, filename))
                    .setCancelable(true)
                    .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.dlAndReceive(
                            context,
                            new RepoDlReceiver(),
                            repo.getZipUrl(),
                            Utils.getLegalFilename(filename)))
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        });
        holder.authorLink.setOnClickListener(view -> {
            if (!TextUtils.isEmpty(repo.getDonateUrl())) {
                context.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(repo.getDonateUrl())));
            }
        });
        holder.supportLink.setOnClickListener(view -> {
            if (!TextUtils.isEmpty(repo.getSupportUrl())) {
                context.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(repo.getSupportUrl())));
            }
        });
    }

    @Override
    public int getItemCount() {
        return mUpdateRepos.size() + mInstalledRepos.size() + mOthersRepos.size();
    }

    private Repo getItem(int position) {
        if (position >= mUpdateRepos.size()) {
            position -= mUpdateRepos.size();
            if (position >= mInstalledRepos.size()) {
                position -= mInstalledRepos.size();
                return mOthersRepos.get(position);
            } else {
                return mInstalledRepos.get(position);
            }
        } else {
            return mUpdateRepos.get(position);
        }
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title) TextView title;
        @BindView(R.id.version_name) TextView versionName;
        @BindView(R.id.description) TextView description;
        @BindView(R.id.author) TextView author;
        @BindView(R.id.expand_layout) LinearLayout expandLayout;
        @BindView(R.id.update) ImageView updateImage;
        @BindView(R.id.changeLog) ImageView changeLog;
        @BindView(R.id.authorLink) ImageView authorLink;
        @BindView(R.id.supportLink) ImageView supportLink;

        private ValueAnimator mAnimator;
        private ObjectAnimator animY2;
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
                            animY2 = ObjectAnimator.ofFloat(updateImage, "translationY", expandHeight / 2);
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
            if (expanded) {
                updateImage.setTranslationY(expandHeight / 2);
            } else {
                updateImage.setTranslationY(0);
            }
        }

        private void expand() {
            expandLayout.setVisibility(View.VISIBLE);
            mAnimator.start();
            animY2.start();
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
            animY2.reverse();
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
