package com.topjohnwu.magisk.model;

import com.topjohnwu.magisk.ui.utils.Utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

public class Module {

    private final boolean isValid;

    private File mRemoveFile;
    private File mDisableFile;

    private File mPropFile;

    private String mName;
    private String mVersion;
    private String mDescription;

    public Module(File file) {
        this.isValid = new File(file + "/module.prop").exists();

        if (!isValid) return;

        mPropFile = new File(file + "/module.prop");
        mRemoveFile = new File(file + "/remove");
        mDisableFile = new File(file + "/disable");
    }

    public boolean isValid() {
        return isValid && mPropFile != null;
    }

    public String getName() {
        return mName;
    }

    public String getVersion() {
        return mVersion;
    }

    public String getDescription() {
        return mDescription;
    }

    public void createDisableFile() {
        Utils.executeCommand("touch " + mDisableFile.getPath());
    }

    public boolean removeDisableFile() {
        return mDisableFile.delete();
    }

    public boolean isEnabled() {
        return mDisableFile.exists();
    }

    public void createRemoveFile() {
        Utils.executeCommand("touch " + mRemoveFile.getPath());
    }

    public boolean deleteRemoveFile() {
        return mRemoveFile.delete();
    }

    public boolean willBeRemoved() {
        return mRemoveFile.exists();
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
                case "description":
                    mDescription = value;
                    break;
            }
        }
        reader.close();
    }
}