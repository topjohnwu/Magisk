package a;

import com.topjohnwu.core.utils.BootSigner;
import com.topjohnwu.magisk.utils.PatchAPK;

import androidx.annotation.Keep;

@Keep
public class a extends BootSigner {
    public static boolean patchAPK(String in, String out, String pkg) {
        return PatchAPK.patch(in, out, pkg);
    }
}
