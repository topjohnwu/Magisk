package a;

import com.topjohnwu.magisk.utils.PatchAPK;
import com.topjohnwu.signing.BootSigner;

public class a {

    @Deprecated
    public static boolean patchAPK(String in, String out, String pkg) {
        return PatchAPK.patch(in, out, pkg);
    }

    public static boolean patchAPK(String in, String out, String pkg, String label) {
        return PatchAPK.patch(in, out, pkg, label);
    }

    public static void main(String[] args) throws Exception {
        BootSigner.main(args);
    }
}
