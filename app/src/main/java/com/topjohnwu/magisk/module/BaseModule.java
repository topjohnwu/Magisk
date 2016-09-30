package com.topjohnwu.magisk.module;


import java.util.List;

public abstract class BaseModule {

    protected String mId, mName, mVersion, mAuthor, mDescription, mSupportUrl, mDonateUrl;
    protected boolean mIsCacheModule = false;
    protected int mVersionCode = 0;

    protected void parseProps(List<String> props) { parseProps(props.toArray(new String[props.size()])); }

    protected void parseProps(String[] props) {
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
                    this.mVersionCode = Integer.parseInt(prop[1]);
                    break;
                case "author":
                    this.mAuthor = prop[1];
                    break;
                case "description":
                    this.mDescription = prop[1];
                    break;
                case "support":
                    this.mSupportUrl = prop[1];
                    break;
                case "donate":
                    this.mDonateUrl = prop[1];
                    break;
                case "cacheModule":
                    this.mIsCacheModule = Boolean.parseBoolean(prop[1]);
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

    public String getId() {return mId; }

    public String getDescription() {
        return mDescription;
    }

    public boolean isCache() {
        return mIsCacheModule;
    }

    public void setCache() {
        mIsCacheModule = true;
    }

    public int getVersionCode() {
        return mVersionCode;
    }

    public String getDonateUrl() {
        return mDonateUrl;
    }

    public String getSupportUrl() {
        return mSupportUrl;
    }
}
