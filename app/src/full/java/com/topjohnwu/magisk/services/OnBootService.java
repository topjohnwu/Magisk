package com.topjohnwu.magisk.services;

import android.content.Context;
import android.content.Intent;
import android.support.annotation.NonNull;
import android.support.v4.app.JobIntentService;

import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.ShowUI;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

public class OnBootService extends JobIntentService {

    public static void enqueueWork(Context context) {
        enqueueWork(context, OnBootService.class, Const.ID.ONBOOT_SERVICE_ID, new Intent());
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
        Shell shell = Shell.newInstance();
        if (shell.getStatus() >= Shell.ROOT_SHELL &&
                Boolean.parseBoolean(ShellUtils.fastCmd(shell, "mm_patch_dtbo")))
            ShowUI.dtboPatchedNotification();
    }
}
