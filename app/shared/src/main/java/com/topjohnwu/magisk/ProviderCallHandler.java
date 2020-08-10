package com.topjohnwu.magisk;

import android.content.Context;
import android.os.Bundle;

public interface ProviderCallHandler {
    Bundle call(Context context, String method, String arg, Bundle extras);
}
