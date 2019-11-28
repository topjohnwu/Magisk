## So every class is case insensitive to avoid some bizare problems
-dontusemixedcaseclassnames

## If reflection issues come up uncomment this, that should temporarily fix it
#-keep class kotlin.** { *; }
#-keep class kotlin.Metadata { *; }
#-keepclassmembers class kotlin.Metadata {
#    public <methods>;
#}

## Never warn about Kotlin, it should work as-is
-dontwarn kotlin.**

## Removes runtime null checks - doesn't really matter if it crashes on kotlin or java NPE
-assumenosideeffects class kotlin.jvm.internal.Intrinsics {
    static void checkParameterIsNotNull(java.lang.Object, java.lang.String);
}

## Useless option for dex
-dontpreverify