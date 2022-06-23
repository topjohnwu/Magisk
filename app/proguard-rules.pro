# Parcelable
-keepclassmembers class * implements android.os.Parcelable {
    public static final ** CREATOR;
}

# Kotlin
-assumenosideeffects class kotlin.jvm.internal.Intrinsics {
	public static void check*(...);
	public static void throw*(...);
}
-assumenosideeffects class java.util.Objects {
    public static ** requireNonNull(...);
}

# Stub
-keep class com.topjohnwu.magisk.core.App { <init>(java.lang.Object); }
-keepclassmembers class androidx.appcompat.app.AppCompatDelegateImpl {
  boolean mActivityHandlesUiModeChecked;
  boolean mActivityHandlesUiMode;
}

# main
-keep,allowoptimization public class com.topjohnwu.magisk.signing.SignBoot {
    public static void main(java.lang.String[]);
}

# Strip Timber verbose and debug logging
-assumenosideeffects class timber.log.Timber$Tree {
  public void v(**);
  public void d(**);
}

# Excessive obfuscation
-repackageclasses 'a'
-allowaccessmodification

-obfuscationdictionary ../dict.txt
-classobfuscationdictionary ../dict.txt
-packageobfuscationdictionary ../dict.txt

-dontwarn org.bouncycastle.jsse.BCSSLParameters
-dontwarn org.bouncycastle.jsse.BCSSLSocket
-dontwarn org.bouncycastle.jsse.provider.BouncyCastleJsseProvider
-dontwarn org.commonmark.ext.gfm.strikethrough.Strikethrough
-dontwarn org.conscrypt.Conscrypt*
-dontwarn org.conscrypt.ConscryptHostnameVerifier
-dontwarn org.openjsse.javax.net.ssl.SSLParameters
-dontwarn org.openjsse.javax.net.ssl.SSLSocket
-dontwarn org.openjsse.net.ssl.OpenJSSE
