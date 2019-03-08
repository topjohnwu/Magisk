package com.topjohnwu.magisk.uicomponents;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.StringRes;
import androidx.cardview.widget.CardView;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.dialogs.CustomAlertDialog;
import com.topjohnwu.magisk.utils.ISafetyNetHelper;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;

import java.io.File;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;
import dalvik.system.DexClassLoader;

public class SafetyNet implements ISafetyNetHelper.Callback {

    private static final File EXT_APK =
            new File(App.self.getFilesDir().getParent() + "/snet", "snet.apk");

    @BindView(R.id.safetyNet_card) CardView safetyNetCard;
    @BindView(R.id.safetyNet_refresh) ImageView safetyNetRefreshIcon;
    @BindView(R.id.safetyNet_status) TextView safetyNetStatusText;
    @BindView(R.id.safetyNet_check_progress) ProgressBar safetyNetProgress;
    @BindView(R.id.safetyNet_expand) ViewGroup expandLayout;
    @BindView(R.id.cts_status_icon) ImageView ctsStatusIcon;
    @BindView(R.id.cts_status) TextView ctsStatusText;
    @BindView(R.id.basic_status_icon) ImageView basicStatusIcon;
    @BindView(R.id.basic_status) TextView basicStatusText;

    @BindColor(R.color.red500) int colorBad;
    @BindColor(R.color.green500) int colorOK;

    public Unbinder unbinder;
    private ExpandableViewHolder expandable;

    public SafetyNet(View v) {
        unbinder = new SafetyNet_ViewBinding(this, v);
        expandable = new ExpandableViewHolder(expandLayout);
        Context context = v.getContext();
        safetyNetCard.setVisibility(hasGms(context) && Networking.checkNetworkStatus(context) ?
                View.VISIBLE : View.GONE);
    }

    @OnClick(R.id.safetyNet_refresh)
    void safetyNet(View v) {
        Runnable task = () -> {
            safetyNetProgress.setVisibility(View.VISIBLE);
            safetyNetRefreshIcon.setVisibility(View.INVISIBLE);
            safetyNetStatusText.setText(R.string.checking_safetyNet_status);
            check((Activity) v.getContext());
            expandable.collapse();
        };
        if (!SafetyNet.EXT_APK.exists()) {
            // Show dialog
            new CustomAlertDialog((Activity) v.getContext())
                    .setTitle(R.string.proprietary_title)
                    .setMessage(R.string.proprietary_notice)
                    .setCancelable(true)
                    .setPositiveButton(R.string.yes, (d, i) -> task.run())
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        } else {
            task.run();
        }
    }

    public void reset() {
        safetyNetStatusText.setText(R.string.safetyNet_check_text);
        expandable.setExpanded(false);
    }

    @Override
    public void onResponse(int response) {
        safetyNetProgress.setVisibility(View.GONE);
        safetyNetRefreshIcon.setVisibility(View.VISIBLE);
        if ((response & 0x0F) == 0) {
            safetyNetStatusText.setText(R.string.safetyNet_check_success);

            boolean b;
            b = (response & ISafetyNetHelper.CTS_PASS) != 0;
            ctsStatusText.setText("ctsProfile: " + b);
            ctsStatusIcon.setImageResource(b ? R.drawable.ic_check_circle : R.drawable.ic_cancel);
            ctsStatusIcon.setColorFilter(b ? colorOK : colorBad);

            b = (response & ISafetyNetHelper.BASIC_PASS) != 0;
            basicStatusText.setText("basicIntegrity: " + b);
            basicStatusIcon.setImageResource(b ? R.drawable.ic_check_circle : R.drawable.ic_cancel);
            basicStatusIcon.setColorFilter(b ? colorOK : colorBad);

            expandable.expand();
        } else {
            @StringRes int resid;
            switch (response) {
                case ISafetyNetHelper.RESPONSE_ERR:
                    resid = R.string.safetyNet_res_invalid;
                    break;
                case ISafetyNetHelper.CONNECTION_FAIL:
                default:
                    resid = R.string.safetyNet_api_error;
                    break;
            }
            safetyNetStatusText.setText(resid);
        }
    }

    private void dyRun(Activity activity) throws Exception {
        DexClassLoader loader = new DexClassLoader(EXT_APK.getPath(), EXT_APK.getParent(),
                null, ISafetyNetHelper.class.getClassLoader());
        Class<?> clazz = loader.loadClass("com.topjohnwu.snet.Snet");
        ISafetyNetHelper helper = (ISafetyNetHelper) clazz.getMethod("newHelper",
                Class.class, String.class, Activity.class, Object.class)
                .invoke(null, ISafetyNetHelper.class, EXT_APK.getPath(), activity, this);
        if (helper.getVersion() < Const.SNET_EXT_VER)
            throw new Exception();
        helper.attest();
    }

    private void check(Activity activity) {
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
                    onResponse(-1);
                }
            });
        }
    }

    private boolean hasGms(Context context) {
        PackageManager pm = context.getPackageManager();
        PackageInfo info;
        try {
            info = pm.getPackageInfo("com.google.android.gms", 0);
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
        return info.applicationInfo.enabled;
    }
}
