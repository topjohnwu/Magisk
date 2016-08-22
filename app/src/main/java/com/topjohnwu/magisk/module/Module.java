package com.topjohnwu.magisk.module;

import com.topjohnwu.magisk.utils.Utils;

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
        Utils.su("touch " + mDisableFile.getPath());
    }

    public void removeDisableFile() {
        Utils.su("rm -f " + mDisableFile.getPath());
    }

    public boolean isEnabled() {
        return ! mDisableFile.exists();
    }

    public void createRemoveFile() {
        Utils.su("touch " + mRemoveFile.getPath());
    }

    public void deleteRemoveFile() {
        Utils.su("rm -f " + mRemoveFile.getPath());
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