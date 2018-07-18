package com.topjohnwu.magisk.asyncs;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.container.ValueSortedMap;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.superuser.Shell;

import java.util.List;

public class LoadModules extends ParallelTask<Void, Void, Void> {

    private List<String> getModList() {
        String command = "ls -d " + Const.MAGISK_PATH + "/* | grep -v lost+found";
        return Shell.Sync.su(command);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        MagiskManager mm = MagiskManager.get();
        mm.moduleMap = new ValueSortedMap<>();

        for (String path : getModList()) {
            Module module = new Module(path);
            mm.moduleMap.put(module.getId(), module);
        }

        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager.get().moduleLoadDone.publish();
        super.onPostExecute(v);
    }
}
