# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in /Users/topjohnwu/Library/Android/sdk/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Kotlin
-assumenosideeffects class kotlin.jvm.internal.Intrinsics {
	public static void check*(...);
	public static void throw*(...);
}

# Snet
-keepclassmembers class com.topjohnwu.magisk.ui.safetynet.SafetyNetHelper { *; }
-keep,allowobfuscation interface com.topjohnwu.magisk.ui.safetynet.SafetyNetHelper$Callback
-keepclassmembers class * implements com.topjohnwu.magisk.ui.safetynet.SafetyNetHelper$Callback {
  void onResponse(org.json.JSONObject);
}

# Stub
-keep class com.topjohnwu.magisk.core.App { <init>(java.lang.Object); }

# Strip Timber verbose and debug logging
-assumenosideeffects class timber.log.Timber$Tree {
  public void v(**);
  public void d(**);
}

# Excessive obfuscation
-repackageclasses 'a'
-allowaccessmodification

# QOL
-dontnote **
-dontwarn com.caverock.androidsvg.**
-dontwarn ru.noties.markwon.**
