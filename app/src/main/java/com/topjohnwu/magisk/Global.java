package com.topjohnwu.magisk;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.CallbackHandler;
import com.topjohnwu.magisk.utils.ValueSortedMap;

import java.util.List;

public class Global {
    public static class Constant {
        // No global constants now
    }
    public static class Info {
        public static double magiskVersion;
        public static double remoteMagiskVersion = -1;
        public static String magiskVersionString = "(none)";
        public static String magiskLink;
        public static String releaseNoteLink;
        public static int SNCheckResult = -1;
        public static String bootBlock = null;
    }
    public static class Data {
        public static ValueSortedMap<String, Repo> repoMap = new ValueSortedMap<>();
        public static ValueSortedMap<String, com.topjohnwu.magisk.module.Module> moduleMap = new ValueSortedMap<>();
        public static List<String> blockList;
    }
    public static class Events {
        public static final CallbackHandler.Event blockDetectionDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event packageLoadDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event reloadMainActivity = new CallbackHandler.Event();
        public static final CallbackHandler.Event moduleLoadDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event repoLoadDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event updateCheckDone = new CallbackHandler.Event();
        public static final CallbackHandler.Event safetyNetDone = new CallbackHandler.Event();
    }

}
