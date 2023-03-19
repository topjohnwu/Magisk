package androidx.lifecycle;

import android.content.Context;

import androidx.annotation.NonNull;

public class ProcessLifecycleAccessor {
    public static void init(@NonNull Context context) {
        LifecycleDispatcher.init(context);
        ProcessLifecycleOwner.init(context);
    }
}
