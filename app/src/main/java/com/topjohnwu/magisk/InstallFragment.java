package com.topjohnwu.magisk;

import android.app.Fragment;
import android.os.Bundle;
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

import com.topjohnwu.magisk.receivers.MagiskDlReceiver;
import com.topjohnwu.magisk.utils.CallbackHandler;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class InstallFragment extends Fragment implements CallbackHandler.EventListener {

    public static final CallbackHandler.Event blockDetectionDone = new CallbackHandler.Event();

    public static List<String> blockList;
    public static String bootBlock = null;

    @BindView(R.id.install_title) TextView installTitle;
    @BindView(R.id.block_spinner) Spinner spinner;
    @BindView(R.id.detect_bootimage) Button detectButton;
    @BindView(R.id.flash_button) CardView flashButton;
    @BindView(R.id.keep_force_enc) CheckBox keepEncChkbox;
    @BindView(R.id.keep_verity) CheckBox keepVerityChkbox;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.install_fragment, container, false);
        ButterKnife.bind(this, v);
        detectButton.setOnClickListener(v1 -> toAutoDetect());
        installTitle.setText(getString(R.string.install_magisk_title, StatusFragment.remoteMagiskVersion));
        flashButton.setOnClickListener(v1 -> {
            String bootImage = bootBlock;
            if (bootImage == null) {
                bootImage = blockList.get(spinner.getSelectedItemPosition() - 1);
            }
            Utils.dlAndReceive(
                    getActivity(),
                    new MagiskDlReceiver(bootImage, keepEncChkbox.isChecked(), keepVerityChkbox.isChecked()),
                    StatusFragment.magiskLink,
                    "Magisk-v" + String.valueOf(StatusFragment.remoteMagiskVersion) + ".zip");
        });
        if (blockDetectionDone.isTriggered) {
            updateUI();
        }
        return v;
    }

    @Override
    public void onTrigger(CallbackHandler.Event event) {
        updateUI();
    }

    private void updateUI() {
        List<String> items = new ArrayList<>(blockList);
        items.add(0, getString(R.string.auto_detect, bootBlock));
        ArrayAdapter<String> adapter = new ArrayAdapter<>(getActivity(),
                android.R.layout.simple_spinner_item, items);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        toAutoDetect();
    }

    private void toAutoDetect() {
        if (bootBlock != null) {
            spinner.setSelection(0);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle(R.string.install);
        CallbackHandler.register(blockDetectionDone, this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        CallbackHandler.unRegister(blockDetectionDone, this);
    }
}
