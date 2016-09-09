package com.topjohnwu.magisk;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
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

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public abstract class BaseRepoFragment extends Fragment {

    @BindView(R.id.recyclerView)
    RecyclerView recyclerView;
    @BindView(R.id.empty_rv)
    TextView emptyTv;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.single_module_fragment, container, false);

        ButterKnife.bind(this, view);

        if (listRepos().size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);

            return view;
        }
        Log.d("Magisk", "BaseRepoFragment: ListRepos size is " + listRepos().size());
        recyclerView.setAdapter(new ReposAdapter(listRepos()) {

        });
        return view;
    }


    protected abstract List<Repo> listRepos();

    public class ReposAdapter extends RecyclerView.Adapter<ReposAdapter.ViewHolder> {

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

        public ReposAdapter(List<Repo> list) {
            this.mList = list;
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

        @Override
        public void onBindViewHolder(final ViewHolder holder, int position) {
            final Repo repo = mList.get(position);

            Log.d("Magisk", "ReposAdapter: Trying set up bindview from list pos " + position + " out of a total of " + mList.size() + " and " + repo.getId());
            if (repo.getId() != null) {
                TextView authorView = holder.author;
                holder.title.setText(repo.getName());
                holder.versionName.setText(repo.getmVersion());
                holder.description.setText(repo.getDescription());
                String authorString = getResources().getString(R.string.author) + " " + repo.getmAuthor();
                holder.author.setText(authorString);
                if ((repo.getmLogUrl() != null) && (repo.getmLogUrl().equals(""))) {
                    holder.log.setText(repo.getmLogUrl());
                    Linkify.addLinks(holder.log, Linkify.WEB_URLS);
                } else {
                    holder.log.setVisibility(View.GONE);
                }
                holder.installedStatus.setText(repo.isInstalled() ? getResources().getString(R.string.module_installed) : getResources().getString(R.string.module_not_installed));
                if (mExpandedList.get(position)) {
                    holder.expandLayout.setVisibility(View.VISIBLE);
                } else {
                    holder.expandLayout.setVisibility(View.GONE);
                }
                if (repo.isInstalled()) {
                    holder.installedStatus.setTextColor(Color.parseColor("#14AD00"));
                    holder.updateStatus.setText(repo.canUpdate() ? getResources().getString(R.string.module_update_available) : getResources().getString(R.string.module_up_to_date));
                }
                Log.d("Magisk", "ReposAdapter: Setting up info " + repo.getId() + " and " + repo.getDescription() + " and " + repo.getmVersion());
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
                updateImage.setImageResource(R.drawable.ic_system_update_alt_black);
                mCanUpdate = prefs.getBoolean("repo-isInstalled_" + repo.getId(), false);
                updateImage.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        if (!mIsInstalled | mCanUpdate) {

                            Utils.DownloadReceiver reciever = new Utils.DownloadReceiver() {
                                @Override
                                public void task(File file) {
                                    Log.d("Magisk", "Task firing");
                                    new Utils.FlashZIP(context, repo.getId(), file.toString()).execute();
                                }
                            };
                            String filename = repo.getId().replace(" ", "") + ".zip";
                            Utils.downloadAndReceive(context, reciever, repo.getmZipUrl(), filename);
                        } else {
                            Toast.makeText(context, repo.getId() + " is already installed.", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
                if (prefs.contains("repo-isInstalled_" + repo.getId())) {
                    mIsInstalled = prefs.getBoolean("repo-isInstalled_" + repo.getId(), false);
//                    if (mIsInstalled) {
//                        installedImage.setImageResource(R.drawable.ic_done_black);
//                    }

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
            @BindView(R.id.log)
            TextView log;
            @BindView(R.id.installedStatus)
            TextView installedStatus;
            @BindView(R.id.updateStatus)
            TextView updateStatus;
            @BindView(R.id.expand_layout)
            LinearLayout expandLayout;
            private ValueAnimator mAnimator;
            private int mMeasuredHeight;


            public ViewHolder(View itemView) {
                super(itemView);
                WindowManager windowmanager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
                ButterKnife.bind(this, itemView);

                DisplayMetrics dimension = new DisplayMetrics();
                windowmanager.getDefaultDisplay().getMetrics(dimension);
                final int mHeight = dimension.heightPixels;
                expandLayout.getViewTreeObserver().addOnPreDrawListener(
                        new ViewTreeObserver.OnPreDrawListener() {

                            @Override
                            public boolean onPreDraw() {
                                expandLayout.getViewTreeObserver().removeOnPreDrawListener(this);
                                expandLayout.setVisibility(View.GONE);

                                final int widthSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                                final int heightSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                                expandLayout.measure(widthSpec, heightSpec);
                                mAnimator = slideAnimator(0, expandLayout.getMeasuredHeight());
                                return true;
                            }

                        });

                viewMain.setOnClickListener(view -> {
                    int position = getAdapterPosition();
                    Log.d("Magisk", "BaseRepoFragment: CLICK. " + position + " and " + mExpandedList.get(position));

                    if (mExpandedList.get(position)) {
                        collapse(expandLayout);
                    } else {
                        expand(expandLayout);
                    }
                    mExpandedList.set(position, !mExpandedList.get(position));

                });

            }

            private void expand(View view) {

                // set Visible


                Log.d("Magisk", "BaseRepoFragment: Expand anim called " + mMeasuredHeight + " and " + view.getId());
                view.setVisibility(View.VISIBLE);
                mAnimator.start();
            }

            private void collapse(View view) {
                int finalHeight = view.getHeight();
                ValueAnimator mAnimator = slideAnimator(finalHeight, 0);
                Log.d("Magisk", "BaseRepoFragment: Collapse anim called " + finalHeight + " and " + view.getId());

                mAnimator.addListener(new Animator.AnimatorListener() {
                    @Override
                    public void onAnimationEnd(Animator animator) {
                        // Height=0, but it set visibility to GONE
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
                    // Update Height
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
}
