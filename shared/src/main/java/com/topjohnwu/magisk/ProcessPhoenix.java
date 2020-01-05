/*
 * Copyright (C) 2014 Jake Wharton
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.topjohnwu.magisk;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import java.util.ArrayList;

import static android.content.Intent.FLAG_ACTIVITY_CLEAR_TASK;
import static android.content.Intent.FLAG_ACTIVITY_NEW_TASK;

/**
 * Modified from JakeWharton/ProcessPhoenix
 *
 * Process Phoenix facilitates restarting your application process. This should only be used for
 * things like fundamental state changes in your debug builds (e.g., changing from staging to
 * production).
 * <p>
 * Trigger process recreation by calling {@link #triggerRebirth} with a {@link Context} instance.
 */
public class ProcessPhoenix extends Activity {
    private static final String KEY_RESTART_INTENT = "phoenix_restart_intent";

    public static void triggerRebirth(Context context, Intent intent) {
        intent.addFlags(FLAG_ACTIVITY_NEW_TASK); // In case we are called with non-Activity context.
        intent.putExtra(KEY_RESTART_INTENT, getRestartIntent(context));
        context.startActivity(intent);
        if (context instanceof Activity) {
            ((Activity) context).finish();
        }
        Runtime.getRuntime().exit(0);
    }

    private static Intent getRestartIntent(Context context) {
        String packageName = context.getPackageName();
        Intent defaultIntent = context.getPackageManager().getLaunchIntentForPackage(packageName);
        if (defaultIntent != null) {
            defaultIntent.addFlags(FLAG_ACTIVITY_NEW_TASK | FLAG_ACTIVITY_CLEAR_TASK);
            return defaultIntent;
        }

        throw new IllegalStateException();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent().getParcelableExtra(KEY_RESTART_INTENT);
        startActivity(intent);
        finish();
        Runtime.getRuntime().exit(0);
    }
}
