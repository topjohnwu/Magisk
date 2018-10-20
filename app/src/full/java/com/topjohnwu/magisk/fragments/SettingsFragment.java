package com.topjohnwu.magisk.fragments;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.PatchAPK;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.FingerprintHelper;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.IOException;
import java.util.Locale;

import androidx.appcompat.app.AlertDialog;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceGroupAdapter;
import androidx.preference.PreferenceScreen;
import androidx.preference.PreferenceViewHolder;
import androidx.preference.SwitchPreference;
import androidx.recyclerview.widget.RecyclerView;

public class SettingsFragment extends PreferenceFragmentCompat
        implements SharedPreferences.OnSharedPreferenceChangeListener,
        Topic.Subscriber, Topic.AutoSubscriber {

    private PreferenceScreen prefScreen;

    private ListPreference updateChannel, suAccess, autoRes, suNotification,
            requestTimeout, multiuserMode, namespaceMode;
    private MagiskManager mm;
    private PreferenceCategory generalCatagory;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.app_settings, rootKey);
        mm = Data.MM();
        prefScreen = getPreferenceScreen();

        generalCatagory = (PreferenceCategory) findPreference("general");
        PreferenceCategory magiskCategory = (PreferenceCategory) findPreference("magisk");
        PreferenceCategory suCategory = (PreferenceCategory) findPreference("superuser");
        Preference hideManager = findPreference("hide");
        Preference restoreManager = findPreference("restore");
        findPreference("clear").setOnPreferenceClickListener((pref) -> {
            mm.prefs.edit().remove(Const.Key.ETAG_KEY).apply();
            mm.repoDB.clearRepo();
            Utils.toast(R.string.repo_cache_cleared, Toast.LENGTH_SHORT);
            return true;
        });

        updateChannel = (ListPreference) findPreference(Const.Key.UPDATE_CHANNEL);
        suAccess = (ListPreference) findPreference(Const.Key.ROOT_ACCESS);
        autoRes = (ListPreference) findPreference(Const.Key.SU_AUTO_RESPONSE);
        requestTimeout = (ListPreference) findPreference(Const.Key.SU_REQUEST_TIMEOUT);
        suNotification = (ListPreference) findPreference(Const.Key.SU_NOTIFICATION);
        multiuserMode = (ListPreference) findPreference(Const.Key.SU_MULTIUSER_MODE);
        namespaceMode = (ListPreference) findPreference(Const.Key.SU_MNT_NS);
        SwitchPreference reauth = (SwitchPreference) findPreference(Const.Key.SU_REAUTH);
        SwitchPreference fingerprint = (SwitchPreference) findPreference(Const.Key.SU_FINGERPRINT);

        updateChannel.setOnPreferenceChangeListener((p, o) -> {
            String prev =String.valueOf(Data.updateChannel);
            int channel = Integer.parseInt((String) o);
            if (channel == Const.Value.CUSTOM_CHANNEL) {
                View v = LayoutInflater.from(requireActivity()).inflate(R.layout.custom_channel_dialog, null);
                EditText url = v.findViewById(R.id.custom_url);
                url.setText(mm.prefs.getString(Const.Key.CUSTOM_CHANNEL, ""));
                new AlertDialog.Builder(requireActivity())
                        .setTitle(R.string.settings_update_custom)
                        .setView(v)
                        .setPositiveButton(R.string.ok, (d, i) ->
                                mm.prefs.edit().putString(Const.Key.CUSTOM_CHANNEL,
                                        url.getText().toString()).apply())
                        .setNegativeButton(R.string.close, (d, i) ->
                                mm.prefs.edit().putString(Const.Key.UPDATE_CHANNEL, prev).apply())
                        .setOnCancelListener(d ->
                                mm.prefs.edit().putString(Const.Key.UPDATE_CHANNEL, prev).apply())
                        .show();
            }
            return true;
        });

        setSummary();

        // Disable dangerous settings in secondary user
        if (Const.USER_ID > 0) {
            suCategory.removePreference(multiuserMode);
        }

        // Disable re-authentication option on Android O, it will not work
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            reauth.setEnabled(false);
            reauth.setChecked(false);
            reauth.setSummary(R.string.android_o_not_support);
        }

        // Disable fingerprint option if not possible
        if (!FingerprintHelper.canUseFingerprint()) {
            fingerprint.setEnabled(false);
            fingerprint.setChecked(false);
            fingerprint.setSummary(R.string.disable_fingerprint);
        }

        if (Shell.rootAccess()) {
            if (mm.getPackageName().equals(Const.ORIG_PKG_NAME)) {
                hideManager.setOnPreferenceClickListener((pref) -> {
                    PatchAPK.hideManager(requireActivity());
                    return true;
                });
                generalCatagory.removePreference(restoreManager);
            } else {
                if (Download.checkNetworkStatus(mm)) {
                    restoreManager.setOnPreferenceClickListener((pref) -> {
                        Download.receive(
                            requireActivity(), new DownloadReceiver() {
                                @Override
                                public void onDownloadDone(Context context, Uri uri) {
                                    Data.exportPrefs();
                                    Shell.su("cp " + uri.getPath() + " /data/local/tmp/manager.apk").exec();
                                    if (ShellUtils.fastCmdResult("pm install /data/local/tmp/manager.apk")) {
                                        Shell.su("rm -f /data/local/tmp/manager.apk").exec();
                                        RootUtils.uninstallPkg(context.getPackageName());
                                        return;
                                    }
                                    Shell.su("rm -f /data/local/tmp/manager.apk").exec();
                                }
                            },
                            Data.managerLink,
                            Utils.fmt("MagiskManager-v%s.apk", Data.remoteManagerVersionString)
                        );
                        return true;
                    });
                } else {
                    generalCatagory.removePreference(restoreManager);
                }
                generalCatagory.removePreference(hideManager);
            }
        } else {
            generalCatagory.removePreference(restoreManager);
            generalCatagory.removePreference(hideManager);
        }

        if (!Shell.rootAccess() || (Const.USER_ID > 0 &&
                Data.multiuserMode == Const.Value.MULTIUSER_MODE_OWNER_MANAGED)) {
            prefScreen.removePreference(suCategory);
        }

        if (!Shell.rootAccess()) {
            prefScreen.removePreference(magiskCategory);
            generalCatagory.removePreference(hideManager);
        }
    }

    private void setLocalePreference(ListPreference lp) {
        CharSequence[] entries = new CharSequence[LocaleManager.locales.size() + 1];
        CharSequence[] entryValues = new CharSequence[LocaleManager.locales.size() + 1];
        entries[0] = LocaleManager.getString(LocaleManager.defaultLocale, R.string.system_default);
        entryValues[0] = "";
        int i = 1;
        for (Locale locale : LocaleManager.locales) {
            entries[i] = locale.getDisplayName(locale);
            entryValues[i++] = locale.toLanguageTag();
        }
        lp.setEntries(entries);
        lp.setEntryValues(entryValues);
        lp.setSummary(LocaleManager.locale.getDisplayName(LocaleManager.locale));
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mm.prefs.registerOnSharedPreferenceChangeListener(this);
        Topic.subscribe(this);
        requireActivity().setTitle(R.string.settings);
        return super.onCreateView(inflater, container, savedInstanceState);
    }

    @Override
    public void onDestroyView() {
        mm.prefs.unregisterOnSharedPreferenceChangeListener(this);
        Topic.unsubscribe(this);
        super.onDestroyView();
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
        switch (key) {
            case Const.Key.ROOT_ACCESS:
            case Const.Key.SU_MULTIUSER_MODE:
            case Const.Key.SU_MNT_NS:
                mm.mDB.setSettings(key, Utils.getPrefsInt(prefs, key));
                break;
        }
        Data.loadConfig();
        setSummary();
        switch (key) {
            case Const.Key.DARK_THEME:
                requireActivity().recreate();
                break;
            case Const.Key.COREONLY:
                if (prefs.getBoolean(key, false)) {
                    try {
                        Const.MAGISK_DISABLE_FILE.createNewFile();
                    } catch (IOException ignored) {}
                } else {
                    Const.MAGISK_DISABLE_FILE.delete();
                }
                Utils.toast(R.string.settings_reboot_toast, Toast.LENGTH_LONG);
                break;
            case Const.Key.MAGISKHIDE:
                if (prefs.getBoolean(key, false)) {
                    Shell.su("magiskhide --enable").submit();
                } else {
                    Shell.su("magiskhide --disable").submit();
                }
                break;
            case Const.Key.HOSTS:
                if (prefs.getBoolean(key, false)) {
                    Shell.su("cp -af /system/etc/hosts " + Const.MAGISK_HOST_FILE,
                            "mount -o bind " + Const.MAGISK_HOST_FILE + " /system/etc/hosts")
                            .submit();
                } else {
                    Shell.su("umount -l /system/etc/hosts",
                            "rm -f " + Const.MAGISK_HOST_FILE)
                            .submit();
                }
                break;
            case Const.Key.LOCALE:
                LocaleManager.setLocale(mm);
                requireActivity().recreate();
                break;
            case Const.Key.UPDATE_CHANNEL:
            case Const.Key.CUSTOM_CHANNEL:
                CheckUpdates.check();
                break;
            case Const.Key.CHECK_UPDATES:
                Utils.setupUpdateCheck();
                break;
        }
    }

    @Override
    public boolean onPreferenceTreeClick(Preference preference) {
        String key = preference.getKey();
        switch (key) {
            case Const.Key.SU_FINGERPRINT:
                boolean checked = ((SwitchPreference) preference).isChecked();
                ((SwitchPreference) preference).setChecked(!checked);
                FingerprintHelper.showAuthDialog(requireActivity(), () -> {
                    ((SwitchPreference) preference).setChecked(checked);
                    Data.suFingerprint = checked;
                    mm.mDB.setSettings(key, checked ? 1 : 0);
                });
                break;
        }
        return true;
    }

    private void setSummary() {
        updateChannel.setSummary(getResources()
                .getStringArray(R.array.update_channel)[Data.updateChannel]);
        suAccess.setSummary(getResources()
                .getStringArray(R.array.su_access)[Data.suAccessState]);
        autoRes.setSummary(getResources()
                .getStringArray(R.array.auto_response)[Data.suResponseType]);
        suNotification.setSummary(getResources()
                .getStringArray(R.array.su_notification)[Data.suNotificationType]);
        requestTimeout.setSummary(
                getString(R.string.request_timeout_summary,
                        mm.prefs.getString(Const.Key.SU_REQUEST_TIMEOUT, "10")));
        multiuserMode.setSummary(getResources()
                .getStringArray(R.array.multiuser_summary)[Data.multiuserMode]);
        namespaceMode.setSummary(getResources()
                .getStringArray(R.array.namespace_summary)[Data.suNamespaceMode]);
    }

    @Override
    public void onPublish(int topic, Object[] result) {
        setLocalePreference((ListPreference) findPreference(Const.Key.LOCALE));
    }

    @Override
    public int[] getSubscribedTopics() {
        return new int[] {Topic.LOCALE_FETCH_DONE};
    }

    @Override
    protected RecyclerView.Adapter onCreateAdapter(PreferenceScreen preferenceScreen) {
        return new PreferenceGroupAdapter(preferenceScreen) {
            @SuppressLint("RestrictedApi")
            @Override
            public void onBindViewHolder(PreferenceViewHolder holder, int position) {
                super.onBindViewHolder(holder, position);
                Preference preference = getItem(position);
                if (preference instanceof PreferenceCategory)
                    setZeroPaddingToLayoutChildren(holder.itemView);
                else {
                    View iconFrame = holder.itemView.findViewById(R.id.icon_frame);
                    if (iconFrame != null) {
                        iconFrame.setVisibility(preference.getIcon() == null ? View.GONE : View.VISIBLE);
                    }
                }
            }
        };
    }

    private void setZeroPaddingToLayoutChildren(View view) {
        if (!(view instanceof ViewGroup))
            return;
        ViewGroup viewGroup = (ViewGroup) view;
        int childCount = viewGroup.getChildCount();
        for (int i = 0; i < childCount; i++) {
            setZeroPaddingToLayoutChildren(viewGroup.getChildAt(i));
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1)
                viewGroup.setPaddingRelative(0, viewGroup.getPaddingTop(), viewGroup.getPaddingEnd(), viewGroup.getPaddingBottom());
            else
                viewGroup.setPadding(0, viewGroup.getPaddingTop(), viewGroup.getPaddingRight(), viewGroup.getPaddingBottom());
        }
    }
}
