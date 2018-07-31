package com.topjohnwu.magisk.utils;

import android.content.res.Configuration;
import android.content.res.Resources;
import android.support.annotation.StringRes;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.ParallelTask;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;

public class LocaleManager {
    public static Locale locale;
    public static Locale defaultLocale;
    public static List<Locale> locales;

    public static void setLocale() {
        MagiskManager mm = Data.MM();
        Data.localeConfig = mm.prefs.getString(Const.Key.LOCALE, "");
        if (Data.localeConfig.isEmpty()) {
            locale = defaultLocale;
        } else {
            locale = Locale.forLanguageTag(Data.localeConfig);
        }
        Resources res = mm.getResources();
        Configuration config = new Configuration(res.getConfiguration());
        config.setLocale(locale);
        res.updateConfiguration(config, res.getDisplayMetrics());
    }

    public static String getString(Locale locale, @StringRes int id) {
        Configuration config = Data.MM().getResources().getConfiguration();
        config.setLocale(locale);
        return getString(config, id);
    }

    private static String getString(Configuration config, @StringRes int id) {
        return Data.MM().createConfigurationContext(config).getString(id);
    }

    private static List<Locale> getAvailableLocale() {
        List<Locale> locales = new ArrayList<>();
        HashSet<String> set = new HashSet<>();
        MagiskManager mm = Data.MM();
        Locale locale;

        @StringRes int compareId = R.string.download_file_error;

        // Add default locale
        locales.add(Locale.ENGLISH);
        set.add(getString(Locale.ENGLISH, compareId));

        // Add some special locales
        locales.add(Locale.TAIWAN);
        set.add(getString(Locale.TAIWAN, compareId));
        locale = new Locale("pt", "BR");
        locales.add(locale);
        set.add(getString(locale, compareId));

        // Other locales
        Configuration config = mm.getResources().getConfiguration();
        for (String s : mm.getAssets().getLocales()) {
            locale = Locale.forLanguageTag(s);
            config.setLocale(locale);
            if (set.add(getString(config, compareId))) {
                locales.add(locale);
            }
        }

        Collections.sort(locales, (a, b) -> a.getDisplayName(a).compareTo(b.getDisplayName(b)));

        return locales;
    }

    public static class LoadLocale extends ParallelTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            locales = getAvailableLocale();
            return null;
        }
        @Override
        protected void onPostExecute(Void aVoid) {
            Topic.publish(Topic.LOCAL_FETCH_DONE);
        }
    }
}
