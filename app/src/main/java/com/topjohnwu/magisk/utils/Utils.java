package com.topjohnwu.magisk.utils;

import java.util.List;

public class Utils {

    public static boolean fileExist(String path) {
        List<String> ret;
        ret = Shell.sh("if [ -f " + path + " ]; then echo true; else echo false; fi");
        if (!Boolean.parseBoolean(ret.get(0)) && Shell.rootAccess()) ret = Shell.su("if [ -f " + path + " ]; then echo true; else echo false; fi");
        return Boolean.parseBoolean(ret.get(0));
    }

    public static boolean createFile(String path) {
        if (!Shell.rootAccess()) {
            return false;
        } else {
            return Boolean.parseBoolean(Shell.su("touch " + path + " 2>/dev/null; if [ -f " + path + " ]; then echo true; else echo false; fi").get(0));
        }
    }

    public static boolean removeFile(String path) {
        if (!Shell.rootAccess()) {
            return false;
        } else {
            return Boolean.parseBoolean(Shell.su("rm -f " + path + " 2>/dev/null; if [ -f " + path + " ]; then echo false; else echo true; fi").get(0));
        }
    }

    public static List<String> getModList(String path) {
        List<String> ret;
        ret = Shell.sh("find " + path + " -type d -maxdepth 1 | while read ITEM ; do if [ -f $ITEM/module.prop ]; then echo $ITEM; fi; done");
        if (ret.isEmpty() && Shell.rootAccess()) ret = Shell.su("find " + path + " -type d -maxdepth 1 | while read ITEM ; do if [ -f $ITEM/module.prop ]; then echo $ITEM; fi; done");
        return ret;
    }

    public static List<String> readFile(String path) {
        List<String> ret;
        ret = Shell.sh("cat " + path);
        if (ret.isEmpty() && Shell.rootAccess()) ret = Shell.su("cat " + path, "echo \' \'");
        return ret;
    }



}
