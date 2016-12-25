package com.topjohnwu.magisk;

import android.app.Fragment;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.CallbackHandler;

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

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.install_fragment, container, false);
        ButterKnife.bind(this, v);
        detectButton.setOnClickListener(v1 -> toAutoDetect());
        installTitle.setText(getString(R.string.install_magisk_title, StatusFragment.remoteMagiskVersion));
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
        blockList.add(0, getString(R.string.auto_detect, bootBlock));
        ArrayAdapter<String> adapter = new ArrayAdapter<>(getActivity(),
                android.R.layout.simple_spinner_item, blockList);
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

//    private AlertDialog.OnClickListener flashMagisk = (dialogInterface, i) -> Utils.dlAndReceive(
//            getActivity(),
//            new DownloadReceiver() {
//                @Override
//                public void task(Uri uri) {
//                    new Async.FlashZIP(mContext, uri, mFilename) {
//                        @Override
//                        protected boolean unzipAndCheck() {
//                            publishProgress(mContext.getString(R.string.zip_install_unzip_zip_msg));
//                            if (Shell.rootAccess()) {
//                                // We might not have busybox yet, unzip with Java
//                                // We will have complete busybox after Magisk installation
//                                ZipUtils.unzip(mCachedFile, new File(mCachedFile.getParent(), "magisk"));
//                                Shell.su(
//                                        "mkdir -p " + Async.TMP_FOLDER_PATH + "/magisk",
//                                        "cp -af " + mCachedFile.getParent() + "/magisk/. " + Async.TMP_FOLDER_PATH + "/magisk"
//                                );
//                            }
//                            super.unzipAndCheck();
//                            return true;
//                        }
//
//                        @Override
//                        protected void done() {
//                            Shell.su("setprop magisk.version " + String.valueOf(StatusFragment.remoteMagiskVersion));
//                            super.done();
//                        }
//                    }.exec();
//                }
//            },
//            StatusFragment.magiskLink,
//            "Magisk-v" + String.valueOf(StatusFragment.remoteMagiskVersion) + ".zip");
