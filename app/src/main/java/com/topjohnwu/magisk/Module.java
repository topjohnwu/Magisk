package com.topjohnwu.magisk;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

public class Module {

    private final boolean isValid;
    private final boolean isCache;
    private File mPropFile;

    private String mName;
    private String mVersion;
    private int mVersionCode;
    private String mDescription;

    public Module(File file) {
        this.isCache = file.getPath().contains("cache");
        this.isValid = new File(file + "/module.prop").exists();

        if (isValid) {
            mPropFile = new File(file + "/module.prop");
        }
    }

    public boolean isValid() {
        return isValid && mPropFile != null;
    }

    public boolean isCache() {
        return isCache;
    }

    public String getName() {
        return mName;
    }

    public int getVersionCode() {
        return mVersionCode;
    }

    public String getVersion() {
        return mVersion;
    }

    public String getDescription() {
        return mDescription;
    }

    public void parse() throws Exception {
        BufferedReader reader = new BufferedReader(new FileReader(mPropFile));
        String line;
        while ((line = reader.readLine()) != null) {
            String[] parts = line.split("=", 2);
            if (parts.length != 2) {
                continue;
            }

            String key = parts[0].trim();
            if (key.charAt(0) == '#') {
                continue;
            }

            String value = parts[1].trim();
            switch (key) {
                case "name":
                    mName = value;
                    break;
                case "version":
                    mVersion = value;
                    break;
                case "versionCode":
                    mVersionCode = Integer.parseInt(value);
                    break;
                case "description":
                    mDescription = value;
                    break;
            }
        }
        reader.close();
    }
}