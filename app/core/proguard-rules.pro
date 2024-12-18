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
-assumenosideeffects public class kotlin.coroutines.jvm.internal.DebugMetadataKt {
   private static ** getDebugMetadataAnnotation(...) return null;
}

# Stub
-keep class com.topjohnwu.magisk.core.App { <init>(java.lang.Object); }
-keepclassmembers class androidx.appcompat.app.AppCompatDelegateImpl {
  boolean mActivityHandlesConfigFlagsChecked;
  int mActivityHandlesConfigFlags;
}

# Strip Timber verbose and debug logging
-assumenosideeffects class timber.log.Timber$Tree {
  public void v(**);
  public void d(**);
}

# With R8 full mode generic signatures are stripped for classes that are not
# kept. Suspend functions are wrapped in continuations where the type argument
# is used.
-keep,allowobfuscation,allowshrinking class kotlin.coroutines.Continuation

# Excessive obfuscation
-flattenpackagehierarchy
-allowaccessmodification

-dontwarn org.junit.**
