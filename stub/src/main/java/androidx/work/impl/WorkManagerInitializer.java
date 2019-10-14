package androidx.work.impl;

import com.topjohnwu.magisk.dummy.DummyProvider;

public class WorkManagerInitializer extends DummyProvider {
    /* This class have to exist, or else pre 9.0 devices
     * will experience ClassNotFoundException crashes when
     * launching the stub, as ContentProviders are constructed
     * when an app starts up. Pre 9.0 devices do not have
     * our custom DelegateComponentFactory to help creating
     * dummies. */
}
