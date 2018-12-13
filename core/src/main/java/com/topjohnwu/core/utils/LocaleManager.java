package com.topjohnwu.core.utils;

import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.superuser.Shell;

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

    public static Locale forLanguageTag(String tag) {
        if (Build.VERSION.SDK_INT >= 21) {
            return Locale.forLanguageTag(tag);
        } else {
            String[] tok = tag.split("-");
            if (tok.length == 0) {
                return new Locale("");
            }
            String language;
            switch (tok[0]) {
                case "und":
                    language = ""; // Undefined
                    break;
                case "fil":
                    language = "tl"; // Filipino
                    break;
                default:
                    language = tok[0];
            }
            if ((language.length() != 2 && language.length() != 3))
                return new Locale("");
            if (tok.length == 1)
                return new Locale(language);
            String country = tok[1];
            if (country.length() != 2 && country.length() != 3)
                return new Locale(language);
            return new Locale(language, country);
        }
    }

    public static void setLocale(App app) {
        String localeConfig = app.prefs.getString(Const.Key.LOCALE, "");
        if (localeConfig.isEmpty()) {
            locale = defaultLocale;
        } else {
            locale = forLanguageTag(localeConfig);
        }
        Locale.setDefault(locale);
        app.setResources(getLocaleContext(locale).getResources());
    }

    public static Context getLocaleContext(Locale locale) {
        Configuration config = new Configuration(App.self.getBaseContext().getResources().getConfiguration());
        config.setLocale(locale);
        return App.self.createConfigurationContext(config);
    }

    public static String getString(Locale locale, @StringRes int id) {
        return getLocaleContext(locale).getString(id);
    }

    public static void loadAvailableLocales(@StringRes int compareId) {
        Shell.EXECUTOR.execute(() -> {
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
                locale = forLanguageTag(s);
                if (set.add(getString(locale, compareId))) {
                    locales.add(locale);
                }
            }

            Collections.sort(locales, (a, b) -> a.getDisplayName(a).compareTo(b.getDisplayName(b)));
            Topic.publish(Topic.LOCALE_FETCH_DONE);
        });
    }
}
