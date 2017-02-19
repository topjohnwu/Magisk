package com.topjohnwu.magisk.module;


import android.database.Cursor;
import android.support.annotation.NonNull;

import com.topjohnwu.magisk.utils.Logger;

import java.util.List;

public abstract class BaseModule implements Comparable<BaseModule> {

    private String mId, mName, mVersion, mAuthor, mDescription;
    private int mVersionCode = 0;

    protected BaseModule() {}

    protected BaseModule(Cursor c) {
        mId = c.getString(c.getColumnIndex("id"));
        mName = c.getString(c.getColumnIndex("name"));
        mVersion = c.getString(c.getColumnIndex("version"));
        mVersionCode = c.getInt(c.getColumnIndex("versionCode"));
        mAuthor = c.getString(c.getColumnIndex("author"));
        mDescription = c.getString(c.getColumnIndex("description"));
    }

    protected void parseProps(List<String> props) throws CacheModException { parseProps(props.toArray(new String[props.size()])); }

    protected void parseProps(String[] props) throws CacheModException {
        for (String line : props) {
            String[] prop = line.split("=", 2);
            if (prop.length != 2)
                continue;

            String key = prop[0].trim();
            if (key.charAt(0) == '#')
                continue;

            switch (key) {
                case "id":
                    mId = prop[1];
                    break;
                case "name":
                    mName = prop[1];
                    break;
                case "version":
                    mVersion = prop[1];
                    break;
                case "versionCode":
                    try {
                        mVersionCode = Integer.parseInt(prop[1]);
                    } catch (NumberFormatException ignored) {}
                    break;
                case "author":
                    mAuthor = prop[1];
                    break;
                case "description":
                    mDescription = prop[1];
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

    public void setName(String name) {
        mName = name;
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

    public void setId(String id) {
        mId = id;
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
    public int compareTo(@NonNull BaseModule module) {
        return this.getName().toLowerCase().compareTo(module.getName().toLowerCase());
    }
}
