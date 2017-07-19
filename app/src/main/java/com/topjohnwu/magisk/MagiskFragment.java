package com.topjohnwu.magisk;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.app.NotificationManager;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.CardView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class MagiskFragment extends Fragment
        implements CallbackEvent.Listener<Void>, SwipeRefreshLayout.OnRefreshListener {

    public static final String SHOW_DIALOG = "dialog";

    private static int expandHeight = 0;
    private static boolean mExpanded = false;

    private MagiskManager magiskManager;
    private Unbinder unbinder;

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    @BindView(R.id.magisk_update_card) CardView magiskUpdateCard;
    @BindView(R.id.magisk_update_icon) ImageView magiskUpdateIcon;
    @BindView(R.id.magisk_update_status) TextView magiskUpdateText;
    @BindView(R.id.magisk_update_progress) ProgressBar magiskUpdateProgress;

    @BindView(R.id.magisk_status_icon) ImageView magiskStatusIcon;
    @BindView(R.id.magisk_version) TextView magiskVersionText;
    @BindView(R.id.root_status_icon) ImageView rootStatusIcon;
    @BindView(R.id.root_status) TextView rootStatusText;

    @BindView(R.id.safetyNet_card) CardView safetyNetCard;
    @BindView(R.id.safetyNet_refresh) ImageView safetyNetRefreshIcon;
    @BindView(R.id.safetyNet_status) TextView safetyNetStatusText;
    @BindView(R.id.safetyNet_check_progress) ProgressBar safetyNetProgress;
    @BindView(R.id.expand_layout) LinearLayout expandLayout;
    @BindView(R.id.cts_status_icon) ImageView ctsStatusIcon;
    @BindView(R.id.cts_status) TextView ctsStatusText;
    @BindView(R.id.basic_status_icon) ImageView basicStatusIcon;
    @BindView(R.id.basic_status) TextView basicStatusText;

    @BindView(R.id.bootimage_card) CardView bootImageCard;
    @BindView(R.id.block_spinner) Spinner spinner;
    @BindView(R.id.detect_bootimage) Button detectButton;
    @BindView(R.id.install_option_card) CardView installOptionCard;
    @BindView(R.id.keep_force_enc) CheckBox keepEncChkbox;
    @BindView(R.id.keep_verity) CheckBox keepVerityChkbox;
    @BindView(R.id.install_button) CardView installButton;
    @BindView(R.id.install_text) TextView installText;
    @BindView(R.id.uninstall_button) CardView uninstallButton;

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

    @OnClick(R.id.install_button)
    public void install() {
        String bootImage = null;
        if (Shell.rootAccess()) {
            if (magiskManager.bootBlock != null) {
                bootImage = magiskManager.bootBlock;
            } else {
                int idx = spinner.getSelectedItemPosition();
                if (idx > 0)  {
                    bootImage = magiskManager.blockList.get(idx - 1);
                } else {
                    SnackbarMaker.make(getActivity(), R.string.manual_boot_image, Snackbar.LENGTH_LONG).show();
                    return;
                }
            }
        }
        final String finalBootImage = bootImage;
        String filename = "Magisk-v" + magiskManager.remoteMagiskVersionString + ".zip";
        new AlertDialogBuilder(getActivity())
                .setTitle(getString(R.string.repo_install_title, getString(R.string.magisk)))
                .setMessage(getString(R.string.repo_install_msg, filename))
                .setCancelable(true)
                .setPositiveButton(Shell.rootAccess() ? R.string.install : R.string.download,
                    (d, i) -> {
                        ((NotificationManager) getActivity()
                                .getSystemService(Context.NOTIFICATION_SERVICE)).cancelAll();
                        Utils.dlAndReceive(
                            getActivity(),
                            new DownloadReceiver() {
                                private String boot = finalBootImage;
                                private boolean enc = keepEncChkbox.isChecked();
                                private boolean verity = keepVerityChkbox.isChecked();

                                @Override
                                public void onDownloadDone(Uri uri) {
                                    if (Shell.rootAccess()) {
                                        magiskManager.shell.su_raw(
                                                "rm -f /dev/.magisk",
                                                "echo \"BOOTIMAGE=" + boot + "\" >> /dev/.magisk",
                                                "echo \"KEEPFORCEENCRYPT=" + String.valueOf(enc) + "\" >> /dev/.magisk",
                                                "echo \"KEEPVERITY=" + String.valueOf(verity) + "\" >> /dev/.magisk"
                                        );
                                        startActivity(new Intent(getActivity(), FlashActivity.class).setData(uri));
                                    } else {
                                        Utils.showUriSnack(getActivity(), uri);
                                    }
                                }
                            },
                            magiskManager.magiskLink,
                            Utils.getLegalFilename(filename));
                    }
                )
                .setNeutralButton(R.string.release_notes, (d, i) -> {
                    if (magiskManager.releaseNoteLink != null) {
                        Intent openReleaseNoteLink = new Intent(Intent.ACTION_VIEW, Uri.parse(magiskManager.releaseNoteLink));
                        openReleaseNoteLink.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        magiskManager.startActivity(openReleaseNoteLink);
                    }
                })
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }

    @OnClick(R.id.uninstall_button)
    public void uninstall() {
        new AlertDialogBuilder(getActivity())
                .setTitle(R.string.uninstall_magisk_title)
                .setMessage(R.string.uninstall_magisk_msg)
                .setPositiveButton(R.string.yes, (dialogInterface, i) -> {
                    try {
                        InputStream in = magiskManager.getAssets().open(MagiskManager.UNINSTALLER);
                        File uninstaller = new File(magiskManager.getCacheDir(), MagiskManager.UNINSTALLER);
                        FileOutputStream out = new FileOutputStream(uninstaller);
                        byte[] bytes = new byte[1024];
                        int read;
                        while ((read = in.read(bytes)) != -1) {
                            out.write(bytes, 0, read);
                        }
                        in.close();
                        out.close();
                        in = magiskManager.getAssets().open(MagiskManager.UTIL_FUNCTIONS);
                        File utils = new File(magiskManager.getCacheDir(), MagiskManager.UTIL_FUNCTIONS);
                        out = new FileOutputStream(utils);
                        while ((read = in.read(bytes)) != -1) {
                            out.write(bytes, 0, read);
                        }
                        in.close();
                        out.close();
                        ProgressDialog progress = new ProgressDialog(getActivity());
                        progress.setTitle(R.string.reboot);
                        progress.show();
                        new CountDownTimer(5000, 1000) {
                            @Override
                            public void onTick(long millisUntilFinished) {
                                progress.setMessage(getString(R.string.reboot_countdown, millisUntilFinished / 1000));
                            }

                            @Override
                            public void onFinish() {
                                progress.setMessage(getString(R.string.reboot_countdown, 0));
                                magiskManager.shell.su_raw(
                                        "mv -f " + uninstaller + " /cache/" + MagiskManager.UNINSTALLER,
                                        "mv -f " + utils + " /data/magisk/" + MagiskManager.UTIL_FUNCTIONS,
                                        "reboot"
                                );
                            }
                        }.start();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                })
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_magisk, container, false);
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

        mSwipeRefreshLayout.setOnRefreshListener(this);

        updateUI();

        if (getArguments() != null && getArguments().getBoolean(SHOW_DIALOG))
            install();

        return v;
    }

    @Override
    public void onRefresh() {
        updateUI();

        magiskUpdateText.setText(R.string.checking_for_updates);
        magiskUpdateProgress.setVisibility(View.VISIBLE);
        magiskUpdateIcon.setVisibility(View.GONE);

        safetyNetStatusText.setText(R.string.safetyNet_check_text);

        magiskManager.safetyNetDone.isTriggered = false;
        magiskManager.updateCheckDone.isTriggered = false;
        magiskManager.remoteMagiskVersionString = null;
        magiskManager.remoteMagiskVersionCode = -1;
        collapse();

        // Trigger state check
        if (Utils.checkNetworkStatus(magiskManager)) {
            new CheckUpdates(getActivity()).exec();
        } else {
            mSwipeRefreshLayout.setRefreshing(false);
        }
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        if (event == magiskManager.updateCheckDone) {
            updateCheckUI();
        } else if (event == magiskManager.safetyNetDone) {
            updateSafetyNetUI();
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        // Manual trigger if already done
        if (magiskManager.updateCheckDone.isTriggered)
            updateCheckUI();
        if (magiskManager.safetyNetDone.isTriggered)
            updateSafetyNetUI();
        magiskManager.updateCheckDone.register(this);
        magiskManager.safetyNetDone.register(this);
        getActivity().setTitle(R.string.magisk);
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
        ((MainActivity) getActivity()).checkHideSection();
        magiskManager.updateMagiskInfo();

        final int ROOT = 0x1, NETWORK = 0x2, UPTODATE = 0x4;
        int status = 0;
        status |= Shell.rootAccess() ? ROOT : 0;
        status |= Utils.checkNetworkStatus(magiskManager) ? NETWORK : 0;
        status |= magiskManager.magiskVersionCode >= 130 ? UPTODATE : 0;
        magiskUpdateCard.setVisibility(Utils.checkBits(status, NETWORK) ? View.VISIBLE : View.GONE);
        safetyNetCard.setVisibility(Utils.checkBits(status, NETWORK) ? View.VISIBLE : View.GONE);
        bootImageCard.setVisibility(Utils.checkBits(status, NETWORK, ROOT) ? View.VISIBLE : View.GONE);
        installOptionCard.setVisibility(Utils.checkBits(status, NETWORK, ROOT) ? View.VISIBLE : View.GONE);
        installButton.setVisibility(Utils.checkBits(status, NETWORK) ? View.VISIBLE : View.GONE);
        uninstallButton.setVisibility(Utils.checkBits(status, UPTODATE, ROOT) ? View.VISIBLE : View.GONE);

        int image, color;

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

        if (!Shell.rootAccess()) {
            installText.setText(R.string.download);
        } else {
            installText.setText(R.string.download_install);

            List<String> items = new ArrayList<>();
            if (magiskManager.bootBlock != null) {
                items.add(getString(R.string.auto_detect, magiskManager.bootBlock));
                spinner.setEnabled(false);
            } else {
                items.add(getString(R.string.cannot_auto_detect));
                items.addAll(magiskManager.blockList);
            }
            ArrayAdapter<String> adapter = new ArrayAdapter<>(getActivity(),
                    android.R.layout.simple_spinner_item, items);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinner.setAdapter(adapter);
        }
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

        magiskUpdateProgress.setVisibility(View.GONE);
        mSwipeRefreshLayout.setRefreshing(false);

        if (magiskManager.remoteMagiskVersionCode > magiskManager.magiskVersionCode)
            install();
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

