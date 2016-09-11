package com.topjohnwu.magisk;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.preference.PreferenceManager;
import android.support.v7.widget.RecyclerView;
import android.text.util.Linkify;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebWindow;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

    private ReposFragment reposFragment;
    private final List<Repo> mList;
    List<Boolean> mExpandedList;
    @BindView(R.id.update)
    ImageView updateImage;
    @BindView(R.id.installed)
    ImageView installedImage;
    @BindView(R.id.expand_layout)
    LinearLayout expandedLayout;
    private View viewMain;
    private Context context;
    private boolean mIsInstalled, mCanUpdate;
    private Repo repo;
    private ViewHolder mHolder;

    public ReposAdapter(ReposFragment reposFragment, List<Repo> list) {
        this.reposFragment = reposFragment;
        this.mList = list;
        Log.d("Magisk", "ReposAdapter: I am alive. I have a list " + list.size());
        mExpandedList = new ArrayList<>(mList.size());
        for (int i = 0; i < mList.size(); i++) {
            mExpandedList.add(false);
        }

    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        viewMain = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_repo, parent, false);
        ButterKnife.bind(this, viewMain);
        context = parent.getContext();
        return new ViewHolder(viewMain);
    }

//		@Override
//    public boolean onOptionsItemSelected(MenuItem item) {
//        switch (item.getItemId()) {
//            case R.id.force_reload:
//                listModulesDownload.clear();
//                new Utils.LoadModules(getActivity(), true).execute();
//                break;
//        }
//
//        return super.onOptionsItemSelected(item);
//    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        repo = mList.get(position);
        mHolder = holder;
        mExpandedList = new ArrayList<>(mList.size());
        for (int i = 0; i < mList.size(); i++) {
            mExpandedList.add(false);
        }
        SetupViewElements();

    }

    private void SetupViewElements() {
        int mPosition = mHolder.getAdapterPosition();
        if (repo.getId() != null) {
            mHolder.title.setText(repo.getName());
            mHolder.versionName.setText(repo.getmVersion());
            mHolder.description.setText(repo.getDescription());
            String authorString = this.context.getResources().getString(R.string.author) + " " + repo.getmAuthor();
            mHolder.author.setText(authorString);
            if ((repo.getmLogUrl() != null) && (repo.getmLogUrl().equals(""))) {
                mHolder.log.setText(repo.getmLogUrl());
                Linkify.addLinks(mHolder.log, Linkify.WEB_URLS);
            } else {
                mHolder.log.setVisibility(View.GONE);
            }
            mHolder.installedStatus.setText(repo.isInstalled() ? this.context.getResources().getString(R.string.module_installed) : this.context.getResources().getString(R.string.module_not_installed));
            if (mExpandedList.get(mPosition)) {
                mHolder.expandLayout.setVisibility(View.VISIBLE);
            } else {
                mHolder.expandLayout.setVisibility(View.GONE);
            }
            if (repo.isInstalled()) {
                mHolder.installedStatus.setTextColor(Color.parseColor("#14AD00"));
                mHolder.updateStatus.setText(repo.canUpdate() ? this.context.getResources().getString(R.string.module_update_available) : this.context.getResources().getString(R.string.module_up_to_date));
            }
            Log.d("Magisk", "ReposAdapter: Setting up info " + repo.getId() + " and " + repo.getDescription() + " and " + repo.getmVersion());
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
            updateImage.setImageResource(R.drawable.ic_system_update_alt_black);
            mCanUpdate = prefs.getBoolean("repo-isInstalled_" + repo.getId(), false);

            View.OnClickListener oCl = view -> {
                if (view == updateImage) {
                    if (!mIsInstalled | mCanUpdate) {

                        Utils.DownloadReceiver receiver = new Utils.DownloadReceiver() {
                            @Override
                            public void task(File file) {
                                Log.d("Magisk", "Task firing");
                                new Utils.FlashZIP(context, repo.getId(), file.toString()).execute();
                            }
                        };
                        String filename = repo.getId().replace(" ", "") + ".zip";
                        Utils.downloadAndReceive(context, receiver, repo.getmZipUrl(), filename);
                    } else {
                        Toast.makeText(context, repo.getId() + " is already installed.", Toast.LENGTH_SHORT).show();
                    }
                } else if (view == mHolder.log) {
                    new WebWindow("Changelog", repo.getmLogUrl(), this.context);
                }
            };

            updateImage.setOnClickListener(oCl);
            mHolder.log.setOnClickListener(oCl);
            if (prefs.contains("repo-isInstalled_" + repo.getId())) {
                mIsInstalled = prefs.getBoolean("repo-isInstalled_" + repo.getId(), false);

            }

        }
    }

//    protected List<Repo> mListRepos() {
//        return ReposFragment.listModulesDownload;
//    }

    @Override
    public int getItemCount() {
        return mList.size();
    }

    class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.title)
        TextView title;
        @BindView(R.id.version_name)
        TextView versionName;
        @BindView(R.id.description)
        TextView description;
        @BindView(R.id.author)
        TextView author;
        @BindView(R.id.log)
        TextView log;
        @BindView(R.id.installedStatus)
        TextView installedStatus;
        @BindView(R.id.updateStatus)
        TextView updateStatus;
        @BindView(R.id.expand_layout)
        LinearLayout expandLayout;
        private ValueAnimator mAnimator;

        public ViewHolder(View itemView) {
            super(itemView);
            WindowManager windowmanager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            ButterKnife.bind(this, itemView);
            DisplayMetrics dimension = new DisplayMetrics();
            windowmanager.getDefaultDisplay().getMetrics(dimension);
            expandLayout.getViewTreeObserver().addOnPreDrawListener(
                    new ViewTreeObserver.OnPreDrawListener() {

                        @Override
                        public boolean onPreDraw() {
                            final int widthSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                            final int heightSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                            expandLayout.getViewTreeObserver().removeOnPreDrawListener(this);
                            expandLayout.setVisibility(View.GONE);
                            expandLayout.measure(widthSpec, heightSpec);
                            mAnimator = slideAnimator(0, expandLayout.getMeasuredHeight());
                            return true;
                        }

                    });

            viewMain.setOnClickListener(view -> {
                int position = getAdapterPosition();
                if (mExpandedList.get(position)) {
                    collapse(expandLayout);
                } else {
                    expand(expandLayout);
                }
                mExpandedList.set(position, !mExpandedList.get(position));

            });

        }

        private void expand(View view) {
            view.setVisibility(View.VISIBLE);
            mAnimator.start();
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
                public void onAnimationStart(Animator animator) {
                }

                @Override
                public void onAnimationCancel(Animator animator) {
                }

                @Override
                public void onAnimationRepeat(Animator animator) {
                }
            });
            mAnimator.start();
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
