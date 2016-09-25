package com.topjohnwu.magisk;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.support.v7.widget.RecyclerView;
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
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebWindow;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

    private final List<Repo> mList;
    List<Boolean> mExpandedList;
    private View viewMain;
    private Context context;
    private boolean mCanUpdate;
    private boolean alertUpdate;
    private boolean ignoreAlertUpdate;
    private Repo repo;
    private ViewHolder mHolder;
    private String mDonateUrl, mSupportUrl, mLogUrl,alertPackage;
    private SharedPreferences prefs;


    public ReposAdapter(ReposFragment reposFragment, List<Repo> list) {
        ReposFragment reposFragment1 = reposFragment;
        alertPackage = "";
        alertUpdate = false;
        this.mList = list;
        Log.d("Magisk", "ReposAdapter: I am alive. I have a list " + list.size());
        mExpandedList = new ArrayList<>(mList.size());
        for (int i = 0; i < mList.size(); i++) {
            mExpandedList.add(false);
            if (mList.get(i).canUpdate()) {
                alertUpdate = true;
                if (alertPackage.equals("")) {
                    alertPackage = mList.get(i).getName();
                } else {
                    alertPackage += mList.get(i).getName() + ", ";
                }
            }
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
        prefs = PreferenceManager.getDefaultSharedPreferences(context);
        repo = mList.get(position);
        mHolder = holder;
        mDonateUrl = repo.getDonateUrl();
        mSupportUrl = repo.getSupportUrl();
        mLogUrl = repo.getLogUrl();
        mExpandedList = new ArrayList<>(mList.size());
        for (int i = 0; i < mList.size(); i++) {
            mExpandedList.add(false);
        }
        SetupViewElements(repo);

    }

    private void SetupViewElements(Repo repo) {
        int mPosition = mHolder.getAdapterPosition();
        String titleString;
        if (repo.getId() != null) {
            if (repo.isCacheModule()) {
                titleString = "[Cache] " + repo.getName();
            } else {
                titleString = repo.getName();
            }

            mHolder.title.setText(titleString);
            mHolder.versionName.setText(repo.getVersion());
            mHolder.description.setText(repo.getDescription());
            String authorString = this.context.getResources().getString(R.string.author) + " " + repo.getAuthor();
            mHolder.author.setText(authorString);
            if (prefs.contains("ignoreUpdateAlerts")) {
                ignoreAlertUpdate = prefs.getBoolean("ignoreUpdateAlerts",false);
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

            Log.d("Magisk", "ReposAdapter: Setting up info " + repo.getId() + " and " + repo.getDescription() + " and " + repo.getVersion());
            prefs = PreferenceManager.getDefaultSharedPreferences(context);
            mCanUpdate = prefs.getBoolean("repo-canUpdate_" + repo.getId(), false);


            View.OnClickListener oCl = view -> {
                Log.d("Magisk", "Onlick captured, view is " + view.getId());

                if (view.getId() == mHolder.updateImage.getId()) {
                    if (!repo.isInstalled() | repo.canUpdate()) {

                        Utils.DownloadReceiver receiver = new Utils.DownloadReceiver() {
                            @Override
                            public void task(File file) {
                                Log.d("Magisk", "Task firing");
                                new Async.FlashZIP(context, repo.getId(), file.toString()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                            }
                        };
                        String filename = repo.getId().replace(" ", "") + ".zip";
                        Utils.downloadAndReceive(context, receiver, repo.getZipUrl(), filename);
                    } else {
                        Toast.makeText(context, repo.getId() + " is already installed.", Toast.LENGTH_SHORT).show();
                    }
                }
                if ((view.getId() == mHolder.changeLog.getId()) && (!repo.getLogUrl().equals(""))) {
                    new WebWindow("Changelog", repo.getLogUrl(),context);
                }
                if ((view.getId() == mHolder.authorLink.getId()) && (!repo.getSupportUrl().equals(""))) {
                    new WebWindow("Donate", repo.getDonateUrl(),context);
                }
                if ((view.getId() == mHolder.supportLink.getId()) && (!repo.getSupportUrl().equals(""))) {
                    new WebWindow("Support", repo.getSupportUrl(),context);
                }
            };
            mHolder.changeLog.setOnClickListener(oCl);
            mHolder.updateImage.setOnClickListener(oCl);
            mHolder.authorLink.setOnClickListener(oCl);
            mHolder.supportLink.setOnClickListener(oCl);
            if (prefs.contains("repo-isInstalled_" + repo.getId())) {
                boolean mIsInstalled = prefs.getBoolean("repo-isInstalled_" + repo.getId(), false);

            }

        }
    }



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
        @BindView(R.id.installedStatus)
        TextView installedStatus;
        @BindView(R.id.updateStatus)
        TextView updateStatus;
        @BindView(R.id.expand_layout)
        LinearLayout expandLayout;
        @BindView(R.id.update)
        ImageView updateImage;
        @BindView(R.id.installed)
        ImageView installedImage;
        @BindView(R.id.changeLog)
        ImageView changeLog;
        @BindView(R.id.authorLink)
        ImageView authorLink;
        @BindView(R.id.supportLink)
        ImageView supportLink;
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

            viewMain.setOnClickListener(view -> {
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
