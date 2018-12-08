package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.utils.ISafetyNetHelper;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.WebService;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;

import dalvik.system.DexClassLoader;

public class CheckSafetyNet extends ParallelTask<Void, Void, Void> {

    public static final File dexPath =
            new File(Data.MM().getFilesDir().getParent() + "/snet", "snet.apk");
    private ISafetyNetHelper helper;

    public CheckSafetyNet(Activity activity) {
        super(activity);
    }

    private void dlSnet() throws Exception {
        Shell.sh("rm -rf " + dexPath.getParent()).exec();
        dexPath.getParentFile().mkdir();
        HttpURLConnection conn = WebService.mustRequest(Const.Url.SNET_URL);
        try (
                OutputStream out = new BufferedOutputStream(new FileOutputStream(dexPath));
                InputStream in = new BufferedInputStream(conn.getInputStream())) {
            ShellUtils.pump(in, out);
        } finally {
            conn.disconnect();
        }
    }

    private void dyload() throws Exception {
        DexClassLoader loader = new DexClassLoader(dexPath.getPath(), dexPath.getParent(),
                null, ISafetyNetHelper.class.getClassLoader());
        Class<?> clazz = loader.loadClass("com.topjohnwu.snet.Snet");
        helper = (ISafetyNetHelper) clazz.getMethod("newHelper",
                Class.class, String.class, Activity.class, Object.class)
                .invoke(null, ISafetyNetHelper.class, dexPath.getPath(), getActivity(),
                        (ISafetyNetHelper.Callback) code ->
                                Topic.publish(false, Topic.SNET_CHECK_DONE, code));
        if (helper.getVersion() < Const.SNET_EXT_VER) {
            throw new Exception();
        }
    }

    @Override
    protected Void doInBackground(Void... voids) {
        try {
            try {
                dyload();
            } catch (Exception e) {
                // If dynamic load failed, try re-downloading and reload
                dlSnet();
                dyload();
            }
            // Run attestation
            helper.attest();
        } catch (Exception e) {
            e.printStackTrace();
            Topic.publish(false, Topic.SNET_CHECK_DONE, -1);
        }
        return null;
    }
}
