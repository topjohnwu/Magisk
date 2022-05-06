package com.topjohnwu.magisk;

import android.app.Application;

import java.util.Map;

public class Telemetry {

    public static void start(Application app, String text, String fileName) {
    }

    public static void trackEvent(String name, Map<String, String> properties) {
    }

    public static void trackError(Throwable throwable, Map<String, String> properties) {
    }

    public static void trackError(Throwable throwable, String text) {
    }
}
