package com.topjohnwu.magisk;

import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.support.annotation.Nullable;
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

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class InstallFragment extends Fragment implements CallbackEvent.Listener<Void> {


    private static final String UNINSTALLER = "magisk_uninstaller.sh";

    private Unbinder unbinder;
    @BindView(R.id.current_version_title) TextView currentVersionTitle;
    @BindView(R.id.install_title) TextView installTitle;
    @BindView(R.id.block_spinner) Spinner spinner;
    @BindView(R.id.detect_bootimage) Button detectButton;
    @BindView(R.id.flash_button) CardView flashButton;
    @BindView(R.id.uninstall_button) CardView uninstallButton;
    @BindView(R.id.keep_force_enc) CheckBox keepEncChkbox;
    @BindView(R.id.keep_verity) CheckBox keepVerityChkbox;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_install, container, false);
        unbinder = ButterKnife.bind(this, v);
        detectButton.setOnClickListener(v1 -> toAutoDetect());
        currentVersionTitle.setText(getString(R.string.current_magisk_title, getApplication().magiskVersionString));
        installTitle.setText(getString(R.string.install_magisk_title, getApplication().remoteMagiskVersion));
        flashButton.setOnClickListener(v1 -> {
            String bootImage;
            if (getApplication().bootBlock != null) {
                if (spinner.getSelectedItemPosition() > 0)
                    bootImage = getApplication().blockList.get(spinner.getSelectedItemPosition() - 1);
                else
                    bootImage = getApplication().bootBlock;
            } else {
                bootImage = getApplication().blockList.get(spinner.getSelectedItemPosition());
            }
            String filename = "Magisk-v" + getApplication().remoteMagiskVersion + ".zip";
            new AlertDialogBuilder(getActivity())
                    .setTitle(getString(R.string.repo_install_title, getString(R.string.magisk)))
                    .setMessage(getString(R.string.repo_install_msg, filename))
                    .setCancelable(true)
                    .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.dlAndReceive(
                            getActivity(),
                            new DownloadReceiver() {
                                private String boot = bootImage;
                                private boolean enc = keepEncChkbox.isChecked();
                                private boolean verity = keepVerityChkbox.isChecked();

                                @Override
                                public void onDownloadDone(Uri uri) {
                                    new ProcessMagiskZip(getActivity(), uri, boot, enc, verity).exec();
                                }
                            },
                            getApplication().magiskLink,
                            Utils.getLegalFilename(filename)))
                    .setNeutralButton(R.string.release_notes, (dialog, which) -> {
                        getActivity().startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(getApplication().releaseNoteLink)));
                    })
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        });
        if (getApplication().magiskVersion < 10.3) {
            uninstallButton.setVisibility(View.GONE);
        } else {
            uninstallButton.setOnClickListener(vi -> {
                new AlertDialogBuilder(getActivity())
                        .setTitle(R.string.uninstall_magisk_title)
                        .setMessage(R.string.uninstall_magisk_msg)
                        .setPositiveButton(R.string.yes, (dialogInterface, i) -> {
                            try {
                                InputStream in = getActivity().getAssets().open(UNINSTALLER);
                                File uninstaller = new File(getActivity().getCacheDir().getAbsolutePath() + "/" + UNINSTALLER);
                                FileOutputStream out = new FileOutputStream(uninstaller);
                                byte[] bytes = new byte[1024];
                                int read;
                                while ((read = in.read(bytes)) != -1)
                                    out.write(bytes, 0, read);
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
                                        Shell.su(true, "cp -af " + uninstaller + " /cache/" + UNINSTALLER,
                                                "reboot");
                                    }
                                }.start();
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        })
                        .setNegativeButton(R.string.no_thanks, null)
                        .show();
            });
        }

        if (getApplication().blockDetectionDone.isTriggered) {
            updateUI();
        }
        return v;
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        updateUI();
    }

    private void updateUI() {
        List<String> items = new ArrayList<>(getApplication().blockList);
        if (getApplication().bootBlock != null)
            items.add(0, getString(R.string.auto_detect, getApplication().bootBlock));
        ArrayAdapter<String> adapter = new ArrayAdapter<>(getActivity(),
                android.R.layout.simple_spinner_item, items);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        toAutoDetect();
    }

    private void toAutoDetect() {
        if (getApplication().bootBlock != null) {
            spinner.setSelection(0);
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        getActivity().setTitle(R.string.install);
        getApplication().blockDetectionDone.register(this);
    }

    @Override
    public void onStop() {
        getApplication().blockDetectionDone.unRegister(this);
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
