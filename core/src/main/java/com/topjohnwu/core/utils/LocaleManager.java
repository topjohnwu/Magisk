package com.topjohnwu.core.utils;

import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.AsyncTask;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;

import androidx.annotation.StringRes;

public class LocaleManager {
    public static Locale locale = Locale.getDefault();
    public final static Locale defaultLocale = Locale.getDefault();
    public static List<Locale> locales;

    public static void setLocale(App app) {
        String localeConfig = app.prefs.getString(Const.Key.LOCALE, "");
        if (localeConfig.isEmpty()) {
            locale = defaultLocale;
        } else {
            locale = Locale.forLanguageTag(localeConfig);
        }
        Locale.setDefault(locale);
        Resources res = app.getResources();
        Configuration config = res.getConfiguration();
        config.setLocale(locale);
        res.updateConfiguration(config, res.getDisplayMetrics());
    }

    public static String getString(Locale locale, @StringRes int id) {
        Configuration config = new Configuration();
        config.setLocale(locale);
        return App.self.createConfigurationContext(config).getString(id);
    }

    public static void loadAvailableLocales(@StringRes int compareId) {
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            locales = new ArrayList<>();
            HashSet<String> set = new HashSet<>();
            Resources res = App.self.getResources();
            Locale locale;

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
            for (String s : res.getAssets().getLocales()) {
                locale = Locale.forLanguageTag(s);
                if (set.add(getString(locale, compareId))) {
                    locales.add(locale);
                }
            }

            Collections.sort(locales, (a, b) -> a.getDisplayName(a).compareTo(b.getDisplayName(b)));
            Topic.publish(Topic.LOCALE_FETCH_DONE);
        });
    }
}
