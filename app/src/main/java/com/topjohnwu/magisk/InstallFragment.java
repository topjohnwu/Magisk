package com.topjohnwu.magisk;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.CardView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Spinner;
import android.widget.TextView;

import com.topjohnwu.magisk.receivers.MagiskDlReceiver;
import com.topjohnwu.magisk.utils.CallbackHandler;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class InstallFragment extends Fragment implements CallbackHandler.EventListener {

    private Unbinder unbinder;
    @BindView(R.id.current_version_title) TextView currentVersionTitle;
    @BindView(R.id.install_title) TextView installTitle;
    @BindView(R.id.block_spinner) Spinner spinner;
    @BindView(R.id.detect_bootimage) Button detectButton;
    @BindView(R.id.flash_button) CardView flashButton;
    @BindView(R.id.keep_force_enc) CheckBox keepEncChkbox;
    @BindView(R.id.keep_verity) CheckBox keepVerityChkbox;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_install, container, false);
        unbinder = ButterKnife.bind(this, v);
        detectButton.setOnClickListener(v1 -> toAutoDetect());
        currentVersionTitle.setText(getString(R.string.current_magisk_title, Global.Info.magiskVersionString));
        installTitle.setText(getString(R.string.install_magisk_title, Global.Info.remoteMagiskVersion));
        flashButton.setOnClickListener(v1 -> {
            String bootImage;
            if (Global.Info.bootBlock != null) {
                if (spinner.getSelectedItemPosition() > 0)
                    bootImage = Global.Data.blockList.get(spinner.getSelectedItemPosition() - 1);
                else
                    bootImage = Global.Info.bootBlock;
            } else {
                bootImage = Global.Data.blockList.get(spinner.getSelectedItemPosition());
            }
            String filename = "Magisk-v" + Global.Info.remoteMagiskVersion + ".zip";
            String finalBootImage = bootImage;
            Utils.getAlertDialogBuilder(getActivity())
                    .setTitle(getString(R.string.repo_install_title, getString(R.string.magisk)))
                    .setMessage(getString(R.string.repo_install_msg, filename))
                    .setCancelable(true)
                    .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.dlAndReceive(
                            getActivity(),
                            new MagiskDlReceiver(finalBootImage, keepEncChkbox.isChecked(), keepVerityChkbox.isChecked()),
                            Global.Info.magiskLink,
                            Utils.getLegalFilename(filename)))
                    .setNeutralButton(R.string.check_release_notes, (dialog, which) -> {
                        getActivity().startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(Global.Info.releaseNoteLink)));
                    })
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        });
        if (Global.Events.blockDetectionDone.isTriggered) {
            updateUI();
        }
        return v;
    }

    @Override
    public void onTrigger(CallbackHandler.Event event) {
        updateUI();
    }

    private void updateUI() {
        List<String> items = new ArrayList<>(Global.Data.blockList);
        if (Global.Info.bootBlock != null)
            items.add(0, getString(R.string.auto_detect, Global.Info.bootBlock));
        ArrayAdapter<String> adapter = new ArrayAdapter<>(getActivity(),
                android.R.layout.simple_spinner_item, items);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        toAutoDetect();
    }

    private void toAutoDetect() {
        if (Global.Info.bootBlock != null) {
            spinner.setSelection(0);
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        getActivity().setTitle(R.string.install);
        CallbackHandler.register(Global.Events.blockDetectionDone, this);
    }

    @Override
    public void onStop() {
        CallbackHandler.unRegister(Global.Events.blockDetectionDone, this);
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
