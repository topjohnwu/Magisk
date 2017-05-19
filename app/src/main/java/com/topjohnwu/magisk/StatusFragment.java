package com.topjohnwu.magisk;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class StatusFragment extends Fragment implements CallbackEvent.Listener<Void> {

    private static boolean noDialog = false;

    private Unbinder unbinder;
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    @BindView(R.id.magisk_update_icon) ImageView magiskUpdateIcon;
    @BindView(R.id.magisk_update_status) TextView magiskUpdateText;
    @BindView(R.id.magisk_check_updates_progress) ProgressBar magiskCheckUpdatesProgress;

    @BindView(R.id.magisk_status_icon) ImageView magiskStatusIcon;
    @BindView(R.id.magisk_version) TextView magiskVersionText;

    @BindView(R.id.root_status_icon) ImageView rootStatusIcon;
    @BindView(R.id.root_status) TextView rootStatusText;

    @BindView(R.id.safetyNet_refresh) ImageView safetyNetRefreshIcon;
    @BindView(R.id.safetyNet_status) TextView safetyNetStatusText;
    @BindView(R.id.safetyNet_check_progress) ProgressBar safetyNetProgress;
    @BindView(R.id.expand_layout) LinearLayout expandLayout;
    @BindView(R.id.cts_status_icon) ImageView ctsStatusIcon;
    @BindView(R.id.cts_status) TextView ctsStatusText;
    @BindView(R.id.basic_status_icon) ImageView basicStatusIcon;
    @BindView(R.id.basic_status) TextView basicStatusText;

    @BindColor(R.color.red500) int colorBad;
    @BindColor(R.color.green500) int colorOK;
    @BindColor(R.color.yellow500) int colorWarn;
    @BindColor(R.color.grey500) int colorNeutral;
    @BindColor(R.color.blue500) int colorInfo;

    @OnClick(R.id.safetyNet_title)
    public void safetyNet() {
        safetyNetProgress.setVisibility(View.VISIBLE);
        safetyNetRefreshIcon.setVisibility(View.GONE);
        safetyNetStatusText.setText(R.string.checking_safetyNet_status);
        Utils.checkSafetyNet(getActivity());
        collapse();
    }

    public void gotoInstall() {
        if (magiskManager.remoteMagiskVersionCode > 0) {
            ((MainActivity) getActivity()).navigate(R.id.install);
        }
    }

    private MagiskManager magiskManager;
    private static int expandHeight = 0;
    private static boolean mExpanded = false;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_status, container, false);
        unbinder = ButterKnife.bind(this, v);
        magiskManager = getApplication();

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
                        setExpanded();
                        return true;
                    }

                });

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            magiskUpdateText.setText(R.string.checking_for_updates);
            magiskCheckUpdatesProgress.setVisibility(View.VISIBLE);
            magiskUpdateIcon.setVisibility(View.GONE);

            safetyNetStatusText.setText(R.string.safetyNet_check_text);

            magiskManager.safetyNetDone.isTriggered = false;
            collapse();
            noDialog = false;

            updateUI();
            new CheckUpdates(getActivity()).exec();
        });

        if (magiskManager.magiskVersionCode < 0 && Shell.rootAccess() && !noDialog) {
            noDialog = true;
            new AlertDialogBuilder(getActivity())
                    .setTitle(R.string.no_magisk_title)
                    .setMessage(R.string.no_magisk_msg)
                    .setCancelable(true)
                    .setPositiveButton(R.string.goto_install, (d, i) -> gotoInstall())
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }

        updateUI();

        if (magiskManager.updateCheckDone.isTriggered)
            updateCheckUI();
        if (magiskManager.safetyNetDone.isTriggered)
            updateSafetyNetUI();

        return v;
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        if (event == magiskManager.updateCheckDone) {
            Logger.dev("StatusFragment: Update Check UI refresh triggered");
            updateCheckUI();
        } else if (event == magiskManager.safetyNetDone) {
            Logger.dev("StatusFragment: SafetyNet UI refresh triggered");
            updateSafetyNetUI();
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        magiskManager.updateCheckDone.register(this);
        magiskManager.safetyNetDone.register(this);
        getActivity().setTitle(R.string.status);
    }

    @Override
    public void onStop() {
        magiskManager.updateCheckDone.unRegister(this);
        magiskManager.safetyNetDone.unRegister(this);
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }

    private void updateUI() {
        int image, color;

        magiskManager.updateMagiskInfo();

        if (magiskManager.magiskVersionCode < 0) {
            color = colorBad;
            image = R.drawable.ic_cancel;
            magiskVersionText.setText(R.string.magisk_version_error);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskVersionText.setText(getString(R.string.current_magisk_title, "v" + magiskManager.magiskVersionString));
        }

        magiskStatusIcon.setImageResource(image);
        magiskStatusIcon.setColorFilter(color);

        switch (Shell.rootStatus) {
            case 0:
                color = colorBad;
                image = R.drawable.ic_cancel;
                rootStatusText.setText(R.string.not_rooted);
                break;
            case 1:
                if (magiskManager.suVersion != null) {
                    color = colorOK;
                    image = R.drawable.ic_check_circle;
                    rootStatusText.setText(magiskManager.suVersion);
                    break;
                }
            case -1:
            default:
                color = colorNeutral;
                image = R.drawable.ic_help;
                rootStatusText.setText(R.string.root_error);
        }

        rootStatusIcon.setImageResource(image);
        rootStatusIcon.setColorFilter(color);
    }

    private void updateCheckUI() {
        int image, color;

        if (magiskManager.remoteMagiskVersionCode < 0) {
            color = colorNeutral;
            image = R.drawable.ic_help;
            magiskUpdateText.setText(R.string.cannot_check_updates);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskUpdateText.setText(getString(R.string.install_magisk_title, "v" + magiskManager.remoteMagiskVersionString));
        }

        magiskUpdateIcon.setImageResource(image);
        magiskUpdateIcon.setColorFilter(color);
        magiskUpdateIcon.setVisibility(View.VISIBLE);

        magiskCheckUpdatesProgress.setVisibility(View.GONE);
        mSwipeRefreshLayout.setRefreshing(false);
    }

    private void updateSafetyNetUI() {
        int image, color;
        safetyNetProgress.setVisibility(View.GONE);
        safetyNetRefreshIcon.setVisibility(View.VISIBLE);
        if (magiskManager.SNCheckResult.failed) {
            safetyNetStatusText.setText(magiskManager.SNCheckResult.errmsg);
            collapse();
        } else {
            safetyNetStatusText.setText(R.string.safetyNet_check_success);
            if (magiskManager.SNCheckResult.ctsProfile) {
                color = colorOK;
                image = R.drawable.ic_check_circle;
            } else {
                color = colorBad;
                image = R.drawable.ic_cancel;
            }
            ctsStatusText.setText("ctsProfile: " + magiskManager.SNCheckResult.ctsProfile);
            ctsStatusIcon.setImageResource(image);
            ctsStatusIcon.setColorFilter(color);

            if (magiskManager.SNCheckResult.basicIntegrity) {
                color = colorOK;
                image = R.drawable.ic_check_circle;
            } else {
                color = colorBad;
                image = R.drawable.ic_cancel;
            }
            basicStatusText.setText("basicIntegrity: " + magiskManager.SNCheckResult.basicIntegrity);
            basicStatusIcon.setImageResource(image);
            basicStatusIcon.setColorFilter(color);
            expand();
        }
    }

    private void setExpanded() {
        ViewGroup.LayoutParams layoutParams = expandLayout.getLayoutParams();
        layoutParams.height = mExpanded ? expandHeight : 0;
        expandLayout.setLayoutParams(layoutParams);
        expandLayout.setVisibility(mExpanded ? View.VISIBLE : View.GONE);
    }

    private void expand() {
        if (mExpanded) return;
        expandLayout.setVisibility(View.VISIBLE);
        ValueAnimator mAnimator = slideAnimator(0, expandHeight);
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

