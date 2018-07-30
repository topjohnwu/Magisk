package com.topjohnwu.magisk.asyncs;

import com.topjohnwu.magisk.Global;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.container.ValueSortedMap;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.superuser.io.SuFile;

public class LoadModules extends ParallelTask<Void, Void, Void> {

    private String[] getModList() {
        SuFile path = new SuFile(Const.MAGISK_PATH);
        return path.list((file, name) -> !name.equals("lost+found") && !name.equals(".core"));
    }

    @Override
    protected Void doInBackground(Void... voids) {
        MagiskManager mm = Global.MM();
        mm.moduleMap = new ValueSortedMap<>();

        for (String name : getModList()) {
            Module module = new Module(Const.MAGISK_PATH + "/" + name);
            mm.moduleMap.put(module.getId(), module);
        }

        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        Global.MM().moduleLoadDone.publish();
        super.onPostExecute(v);
    }
}
