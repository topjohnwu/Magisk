package com.topjohnwu.magisk;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;

import java.util.ArrayList;
import java.util.List;

public class ProviderInstaller {
    private static final String GMS_SIGNATURE = "" +
            "308204433082032ba003020102020900c2e08746644a308d300d06092a864886" +
            "f70d01010405003074310b300906035504061302555331133011060355040813" +
            "0a43616c69666f726e6961311630140603550407130d4d6f756e7461696e2056" +
            "69657731143012060355040a130b476f6f676c6520496e632e3110300e060355" +
            "040b1307416e64726f69643110300e06035504031307416e64726f6964301e17" +
            "0d3038303832313233313333345a170d3336303130373233313333345a307431" +
            "0b3009060355040613025553311330110603550408130a43616c69666f726e69" +
            "61311630140603550407130d4d6f756e7461696e205669657731143012060355" +
            "040a130b476f6f676c6520496e632e3110300e060355040b1307416e64726f69" +
            "643110300e06035504031307416e64726f696430820120300d06092a864886f7" +
            "0d01010105000382010d00308201080282010100ab562e00d83ba208ae0a966f" +
            "124e29da11f2ab56d08f58e2cca91303e9b754d372f640a71b1dcb130967624e" +
            "4656a7776a92193db2e5bfb724a91e77188b0e6a47a43b33d9609b77183145cc" +
            "df7b2e586674c9e1565b1f4c6a5955bff251a63dabf9c55c27222252e875e4f8" +
            "154a645f897168c0b1bfc612eabf785769bb34aa7984dc7e2ea2764cae8307d8" +
            "c17154d7ee5f64a51a44a602c249054157dc02cd5f5c0e55fbef8519fbe327f0" +
            "b1511692c5a06f19d18385f5c4dbc2d6b93f68cc2979c70e18ab93866b3bd5db" +
            "8999552a0e3b4c99df58fb918bedc182ba35e003c1b4b10dd244a8ee24fffd33" +
            "3872ab5221985edab0fc0d0b145b6aa192858e79020103a381d93081d6301d06" +
            "03551d0e04160414c77d8cc2211756259a7fd382df6be398e4d786a53081a606" +
            "03551d2304819e30819b8014c77d8cc2211756259a7fd382df6be398e4d786a5" +
            "a178a4763074310b3009060355040613025553311330110603550408130a4361" +
            "6c69666f726e6961311630140603550407130d4d6f756e7461696e2056696577" +
            "31143012060355040a130b476f6f676c6520496e632e3110300e060355040b13" +
            "07416e64726f69643110300e06035504031307416e64726f6964820900c2e087" +
            "46644a308d300c0603551d13040530030101ff300d06092a864886f70d010104" +
            "050003820101006dd252ceef85302c360aaace939bcff2cca904bb5d7a1661f8" +
            "ae46b2994204d0ff4a68c7ed1a531ec4595a623ce60763b167297a7ae35712c4" +
            "07f208f0cb109429124d7b106219c084ca3eb3f9ad5fb871ef92269a8be28bf1" +
            "6d44c8d9a08e6cb2f005bb3fe2cb96447e868e731076ad45b33f6009ea19c161" +
            "e62641aa99271dfd5228c5c587875ddb7f452758d661f6cc0cccb7352e424cc4" +
            "365c523532f7325137593c4ae341f4db41edda0d0b1071a7c440f0fe9ea01cb6" +
            "27ca674369d084bd2fd911ff06cdbf2cfa10dc0f893ae35762919048c7efc64c" +
            "7144178342f70581c9de573af55b390dd7fdb9418631895d5f759f30112687ff" +
            "621410c069308a";
    private static final String MICROG_SIGNATURE = "" +
            "308202ed308201d5a003020102020426ffa009300d06092a864886f70d01010b" +
            "05003027310b300906035504061302444531183016060355040a130f4e4f4741" +
            "5050532050726f6a656374301e170d3132313030363132303533325a170d3337" +
            "303933303132303533325a3027310b3009060355040613024445311830160603" +
            "55040a130f4e4f47415050532050726f6a65637430820122300d06092a864886" +
            "f70d01010105000382010f003082010a02820101009a8d2a5336b0eaaad89ce4" +
            "47828c7753b157459b79e3215dc962ca48f58c2cd7650df67d2dd7bda0880c68" +
            "2791f32b35c504e43e77b43c3e4e541f86e35a8293a54fb46e6b16af54d3a4ed" +
            "a458f1a7c8bc1b7479861ca7043337180e40079d9cdccb7e051ada9b6c88c9ec" +
            "635541e2ebf0842521c3024c826f6fd6db6fd117c74e859d5af4db04448965ab" +
            "5469b71ce719939a06ef30580f50febf96c474a7d265bb63f86a822ff7b643de" +
            "6b76e966a18553c2858416cf3309dd24278374bdd82b4404ef6f7f122cec9385" +
            "9351fc6e5ea947e3ceb9d67374fe970e593e5cd05c905e1d24f5a5484f4aadef" +
            "766e498adf64f7cf04bddd602ae8137b6eea40722d0203010001a321301f301d" +
            "0603551d0e04160414110b7aa9ebc840b20399f69a431f4dba6ac42a64300d06" +
            "092a864886f70d01010b0500038201010007c32ad893349cf86952fb5a49cfdc" +
            "9b13f5e3c800aece77b2e7e0e9c83e34052f140f357ec7e6f4b432dc1ed54221" +
            "8a14835acd2df2deea7efd3fd5e8f1c34e1fb39ec6a427c6e6f4178b609b3690" +
            "40ac1f8844b789f3694dc640de06e44b247afed11637173f36f5886170fafd74" +
            "954049858c6096308fc93c1bc4dd5685fa7a1f982a422f2a3b36baa8c9500474" +
            "cf2af91c39cbec1bc898d10194d368aa5e91f1137ec115087c31962d8f76cd12" +
            "0d28c249cf76f4c70f5baa08c70a7234ce4123be080cee789477401965cfe537" +
            "b924ef36747e8caca62dfefdd1a6288dcb1c4fd2aaa6131a7ad254e9742022cf" +
            "d597d2ca5c660ce9e41ff537e5a4041e37";

    private static final String GMS_PACKAGE_NAME = "com.google.android.gms";
    private static final List<Signature> sWhitelistedSignatures = new ArrayList<>();

    static {
        sWhitelistedSignatures.add(new Signature(GMS_SIGNATURE));
        sWhitelistedSignatures.add(new Signature(MICROG_SIGNATURE));
    }

    private static boolean isSystemApp(PackageManager pm)
            throws PackageManager.NameNotFoundException {
        ApplicationInfo appInfo = pm.getApplicationInfo(GMS_PACKAGE_NAME, 0);
        return (appInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0;
    }

    private static boolean isSignatureWhitelisted(PackageManager pm)
            throws PackageManager.NameNotFoundException {
        PackageInfo packageInfo = pm.getPackageInfo(GMS_PACKAGE_NAME,
                PackageManager.GET_SIGNATURES);
        Signature[] signatures = packageInfo.signatures;
        for (Signature signature : signatures) {
            if (sWhitelistedSignatures.contains(signature)) {
                return true;
            }
        }
        return false;
    }

    public static boolean install(Context context) {
        try {
            // Check if GMS is a system app or has a trusted signature
            PackageManager pm = context.getPackageManager();
            if (!isSystemApp(pm) || !isSignatureWhitelisted(pm)) {
                return false;
            }

            // Try installing new SSL provider from Google Play Service
            Context gms = context.createPackageContext(GMS_PACKAGE_NAME,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            gms.getClassLoader()
                    .loadClass("com.google.android.gms.common.security.ProviderInstallerImpl")
                    .getMethod("insertProvider", Context.class)
                    .invoke(null, gms);
        } catch (Exception e) {
            return false;
        }
        return true;
    }
}
