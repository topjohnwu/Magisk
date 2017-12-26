package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Proxy;
import java.net.HttpURLConnection;

import dalvik.system.DexClassLoader;

public class CheckSafetyNet extends ParallelTask<Void, Void, Exception> {

    public static final File dexPath =
            new File(MagiskManager.get().getFilesDir().getParent() + "/snet", "snet.apk");
    private DexClassLoader loader;
    private Class<?> helperClazz, callbackClazz;

    public CheckSafetyNet(Activity activity) {
        super(activity);
    }

    private void dlSnet() throws IOException {
        Shell.sh("rm -rf " + dexPath.getParent());
        HttpURLConnection conn = WebService.request(Const.Url.SNET_URL, null);
        dexPath.getParentFile().mkdir();
        try (
                OutputStream out = new BufferedOutputStream(new FileOutputStream(dexPath));
                InputStream in = new BufferedInputStream(conn.getInputStream())) {
            Utils.inToOut(in, out);
        }
        conn.disconnect();
    }

    private void loadClasses() throws ClassNotFoundException {
        loader = new DexClassLoader(dexPath.toString(), dexPath.getParent(),
                null, ClassLoader.getSystemClassLoader());
        helperClazz = loader.loadClass(Const.SNET_PKG + ".SafetyNetHelper");
        callbackClazz = loader.loadClass(Const.SNET_PKG + ".SafetyNetCallback");
    }

    @Override
    protected Exception doInBackground(Void... voids) {
        int snet_ver = -1;

        try {
            if (!dexPath.exists())
                dlSnet();
            loadClasses();

            try {
                snet_ver = (int) helperClazz.getMethod("getVersion").invoke(null);
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            }

            if (snet_ver != Const.SNET_VER) {
                dlSnet();
                loadClasses();
            }
        } catch (Exception e) {
            return e;
        }

        return null;
    }

    @Override
    protected void onPostExecute(Exception err) {
        MagiskManager mm = MagiskManager.get();
        try {
            if (err != null) throw err;
            Object helper = helperClazz.getConstructors()[0].newInstance(
                    getActivity(), dexPath.getPath(), Proxy.newProxyInstance(
                            loader, new Class[] { callbackClazz }, (proxy, method, args) -> {
                                mm.safetyNetDone.publish(false, args[0]);
                                return null;
                            }));
            helperClazz.getMethod("attest").invoke(helper);
        } catch (Exception e) {
            e.printStackTrace();
            mm.safetyNetDone.publish(false, -1);
        }
        super.onPostExecute(err);
    }
}
