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
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Proxy;
import java.net.HttpURLConnection;

import dalvik.system.DexClassLoader;

public class CheckSafetyNet extends ParallelTask<Void, Void, Exception> {

    private File dexPath;
    private DexClassLoader loader;

    public CheckSafetyNet(Activity activity) {
        super(activity);
        dexPath = new File(activity.getCacheDir().getParent() + "/snet", "snet.apk");
    }

    @Override
    protected void onPreExecute() {
        MagiskManager mm = MagiskManager.get();
        if (mm.snetVersion != Const.Value.SNET_VER) {
            Shell.sh("rm -rf " + dexPath.getParent());
        }
        mm.snetVersion = Const.Value.SNET_VER;
        mm.prefs.edit().putInt(Const.Key.SNET_VER, Const.Value.SNET_VER).apply();
    }

    @Override
    protected Exception doInBackground(Void... voids) {
        try {
            if (!dexPath.exists()) {
                HttpURLConnection conn = WebService.request(Const.Url.SNET_URL, null);
                dexPath.getParentFile().mkdir();
                try (
                        OutputStream out = new BufferedOutputStream(new FileOutputStream(dexPath));
                        InputStream in = new BufferedInputStream(conn.getInputStream())) {
                    Utils.inToOut(in, out);
                }
                conn.disconnect();
            }
            loader = new DexClassLoader(dexPath.toString(), dexPath.getParent(),
                    null, ClassLoader.getSystemClassLoader());
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
            Class<?> helperClazz = loader.loadClass(Const.SNET_PKG + ".SafetyNetHelper");
            Class<?> callbackClazz = loader.loadClass(Const.SNET_PKG + ".SafetyNetCallback");
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
