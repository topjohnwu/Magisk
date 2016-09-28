package com.topjohnwu.magisk;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Color;
import android.net.Uri;
import android.os.AsyncTask;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebWindow;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

    private final List<Repo> mList;
    private List<Boolean> mExpandedList;
    private View mView;
    private Context context;

    public ReposAdapter(List<Repo> list) {
        mList = list;
        mExpandedList = new ArrayList<>(mList.size());
        for (int i = 0; i < mList.size(); i++) {
            mExpandedList.add(false);
        }
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        mView = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_repo, parent, false);
        ButterKnife.bind(this, mView);
        context = parent.getContext();
        return new ViewHolder(mView);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        final Repo repo = mList.get(position);
        mExpandedList = new ArrayList<>(mList.size());
        for (int i = 0; i < mList.size(); i++) {
            mExpandedList.add(false);
        }
        if (repo.isCache()) {
            holder.title.setText("[Cache] " + repo.getName());
        } else {
            holder.title.setText(repo.getName());
        }
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
        if (repo.isInstalled()) {
            holder.installedStatus.setText(context.getString(R.string.module_installed));
            holder.installedStatus.setTextColor(Color.parseColor("#14AD00"));
            holder.updateStatus.setText(repo.canUpdate() ? context.getString(R.string.module_update_available) : context.getString(R.string.module_up_to_date));
        } else {
            holder.installedStatus.setText(context.getString(R.string.module_not_installed));
        }

        View.OnClickListener listener = view -> {
            if (view.getId() == holder.updateImage.getId()) {
                Utils.downloadAndReceive(
                        context,
                        new DownloadReceiver(repo.getName() + "-" + repo.getVersion()) {
                            @Override
                            public void task(Uri uri) {
                                new Async.FlashZIP(context, uri, mName) {
                                    @Override
                                    protected void preProcessing() throws Throwable {
                                        super.preProcessing();
                                        new File(mUri.getPath()).delete();
                                        Shell.su(
                                                "cd " + mFile.getParent(),
                                                "mkdir git",
                                                "unzip -o install.zip -d git",
                                                "mv git/* install",
                                                "cd install",
                                                "rm -rf system/placeholder",
                                                "chmod 644 $(find . -type f)",
                                                "chmod 755 $(find . -type d)",
                                                "rm -rf ../install.zip ../git",
                                                "zip -r ../install.zip *",
                                                "rm -rf ../install"
                                        );
                                    }
                                }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                            }
                        },
                        repo.getZipUrl(),
                        repo.getId().replace(" ", "") + ".zip");
            }
            if ((view.getId() == holder.changeLog.getId()) && (!repo.getLogUrl().equals(""))) {
                new WebWindow("Changelog", repo.getLogUrl(), context);
            }
            if ((view.getId() == holder.authorLink.getId()) && (!repo.getSupportUrl().equals(""))) {
                new WebWindow("Donate", repo.getDonateUrl(), context);
            }
            if ((view.getId() == holder.supportLink.getId()) && (!repo.getSupportUrl().equals(""))) {
                new WebWindow("Support", repo.getSupportUrl(), context);
            }
        };

        holder.changeLog.setOnClickListener(listener);
        holder.updateImage.setOnClickListener(listener);
        holder.authorLink.setOnClickListener(listener);
        holder.supportLink.setOnClickListener(listener);
    }

    @Override
    public int getItemCount() {
        return mList.size();
    }

    class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title) TextView title;
        @BindView(R.id.version_name) TextView versionName;
        @BindView(R.id.description) TextView description;
        @BindView(R.id.author) TextView author;
        @BindView(R.id.installedStatus) TextView installedStatus;
        @BindView(R.id.updateStatus) TextView updateStatus;
        @BindView(R.id.expand_layout) LinearLayout expandLayout;
        @BindView(R.id.update) ImageView updateImage;
        @BindView(R.id.installed) ImageView installedImage;
        @BindView(R.id.changeLog) ImageView changeLog;
        @BindView(R.id.authorLink) ImageView authorLink;
        @BindView(R.id.supportLink) ImageView supportLink;

        private ValueAnimator mAnimator;
        private ObjectAnimator animY2;
        private ViewHolder holder;

        public ViewHolder(View itemView) {
            super(itemView);
            WindowManager windowmanager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            ButterKnife.bind(this, itemView);
            DisplayMetrics dimension = new DisplayMetrics();
            windowmanager.getDefaultDisplay().getMetrics(dimension);
            holder = this;
            this.expandLayout.getViewTreeObserver().addOnPreDrawListener(
                    new ViewTreeObserver.OnPreDrawListener() {

                        @Override
                        public boolean onPreDraw() {
                            final int widthSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                            final int heightSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                            holder.expandLayout.getViewTreeObserver().removeOnPreDrawListener(this);
                            holder.expandLayout.setVisibility(View.GONE);
                            holder.expandLayout.measure(widthSpec, heightSpec);
                            final int holderHeight = holder.expandLayout.getMeasuredHeight();
                            mAnimator = slideAnimator(0, holderHeight);
                            animY2 = ObjectAnimator.ofFloat(holder.updateImage, "translationY", holderHeight / 2);

                            return true;
                        }

                    });

            mView.setOnClickListener(view -> {
                int position = getAdapterPosition();
                if (mExpandedList.get(position)) {
                    collapse(holder.expandLayout);
                } else {
                    expand(holder.expandLayout);
                }
                mExpandedList.set(position, !mExpandedList.get(position));
            });

        }

        private void expand(View view) {
            view.setVisibility(View.VISIBLE);
            mAnimator.start();
            animY2.start();

        }

        private void collapse(View view) {
            int finalHeight = view.getHeight();
            ValueAnimator mAnimator = slideAnimator(finalHeight, 0);
            mAnimator.addListener(new Animator.AnimatorListener() {
                @Override
                public void onAnimationEnd(Animator animator) {
                    view.setVisibility(View.GONE);
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

        }

        private ValueAnimator slideAnimator(int start, int end) {

            ValueAnimator animator = ValueAnimator.ofInt(start, end);

            animator.addUpdateListener(valueAnimator -> {
                int value = (Integer) valueAnimator.getAnimatedValue();
                ViewGroup.LayoutParams layoutParams = expandLayout
                        .getLayoutParams();
                layoutParams.height = value;
                expandLayout.setLayoutParams(layoutParams);
            });
            return animator;
        }

    }
}
