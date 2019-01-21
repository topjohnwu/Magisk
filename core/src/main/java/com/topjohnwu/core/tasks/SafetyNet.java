package com.topjohnwu.core.tasks;

import android.app.Activity;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.utils.ISafetyNetHelper;
import com.topjohnwu.core.utils.Topic;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;

import java.io.File;

import dalvik.system.DexClassLoader;

public class SafetyNet {

    public static final File EXT_APK =
            new File(App.self.getFilesDir().getParent() + "/snet", "snet.apk");

    private static void dyRun(Activity activity) throws Exception {
        DexClassLoader loader = new DexClassLoader(EXT_APK.getPath(), EXT_APK.getParent(),
                null, ISafetyNetHelper.class.getClassLoader());
        Class<?> clazz = loader.loadClass("com.topjohnwu.snet.Snet");
        ISafetyNetHelper helper = (ISafetyNetHelper) clazz.getMethod("newHelper",
                Class.class, String.class, Activity.class, Object.class)
                .invoke(null, ISafetyNetHelper.class, EXT_APK.getPath(), activity,
                        (ISafetyNetHelper.Callback) code ->
                                Topic.publish(false, Topic.SNET_CHECK_DONE, code));
        if (helper.getVersion() < Const.SNET_EXT_VER)
            throw new Exception();
        helper.attest();
    }

    public static void check(Activity activity) {
        try {
            dyRun(activity);
        } catch (Exception ignored) {
            Shell.sh("rm -rf " + EXT_APK.getParent()).exec();
            EXT_APK.getParentFile().mkdir();
            Networking.get(Const.Url.SNET_URL).getAsFile(EXT_APK, f -> {
                try {
                    dyRun(activity);
                } catch (Exception e) {
                    e.printStackTrace();
                    Topic.publish(false, Topic.SNET_CHECK_DONE, -1);
                }
            });
        }
    }
}
