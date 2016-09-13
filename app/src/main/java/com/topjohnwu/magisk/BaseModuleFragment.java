package com.topjohnwu.magisk;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.RepoHelper;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebWindow;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public abstract class BaseModuleFragment extends Fragment {
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyTv;
    
    private RepoHelper.TaskDelegate mDelegate;
    private SharedPreferences prefs;

    public BaseModuleFragment SetDelegate(RepoHelper.TaskDelegate delegate) {
        mDelegate = delegate;
        return null;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View viewMain = inflater.inflate(R.layout.single_module_fragment, container, false);


        ButterKnife.bind(this, viewMain);
        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            Log.d("Magisk","ModulesFragment: SWIPE");
            mDelegate.taskCompletionResult("OK");
        });


        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        prefs.registerOnSharedPreferenceChangeListener((sharedPreferences, s) -> {
            if (s.contains("updated")) {
                viewMain.invalidate();
                viewMain.requestLayout();

            }
        });
        if (listModules().size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);

            return viewMain;
        }

        recyclerView.setAdapter(new ModulesAdapter(listModules(), (chk, position) -> {
            // On Checkbox change listener
            CheckBox chbox = (CheckBox) chk;

            if (!chbox.isChecked()) {
                listModules().get(position).createDisableFile();
                Snackbar.make(chk, R.string.disable_file_created, Snackbar.LENGTH_SHORT).show();
            } else {
                listModules().get(position).removeDisableFile();
                Snackbar.make(chk, R.string.disable_file_removed, Snackbar.LENGTH_SHORT).show();
            }
        }, (deleteBtn, position) -> {
            // On delete button click listener

            listModules().get(position).createRemoveFile();
            Snackbar.make(deleteBtn, R.string.remove_file_created, Snackbar.LENGTH_SHORT).show();
        }, (undeleteBtn, position) -> {
            // On undelete button click listener

            listModules().get(position).deleteRemoveFile();
            Snackbar.make(undeleteBtn, R.string.remove_file_deleted, Snackbar.LENGTH_SHORT).show();
        }));
        return viewMain;
    }


    protected abstract List<Module> listModules();

    public class ModulesAdapter extends RecyclerView.Adapter<ModulesAdapter.ViewHolder> {

        private final List<Module> mList;
        List<Boolean> mExpandedList;
		@BindView(R.id.expand_layout)
        LinearLayout expandedLayout;
        private View viewMain;
		private Context context;
        private final Utils.ItemClickListener chboxListener;
        private final Utils.ItemClickListener deleteBtnListener;
        private final Utils.ItemClickListener unDeleteBtnListener;

        public ModulesAdapter(List<Module> list, Utils.ItemClickListener chboxListener, Utils.ItemClickListener deleteBtnListener, Utils.ItemClickListener undeleteBtnListener) {
            this.mList = list;
			mExpandedList = new ArrayList<>(mList.size());
            for (int i = 0; i < mList.size(); i++) {
                mExpandedList.add(false);
            }
            this.chboxListener = chboxListener;
            this.deleteBtnListener = deleteBtnListener;
            this.unDeleteBtnListener = undeleteBtnListener;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            viewMain = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_module, parent, false);
			context = parent.getContext();
			ButterKnife.bind(this, viewMain);
            return new ViewHolder(viewMain);
        }

        @Override
        public void onBindViewHolder(final ViewHolder holder, int position) {
            final Module module = mList.get(position);
            Log.d("Magisk","ModulesAdapter: Trying set up bindview from list pos " + position + " and " + module.getName() );

            holder.title.setText(module.getName());
            holder.versionName.setText(module.getVersion());
            holder.description.setText(module.getDescription());
            holder.author.setText(module.getAuthor());

            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
            if (prefs.contains("repo-canUpdate_" + module.getId())) {
                if (prefs.getBoolean("repo-canUpdate_" + module.getId(),false)) {
                    holder.updateStatus.setText(R.string.module_update_available);
                    holder.updateStatus.setVisibility(View.VISIBLE);
                } else {
                    holder.updateStatus.setVisibility(View.GONE);
                }
            }

            View.OnClickListener oCl = new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (view.getId() == holder.changeLog.getId()) {
                        new WebWindow("Changelog",module.getChangeLog(),context);
                    }
                    if (view.getId() == holder.authorLink.getId()) {
                        new WebWindow("Donate",module.getmDonateUrl(),context);
                    }
                    if (view.getId() == holder.supportLink.getId()) {
                        new WebWindow("Support",module.getmSupportUrl(),context);
                    }
                }
            };

            holder.authorLink.setOnClickListener(oCl);
            holder.changeLog.setOnClickListener(oCl);
            holder.supportLink.setOnClickListener(oCl);
            holder.checkBox.setChecked(module.isEnabled());
            holder.checkBox.setOnCheckedChangeListener((compoundButton, b) -> chboxListener.onItemClick(compoundButton, holder.getAdapterPosition()));

            holder.delete.setOnClickListener(view -> {
                if (module.willBeRemoved()) {
                    unDeleteBtnListener.onItemClick(holder.delete, holder.getAdapterPosition());
                } else {
                    deleteBtnListener.onItemClick(holder.delete, holder.getAdapterPosition());
                }

                updateDeleteButton(holder, module);
            });

            updateDeleteButton(holder, module);
        }

        private void updateDeleteButton(ViewHolder holder, Module module) {
            holder.warning.setVisibility(module.willBeRemoved() ? View.VISIBLE : View.GONE);

            if (module.willBeRemoved()) {
                holder.delete.setImageResource(R.drawable.ic_undelete);
            } else {
                holder.delete.setImageResource(R.drawable.ic_delete);
            }
        }

        @Override
        public int getItemCount() {
            return mList.size();
        }

        class ViewHolder extends RecyclerView.ViewHolder {

            @BindView(R.id.title) TextView title;

            @BindView(R.id.version_name) TextView versionName;
            @BindView(R.id.description) TextView description;
            @BindView(R.id.warning) TextView warning;
            @BindView(R.id.checkbox) CheckBox checkBox;
            @BindView(R.id.author) TextView author;
            @BindView(R.id.updateStatus) TextView updateStatus;
            @BindView(R.id.delete) ImageView delete;
            @BindView(R.id.changeLog) ImageView changeLog;
            @BindView(R.id.authorLink) ImageView authorLink;
            @BindView(R.id.supportLink) ImageView supportLink;
			@BindView(R.id.expand_layout) LinearLayout expandLayout;
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
                                final int heightSpec = View.MeasureSpec.makeMeasureSpec(0,View.MeasureSpec.UNSPECIFIED);
                                expandLayout.measure(widthSpec, heightSpec);
                                mAnimator = slideAnimator(0, expandLayout.getMeasuredHeight());
                                return true;
                            }
                        });

                viewMain.setOnClickListener(view -> {
                    int position = getAdapterPosition();
                    Log.d("Magisk", "ReposFragment: CLICK. " + position + " and " + mExpandedList.get(position));

                    if (mExpandedList.get(position)) {
                        collapse(expandLayout);
                    } else {
                        expand(expandLayout);
                    }
                    mExpandedList.set(position, !mExpandedList.get(position));

                });
                if (!Shell.rootAccess()) {
                    checkBox.setEnabled(false);
                    delete.setEnabled(false);
                }
            }
                private void expand(View view) {

                    // set Visible


                    Log.d("Magisk", "ReposFragment: Expand anim called " + mMeasuredHeight + " and " + view.getId());
                    view.setVisibility(View.VISIBLE);
                    mAnimator.start();
                }

                private void collapse(View view) {
                    int finalHeight = view.getHeight();
                    ValueAnimator mAnimator = slideAnimator(finalHeight, 0);
                    Log.d("Magisk", "ReposFragment: Collapse anim called " + finalHeight + " and " + view.getId());

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
