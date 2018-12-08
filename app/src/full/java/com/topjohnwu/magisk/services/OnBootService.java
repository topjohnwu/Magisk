package com.topjohnwu.magisk.services;

import android.content.Context;
import android.content.Intent;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.components.Notifications;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import androidx.annotation.NonNull;
import androidx.core.app.JobIntentService;

public class OnBootService extends JobIntentService {

    public static void enqueueWork(Context context) {
        enqueueWork(context, Data.classMap.get(OnBootService.class), Const.ID.ONBOOT_SERVICE_ID, new Intent());
    }

    @Override
    protected void onHandleWork(@NonNull Intent intent) {
        /* Devices with DTBO might want to patch dtbo.img.
         * However, that is not possible if Magisk is installed by
         * patching boot image with Magisk Manager and flashed via
         * fastboot, since at that time we do not have root.
         * Check for dtbo status every boot time, and prompt user
         * to reboot if dtbo wasn't patched and patched by Magisk Manager.
         * */
        if (Shell.rootAccess() && ShellUtils.fastCmdResult("mm_patch_dtbo"))
            Notifications.dtboPatched();
    }
}
