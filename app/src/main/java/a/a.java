package a;

import androidx.annotation.Keep;
import androidx.core.app.AppComponentFactory;

import com.topjohnwu.magisk.utils.PatchAPK;
import com.topjohnwu.signing.BootSigner;

@Keep
public class a extends AppComponentFactory {

    public static boolean patchAPK(String in, String out, String pkg) {
        return PatchAPK.patch(in, out, pkg);
    }

    public static void main(String[] args) throws Exception {
        BootSigner.main(args);
    }
}
