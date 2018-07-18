package com.topjohnwu.magisk.container;


import android.content.ContentValues;
import android.database.Cursor;
import android.support.annotation.NonNull;

import java.util.List;

public abstract class BaseModule implements Comparable<BaseModule> {

    private String mId = null, mName, mVersion, mAuthor, mDescription;
    private int mVersionCode = -1, minMagiskVersion = -1;

    protected BaseModule() {}

    protected BaseModule(Cursor c) {
        mId = c.getString(c.getColumnIndex("id"));
        mName = c.getString(c.getColumnIndex("name"));
        mVersion = c.getString(c.getColumnIndex("version"));
        mVersionCode = c.getInt(c.getColumnIndex("versionCode"));
        mAuthor = c.getString(c.getColumnIndex("author"));
        mDescription = c.getString(c.getColumnIndex("description"));
        minMagiskVersion = c.getInt(c.getColumnIndex("minMagisk"));
    }

    public ContentValues getContentValues() {
        ContentValues values = new ContentValues();
        values.put("id", mId);
        values.put("name", mName);
        values.put("version", mVersion);
        values.put("versionCode", mVersionCode);
        values.put("author", mAuthor);
        values.put("description", mDescription);
        values.put("minMagisk", minMagiskVersion);
        return values;
    }

    protected void parseProps(List<String> props) { parseProps(props.toArray(new String[0])); }

    protected void parseProps(String[] props) throws NumberFormatException {
        for (String line : props) {
            String[] prop = line.split("=", 2);
            if (prop.length != 2)
                continue;

            String key = prop[0].trim();
            String value = prop[1].trim();
            if (key.isEmpty() || key.charAt(0) == '#')
                continue;

            switch (key) {
                case "id":
                    mId = value;
                    break;
                case "name":
                    mName = value;
                    break;
                case "version":
                    mVersion = value;
                    break;
                case "versionCode":
                    mVersionCode = Integer.parseInt(value);
                    break;
                case "author":
                    mAuthor = value;
                    break;
                case "description":
                    mDescription = value;
                    break;
                case "minMagisk":
                case "template":
                    minMagiskVersion = Integer.parseInt(value);
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

    public int getMinMagiskVersion() {
        return minMagiskVersion;
    }

    @Override
    public int compareTo(@NonNull BaseModule module) {
        return this.getName().toLowerCase().compareTo(module.getName().toLowerCase());
    }
}
