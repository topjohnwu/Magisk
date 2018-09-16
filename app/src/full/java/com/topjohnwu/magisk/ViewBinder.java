package com.topjohnwu.magisk;

import android.content.Context;
import android.view.View;

import com.topjohnwu.magisk.adapters.ApplicationAdapter;
import com.topjohnwu.magisk.adapters.ModulesAdapter;
import com.topjohnwu.magisk.adapters.PolicyAdapter;
import com.topjohnwu.magisk.adapters.ReposAdapter;
import com.topjohnwu.magisk.adapters.SuLogAdapter;
import com.topjohnwu.magisk.components.AboutCardRow;
import com.topjohnwu.magisk.components.CustomAlertDialog;
import com.topjohnwu.magisk.fragments.LogFragment;
import com.topjohnwu.magisk.fragments.MagiskFragment;
import com.topjohnwu.magisk.fragments.MagiskHideFragment;
import com.topjohnwu.magisk.fragments.MagiskLogFragment;
import com.topjohnwu.magisk.fragments.ModulesFragment;
import com.topjohnwu.magisk.fragments.ReposFragment;
import com.topjohnwu.magisk.fragments.SuLogFragment;
import com.topjohnwu.magisk.fragments.SuperuserFragment;
import com.topjohnwu.magisk.superuser.RequestActivity;

import androidx.core.content.ContextCompat;

public class ViewBinder {

    public static void bind(MainActivity target) {
        target.drawer = target.findViewById(R.id.drawer_layout);
        target.toolbar = target.findViewById(R.id.toolbar);
        target.navigationView = target.findViewById(R.id.nav_view);
    }

    public static void bind(AboutActivity target) {
        target.toolbar = target.findViewById(R.id.toolbar);
        target.appVersionInfo = target.findViewById(R.id.app_version_info);
        target.appChangelog = target.findViewById(R.id.app_changelog);
        target.appTranslators = target.findViewById(R.id.app_translators);
        target.appSourceCode = target.findViewById(R.id.app_source_code);
        target.supportThread = target.findViewById(R.id.support_thread);
        target.twitter = target.findViewById(R.id.follow_twitter);
    }

    public static void bind(DonationActivity target) {
        target.toolbar = target.findViewById(R.id.toolbar);
        target.paypal = target.findViewById(R.id.paypal);
        target.patreon = target.findViewById(R.id.patreon);
    }

    public static void bind(FlashActivity target) {
        target.toolbar = target.findViewById(R.id.toolbar);
        target.flashLogs = target.findViewById(R.id.txtLog);
        target.buttonPanel = target.findViewById(R.id.button_panel);
        target.sv = target.findViewById(R.id.scrollView);
        target.reboot = target.findViewById(R.id.reboot);
        target.reboot.setOnClickListener(v -> target.reboot());
        target.findViewById(R.id.no_thanks).setOnClickListener(v -> target.finish());
        target.findViewById(R.id.save_logs).setOnClickListener(v -> target.saveLogs());
    }
    
    public static void bind(RequestActivity target) {
        target.suPopup = target.findViewById(R.id.su_popup);
        target.timeout = target.findViewById(R.id.timeout);
        target.appIcon = target.findViewById(R.id.app_icon);
        target.appNameView = target.findViewById(R.id.app_name);
        target.packageNameView = target.findViewById(R.id.package_name);
        target.grant_btn = target.findViewById(R.id.grant_btn);
        target.deny_btn = target.findViewById(R.id.deny_btn);
        target.fingerprintImg = target.findViewById(R.id.fingerprint);
        target.warning = target.findViewById(R.id.warning);
    }

    public static void bind(LogFragment target, View v) {
        target.viewPager = v.findViewById(R.id.container);
        target.tab = v.findViewById(R.id.tab);
    }

    public static void unbind(LogFragment target) {
        target.viewPager = null;
        target.tab = null;
    }

    public static void bind(MagiskFragment target, View v) {
        target.mSwipeRefreshLayout = v.findViewById(R.id.swipeRefreshLayout);
        target.coreOnlyNotice = v.findViewById(R.id.core_only_notice);
        target.magiskUpdate = v.findViewById(R.id.magisk_update);
        target.magiskUpdateIcon = v.findViewById(R.id.magisk_update_icon);
        target.magiskUpdateText = v.findViewById(R.id.magisk_update_status);
        target.magiskUpdateProgress = v.findViewById(R.id.magisk_update_progress);
        target.magiskStatusIcon = v.findViewById(R.id.magisk_status_icon);
        target.magiskVersionText = v.findViewById(R.id.magisk_version);
        target.safetyNetCard = v.findViewById(R.id.safetyNet_card);
        target.safetyNetRefreshIcon = v.findViewById(R.id.safetyNet_refresh);
        target.safetyNetStatusText = v.findViewById(R.id.safetyNet_status);
        target.safetyNetProgress = v.findViewById(R.id.safetyNet_check_progress);
        target.expandLayout = v.findViewById(R.id.expand_layout);
        target.ctsStatusIcon = v.findViewById(R.id.cts_status_icon);
        target.ctsStatusText = v.findViewById(R.id.cts_status);
        target.basicStatusIcon = v.findViewById(R.id.basic_status_icon);
        target.basicStatusText = v.findViewById(R.id.basic_status);
        target.installOptionCard = v.findViewById(R.id.install_option_card);
        target.keepEncChkbox = v.findViewById(R.id.keep_force_enc);
        target.keepVerityChkbox = v.findViewById(R.id.keep_verity);
        target.installButton = v.findViewById(R.id.install_button);
        target.installText = v.findViewById(R.id.install_text);
        target.uninstallButton = v.findViewById(R.id.uninstall_button);

        v.findViewById(R.id.safetyNet_title).setOnClickListener(v1 -> target.safetyNet());
        v.findViewById(R.id.install_button).setOnClickListener(v1 -> target.install());
        v.findViewById(R.id.uninstall_button).setOnClickListener(v1 -> target.uninstall());

        Context ctx = target.getContext();
        target.colorBad = ContextCompat.getColor(ctx, R.color.red500);
        target.colorOK = ContextCompat.getColor(ctx, R.color.green500);
        target.colorWarn = ContextCompat.getColor(ctx, R.color.yellow500);
        target.colorNeutral = ContextCompat.getColor(ctx, R.color.grey500);
        target.colorInfo = ContextCompat.getColor(ctx, R.color.blue500);
    }

    public static void unbind(MagiskFragment target) {
        target.mSwipeRefreshLayout = null;
        target.coreOnlyNotice = null;
        target.magiskUpdate = null;
        target.magiskUpdateIcon = null;
        target.magiskUpdateText = null;
        target.magiskUpdateProgress = null;
        target.magiskStatusIcon = null;
        target.magiskVersionText = null;
        target.safetyNetCard = null;
        target.safetyNetRefreshIcon = null;
        target.safetyNetStatusText = null;
        target.safetyNetProgress = null;
        target.expandLayout = null;
        target.ctsStatusIcon = null;
        target.ctsStatusText = null;
        target.basicStatusIcon = null;
        target.basicStatusText = null;
        target.installOptionCard = null;
        target.keepEncChkbox = null;
        target.keepVerityChkbox = null;
        target.installButton = null;
        target.installText = null;
        target.uninstallButton = null;

        View v = target.getView();
        v.findViewById(R.id.safetyNet_title).setOnClickListener(null);
        v.findViewById(R.id.install_button).setOnClickListener(null);
        v.findViewById(R.id.uninstall_button).setOnClickListener(null);
    }

    public static void bind(MagiskHideFragment target, View v) {
        target.mSwipeRefreshLayout = v.findViewById(R.id.swipeRefreshLayout);
        target.recyclerView = v.findViewById(R.id.recyclerView);
    }

    public static void unbind(MagiskHideFragment target) {
        target.mSwipeRefreshLayout = null;
        target.recyclerView = null;
    }
    
    public static void bind(MagiskLogFragment target, View v) {
        target.txtLog = v.findViewById(R.id.txtLog);
        target.svLog = v.findViewById(R.id.svLog);
        target.hsvLog = v.findViewById(R.id.hsvLog);
        target.progressBar = v.findViewById(R.id.progressBar);
    }
    
    public static void unbind(MagiskLogFragment target) {
        target.txtLog = null;
        target.svLog = null;
        target.hsvLog = null;
        target.progressBar = null;
    }
    
    public static void bind(ModulesFragment target, View v) {
        target.mSwipeRefreshLayout = v.findViewById(R.id.swipeRefreshLayout);
        target.recyclerView = v.findViewById(R.id.recyclerView);
        target.emptyRv = v.findViewById(R.id.empty_rv);
        v.findViewById(R.id.fab).setOnClickListener(v1 -> target.selectFile());
    }

    public static void unbind(ModulesFragment target) {
        target.mSwipeRefreshLayout = null;
        target.recyclerView = null;
        target.emptyRv = null;
        View v = target.getView();
        v.findViewById(R.id.fab).setOnClickListener(null);
    }
    
    public static void bind(ReposFragment target, View source) {
        target.recyclerView = source.findViewById(R.id.recyclerView);
        target.emptyRv = source.findViewById(R.id.empty_rv);
        target.mSwipeRefreshLayout = source.findViewById(R.id.swipeRefreshLayout);
    }
    
    public static void unbind(ReposFragment target) {
        target.recyclerView = null;
        target.emptyRv = null;
        target.mSwipeRefreshLayout = null;
    }
    
    public static void bind(SuLogFragment target, View source) {
        target.emptyRv = source.findViewById(R.id.empty_rv);
        target.recyclerView = source.findViewById(R.id.recyclerView);
    }
    
    public static void unbind(SuLogFragment target) {
        target.emptyRv = null;
        target.recyclerView = null;
    }
    
    public static void bind(SuperuserFragment target, View source) {
        target.recyclerView = source.findViewById(R.id.recyclerView);
        target.emptyRv = source.findViewById(R.id.empty_rv);
    }
    
    public static void unbind(SuperuserFragment target) {
        target.emptyRv = null;
        target.recyclerView = null;
    }
    
    public static void bind(CustomAlertDialog.ViewHolder target, View source) {
        target.dialogLayout = source.findViewById(R.id.dialog_layout);
        target.buttons = source.findViewById(R.id.button_panel);
        target.messageView = source.findViewById(R.id.message);
        target.negative = source.findViewById(R.id.negative);
        target.positive = source.findViewById(R.id.positive);
        target.neutral = source.findViewById(R.id.neutral);
    }
    
    public static void bind(AboutCardRow target, View source) {
        target.mTitle = source.findViewById(android.R.id.title);
        target.mSummary = source.findViewById(android.R.id.summary);
        target.mIcon = source.findViewById(android.R.id.icon);
        target.mView = source.findViewById(R.id.container);
    }
    
    public static void bind(ApplicationAdapter.ViewHolder target, View source) {
        target.appIcon = source.findViewById(R.id.app_icon);
        target.appName = source.findViewById(R.id.app_name);
        target.appPackage = source.findViewById(R.id.package_name);
        target.checkBox = source.findViewById(R.id.checkbox);
    }
    
    public static void bind(ModulesAdapter.ViewHolder target, View source) {
        target.title = source.findViewById(R.id.title);
        target.versionName = source.findViewById(R.id.version_name);
        target.description = source.findViewById(R.id.description);
        target.notice = source.findViewById(R.id.notice);
        target.checkBox = source.findViewById(R.id.checkbox);
        target.author = source.findViewById(R.id.author);
        target.delete = source.findViewById(R.id.delete);
    }

    public static void bind(PolicyAdapter.ViewHolder target, View source) {
        target.appName = source.findViewById(R.id.app_name);
        target.packageName = source.findViewById(R.id.package_name);
        target.appIcon = source.findViewById(R.id.app_icon);
        target.masterSwitch = source.findViewById(R.id.master_switch);
        target.notificationSwitch = source.findViewById(R.id.notification_switch);
        target.loggingSwitch = source.findViewById(R.id.logging_switch);
        target.expandLayout = source.findViewById(R.id.expand_layout);
        target.delete = source.findViewById(R.id.delete);
        target.moreInfo = source.findViewById(R.id.more_info);
    }

    public static void bind(ReposAdapter.SectionHolder target, View source) {
        target.sectionText = source.findViewById(R.id.section_text);
    }

    public static void bind(ReposAdapter.RepoHolder target, View source) {
        target.title = source.findViewById(R.id.title);
        target.versionName = source.findViewById(R.id.version_name);
        target.description = source.findViewById(R.id.description);
        target.author = source.findViewById(R.id.author);
        target.infoLayout = source.findViewById(R.id.info_layout);
        target.downloadImage = source.findViewById(R.id.download);
        target.updateTime = source.findViewById(R.id.update_time);
    }

    public static void bind(SuLogAdapter.SectionHolder target, View source) {
        target.date = source.findViewById(R.id.date);
        target.arrow = source.findViewById(R.id.arrow);
    }

    public static void bind(SuLogAdapter.LogViewHolder target, View source) {
        target.appName = source.findViewById(R.id.app_name);
        target.action = source.findViewById(R.id.action);
        target.time = source.findViewById(R.id.time);
        target.fromPid = source.findViewById(R.id.fromPid);
        target.toUid = source.findViewById(R.id.toUid);
        target.command = source.findViewById(R.id.command);
        target.expandLayout = source.findViewById(R.id.expand_layout);
    }
}
