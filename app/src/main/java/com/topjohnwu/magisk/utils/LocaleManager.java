package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.text.TextUtils;

import androidx.annotation.StringRes;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.internal.InternalUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;

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

    public static String toLanguageTag(Locale loc) {
        if (Build.VERSION.SDK_INT >= 21) {
            return loc.toLanguageTag();
        } else {
            String language = loc.getLanguage();
            String country = loc.getCountry();
            String variant = loc.getVariant();
            if (language.isEmpty() || !language.matches("\\p{Alpha}{2,8}")) {
                language = "und";       // Follow the Locale#toLanguageTag() implementation
            } else if (language.equals("iw")) {
                language = "he";        // correct deprecated "Hebrew"
            } else if (language.equals("in")) {
                language = "id";        // correct deprecated "Indonesian"
            } else if (language.equals("ji")) {
                language = "yi";        // correct deprecated "Yiddish"
            }
            // ensure valid country code, if not well formed, it's omitted
            if (!country.matches("\\p{Alpha}{2}|\\p{Digit}{3}")) {
                country = "";
            }

            // variant subtags that begin with a letter must be at least 5 characters long
            if (!variant.matches("\\p{Alnum}{5,8}|\\p{Digit}\\p{Alnum}{3}")) {
                variant = "";
            }
            StringBuilder tag = new StringBuilder(language);
            if (!country.isEmpty())
                tag.append('-').append(country);
            if (!variant.isEmpty())
                tag.append('-').append(variant);
            return tag.toString();
        }
    }

    public static void setLocale(ContextWrapper wrapper) {
        String localeConfig = Config.get(Config.Key.LOCALE);
        if (TextUtils.isEmpty(localeConfig)) {
            locale = defaultLocale;
        } else {
            locale = forLanguageTag(localeConfig);
        }
        Locale.setDefault(locale);
        InternalUtils.replaceBaseContext(wrapper, getLocaleContext(locale));
    }

    public static Context getLocaleContext(Context context, Locale locale) {
        Configuration config = new Configuration(context.getResources().getConfiguration());
        config.setLocale(locale);
        return context.createConfigurationContext(config);
    }

    public static Context getLocaleContext(Locale locale) {
        return getLocaleContext(App.self.getBaseContext(), locale);
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
            Event.trigger(Event.LOCALE_FETCH_DONE);
        });
    }
}
