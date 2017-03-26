package com.topjohnwu.magisk;

import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.support.v7.widget.CardView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Spinner;
import android.widget.TextView;

import com.topjohnwu.magisk.asyncs.ProcessMagiskZip;
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
import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class InstallFragment extends Fragment implements CallbackEvent.Listener<Void> {


    private static final String UNINSTALLER = "magisk_uninstaller.sh";

    @BindView(R.id.current_version_title) TextView currentVersionTitle;
    @BindView(R.id.install_title) TextView installTitle;
    @BindView(R.id.block_spinner) Spinner spinner;
    @BindView(R.id.detect_bootimage) Button detectButton;
    @BindView(R.id.install_button) CardView installButton;
    @BindView(R.id.install_text) TextView installText;
    @BindView(R.id.uninstall_button) CardView uninstallButton;
    @BindView(R.id.keep_force_enc) CheckBox keepEncChkbox;
    @BindView(R.id.keep_verity) CheckBox keepVerityChkbox;

    @OnClick(R.id.detect_bootimage)
    public void toAutoDetect() {
        if (magiskManager.bootBlock != null) {
            spinner.setSelection(0);
        }
    }

    @OnClick(R.id.install_button)
    public void install() {
        String bootImage = null;
        if (magiskManager.blockList != null) {
            int idx = spinner.getSelectedItemPosition();
            if (magiskManager.bootBlock != null) {
                if (idx > 0) {
                    bootImage = magiskManager.blockList.get(idx - 1);
                }
            } else {
                if (idx > 0)  {
                    bootImage = magiskManager.blockList.get(idx - 1);
                } else {
                    SnackbarMaker.make(getActivity(), R.string.manual_boot_image, Snackbar.LENGTH_LONG);
                }
            }
        }
        final String finalBootImage = bootImage;
        String filename = "Magisk-v" + magiskManager.remoteMagiskVersion + ".zip";
        new AlertDialogBuilder(getActivity())
                .setTitle(getString(R.string.repo_install_title, getString(R.string.magisk)))
                .setMessage(getString(R.string.repo_install_msg, filename))
                .setCancelable(true)
                .setPositiveButton(Shell.rootAccess() ? R.string.install : R.string.download,
                    (dialogInterface, i) -> Utils.dlAndReceive(
                        getActivity(),
                        new DownloadReceiver() {
                            private String boot = finalBootImage;
                            private boolean enc = keepEncChkbox.isChecked();
                            private boolean verity = keepVerityChkbox.isChecked();

                            @Override
                            public void onDownloadDone(Uri uri) {
                                new ProcessMagiskZip(getActivity(), uri, boot, enc, verity).exec();
                            }
                        },
                        magiskManager.magiskLink,
                        Utils.getLegalFilename(filename)))
                .setNeutralButton(R.string.release_notes, (dialog, which) -> {
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
                        InputStream in = magiskManager.getAssets().open(UNINSTALLER);
                        File uninstaller = new File(magiskManager.getCacheDir(), UNINSTALLER);
                        FileOutputStream out = new FileOutputStream(uninstaller);
                        byte[] bytes = new byte[1024];
                        int read;
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
                                Shell.su(true, "mv -f " + uninstaller + " /cache/" + UNINSTALLER,
                                        "reboot");
                            }
                        }.start();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                })
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }

    private Unbinder unbinder;
    private MagiskManager magiskManager;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_install, container, false);
        unbinder = ButterKnife.bind(this, v);
        magiskManager = getApplication();
        if (magiskManager.magiskVersion < 0) {
            currentVersionTitle.setText(getString(R.string.current_magisk_title, getString(R.string.version_none)));
        } else {
            currentVersionTitle.setText(getString(R.string.current_magisk_title, "v" + magiskManager.magiskVersionString));
        }
        installTitle.setText(getString(R.string.install_magisk_title, "v" + String.format(Locale.US, "%.1f", magiskManager.remoteMagiskVersion)));

        updateUI();
        return v;
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        updateUI();
    }

    private void updateUI() {
        if (magiskManager.blockList == null || !Shell.rootAccess()) {
            uninstallButton.setVisibility(View.GONE);
            installText.setText(R.string.download);
            detectButton.setEnabled(false);
            keepEncChkbox.setEnabled(false);
            keepVerityChkbox.setEnabled(false);
            spinner.setEnabled(false);
        } else {
            uninstallButton.setVisibility(magiskManager.magiskVersion > 10.3 ? View.VISIBLE : View.GONE);
            installText.setText(R.string.download_install);
            detectButton.setEnabled(true);
            keepEncChkbox.setEnabled(true);
            keepVerityChkbox.setEnabled(true);
            spinner.setEnabled(true);

            List<String> items = new ArrayList<>();
            if (magiskManager.bootBlock != null) {
                items.add(getString(R.string.auto_detect, magiskManager.bootBlock));
            } else {
                items.add(getString(R.string.cannot_auto_detect));
            }
            items.addAll(magiskManager.blockList);
            ArrayAdapter<String> adapter = new ArrayAdapter<>(getActivity(),
                    android.R.layout.simple_spinner_item, items);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinner.setAdapter(adapter);
            toAutoDetect();
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        getActivity().setTitle(R.string.install);
        magiskManager.blockDetectionDone.register(this);
    }

    @Override
    public void onStop() {
        magiskManager.blockDetectionDone.unRegister(this);
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
