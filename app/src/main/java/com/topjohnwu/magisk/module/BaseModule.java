package com.topjohnwu.magisk.module;


import android.support.annotation.NonNull;

import com.topjohnwu.magisk.utils.Logger;

import java.util.List;

public abstract class BaseModule implements Comparable<BaseModule> {

    protected String mId, mName, mVersion, mAuthor, mDescription;
    protected int mVersionCode = 0;

    protected void parseProps(List<String> props) throws CacheModException { parseProps(props.toArray(new String[props.size()])); }

    protected void parseProps(String[] props) throws CacheModException {
        for (String line : props) {
            String[] prop = line.split("=", 2);
            if (prop.length != 2) {
                continue;
            }

            String key = prop[0].trim();
            if (key.charAt(0) == '#') {
                continue;
            }

            switch (key) {
                case "id":
                    this.mId = prop[1];
                    break;
                case "name":
                    this.mName = prop[1];
                    break;
                case "version":
                    this.mVersion = prop[1];
                    break;
                case "versionCode":
                    try {
                        this.mVersionCode = Integer.parseInt(prop[1]);
                    } catch (NumberFormatException ignored) {}
                    break;
                case "author":
                    this.mAuthor = prop[1];
                    break;
                case "description":
                    this.mDescription = prop[1];
                    break;
                case "cacheModule":
                    if (Boolean.parseBoolean(prop[1]))
                        throw new CacheModException(mId);
                    break;
                default:
                    break;
            }
        }
    }

    public String getName() {
        return mName;
    }

    public String getVersion() {
        return mVersion;
    }

    public String getAuthor() {
        return mAuthor;
    }

    public String getId() {
        return mId;
    }

    public String getDescription() {
        return mDescription;
    }

    public int getVersionCode() {
        return mVersionCode;
    }

    public static class CacheModException extends Exception {
        public CacheModException(String id) {
            Logger.error("Cache mods are no longer supported! id: " + id);
        }
    }

    @Override
    public int compareTo(@NonNull BaseModule o) {
        return this.getName().toLowerCase().compareTo(o.getName().toLowerCase());
    }
}
