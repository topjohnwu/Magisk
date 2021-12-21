# Updating the latest framework manifest

## Adding new HALs / Major version update

Add a new `<hal>` entry without a `max-level` attribute. The `<hal>` entry can
be added to the main manifest under `manifest.xml`, or to the manifest
fragment for the server module specified in `vintf_fragments`.

Introducing new HALs are backwards compatible.

## Minor version update

When a framework HAL updates its minor version, simply update the `<version>` or
`<fqname>` field to the latest version. This is the same as any other HALs.

For example, when `IServiceManager` updates to 1.2, change its `<fqname>` field
to `@1.2::IServiceManager/default`.

Because minor version updates are backwards compatible, all devices that require
a lower minor version of the HAL are still compatible.

Leave `max-level` attribute empty.

## Deprecating HAL

When a framework HAL is deprecated, set `max-level` field of the HAL from empty
to the last frozen version.
For example, if IDisplayService is deprecated in Android S, set `max-level` to
Android R (5):

```xml
<manifest version="3.0" type="framework">
  <hal format="hidl" max-level="5"> <!-- Level::R -->
    <name>android.frameworks.displayservice</name>
    <transport>hwbinder</transport>
    <fqname>@1.0::IDisplayService/default</fqname>
  </hal>
</manifest>
```

Note that the `max-level` of the HAL is set to Android R, meaning that the HAL
is last available in Android R and disabled in Android S.

Deprecating a HAL doesn’t mean dropping support of the HAL, so no devices will
break.

When setting `max-level` of a HAL:
- If `optional="false"` in frozen DCMs, the build system checks that adding the
  attribute does not break backwards compatibility; that is,
  `max-level > last_frozen_level`.
- If `optional="true"`, the check is disabled. Care must be taken to ensure
  `max-level` is set appropriately.

## Removing HAL

When the framework drops support of a certain HAL, the corresponding HAL entry
is removed from the framework manifest, and code that serves and registers the
HAL must be removed simultaneously.

Devices that are lower than the `max-level` attribute of the HAL may start to
break if they require this HAL. Hence, this must only be done when there is
enough evidence that the devices are not updateable to the latest Android
release.

# Freezing framework HAL manifest

First, check `libvintf` or `hardware/interfaces/compatibility_matrices` to
determine the current level.

Execute the following, replacing the argument with the level to freeze:

```shell script
lunch cf_x86_phone-userdebug # or any generic target
LEVEL=5
./freeze.sh ${LEVEL}
```

A new file, `frozen/${LEVEL}.xml`, will be created after the command is
executed. Frozen system manifests are stored in compatibility matrices. Then,
manually inspect the frozen compatibility matrix. Modify the `optional`
field for certain HALs. See comments in the compatibility matrix of the previous
level for details.

These compatibility matrices served as a reference for devices at that
target FCM version. Devices at the given target FCM version should
reference DCMs in the `frozen/` dir, with some of the HALs marked
as `optional="true"` or even omitted if unused by device-specific code.

At build time, compatibiltiy is checked between framework manifest and
the respective frozen DCM. HALs in the framework manifest with `max-level`
less than the specified level are omitted.
