package com.topjohnwu.magisk;

import android.app.Fragment;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.File;

import butterknife.ButterKnife;

// Currently empty, placing some code that we be used in the future
public class InstallFragment extends Fragment {

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.install_fragment, container, false);
        ButterKnife.bind(this, v);
        return v;
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle(R.string.install);
    }

    private AlertDialog.OnClickListener flashMagisk = (dialogInterface, i) -> Utils.dlAndReceive(
            getActivity(),
            new DownloadReceiver() {
                @Override
                public void task(Uri uri) {
                    new Async.FlashZIP(mContext, uri, mFilename) {
                        @Override
                        protected boolean unzipAndCheck() {
                            publishProgress(mContext.getString(R.string.zip_install_unzip_zip_msg));
                            if (Shell.rootAccess()) {
                                // We might not have busybox yet, unzip with Java
                                // We will have complete busybox after Magisk installation
                                ZipUtils.unzip(mCachedFile, new File(mCachedFile.getParent(), "magisk"));
                                Shell.su(
                                        "mkdir -p " + Async.TMP_FOLDER_PATH + "/magisk",
                                        "cp -af " + mCachedFile.getParent() + "/magisk/. " + Async.TMP_FOLDER_PATH + "/magisk"
                                );
                            }
                            super.unzipAndCheck();
                            return true;
                        }

                        @Override
                        protected void done() {
                            Shell.su("setprop magisk.version " + String.valueOf(StatusFragment.remoteMagiskVersion));
                            super.done();
                        }
                    }.exec();
                }
            },
            StatusFragment.magiskLink,
            "Magisk-v" + String.valueOf(StatusFragment.remoteMagiskVersion) + ".zip");
}
