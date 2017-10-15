package com.topjohnwu.magisk.asyncs;

import android.content.Context;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.container.ValueSortedMap;
import com.topjohnwu.magisk.utils.Utils;

public class LoadModules extends ParallelTask<Void, Void, Void> {

    public LoadModules(Context context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        MagiskManager mm = getMagiskManager();
        if (mm == null) return null;
        mm.moduleMap = new ValueSortedMap<>();

        for (String path : Utils.getModList(MagiskManager.MAGISK_PATH)) {
            Module module = new Module(path);
            mm.moduleMap.put(module.getId(), module);
        }

        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager mm = getMagiskManager();
        if (mm == null) return;
        mm.moduleLoadDone.publish();
        super.onPostExecute(v);
    }
}
