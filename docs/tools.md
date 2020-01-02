# Magisk Tools
Magisk comes with a huge collections of tools for installation, daemons, and utilities for developers. This documentation covers the 3 binaries and all included applets. The binaries and applets are shown below:

```
magiskboot                 /* binary */
magiskinit                 /* binary */
magiskpolicy -> magiskinit
supolicy -> magiskinit
magisk                     /* binary */
magiskhide -> magisk
resetprop -> magisk
su -> magisk
```

Note: The Magisk zip you download only contains `magiskboot`, `magiskinit`, and `magiskinit64`. The binary `magisk` is compressed and embedded into `magiskinit(64)`. Push `magiskinit(64)` to your device and run `./magiskinit(64) -x magisk <path>` to extract `magisk` out of the binary.

### magiskboot
A tool to unpack / repack boot images, parse / patch / extract cpio, patch dtb, hex patch binaries, and compress / decompress files with multiple algorithms.

`magiskboot` natively supports (which means it does not rely on external tools) common compression formats including `gzip`, `lz4`, `lz4_legacy` ([only used on LG](https://events.static.linuxfound.org/sites/events/files/lcjpcojp13_klee.pdf)), `lzma`, `xz`, and `bzip2`.

The concept of `magiskboot` is to make boot image modification simpler. For unpacking, it parses the header and extracts all sections in the image, decompressing on-the-fly if compression is detected in any sections. For repacking, the original boot image is required so the original headers can be used, changing only the necessary entries such as section sizes and checksum. All sections will be compressed back to the original format if required. The tool also supports many CPIO and DTB operations.

```
Usage: magiskboot <action> [args...]

Supported actions:
  unpack [-n] [-h] <bootimg>
    Unpack <bootimg> to, if available, kernel, kernel_dtb, ramdisk.cpio,
    second, dtb, extra, and recovery_dtbo into current directory.
    If '-n' is provided, it will not attempt to decompress kernel or
    ramdisk.cpio from their original formats.
    If '-h' is provided, it will dump header info to 'header',
    which will be parsed when repacking.
    Return values:
    0:valid    1:error    2:chromeos

  repack [-n] <origbootimg> [outbootimg]
    Repack boot image components from current directory
    to [outbootimg], or new-boot.img if not specified.
    If '-n' is provided, it will not attempt to recompress ramdisk.cpio,
    otherwise it will compress ramdisk.cpio and kernel with the same method
    in <origbootimg> if the file provided is not already compressed.

  hexpatch <file> <hexpattern1> <hexpattern2>
    Search <hexpattern1> in <file>, and replace with <hexpattern2>

  cpio <incpio> [commands...]
    Do cpio commands to <incpio> (modifications are done directly)
    Each command is a single argument, add quotes for each command
    Supported commands:
      exists ENTRY
        Return 0 if ENTRY exists, else return 1
      rm [-r] ENTRY
        Remove ENTRY, specify [-r] to remove recursively
      mkdir MODE ENTRY
        Create directory ENTRY in permissions MODE
      ln TARGET ENTRY
        Create a symlink to TARGET with the name ENTRY
      mv SOURCE DEST
        Move SOURCE to DEST
      add MODE ENTRY INFILE
        Add INFILE as ENTRY in permissions MODE; replaces ENTRY if exists
      extract [ENTRY OUT]
        Extract ENTRY to OUT, or extract all entries to current directory
      test
        Test the current cpio's patch status
        Return values:
        0:stock    1:Magisk    2:unsupported (phh, SuperSU, Xposed)
      patch
        Apply ramdisk patches. Configure settings with env variables:
        KEEPVERITY KEEPFORCEENCRYPT
      backup ORIG
        Create ramdisk backups from ORIG
      restore
        Restore ramdisk from ramdisk backup stored within incpio
      sha1
        Print stock boot SHA1 if previously backed up in ramdisk

  dtb <input> <action> [args...]
    Do dtb related actions to <input>
    Supported actions:
      print [-f]
        Print all contents of dtb for debugging
        Specify [-f] to only print fstab nodes
      patch [OUT]
        Search for fstab and remove verity/avb
        If [OUT] is not specified, it will directly output to <input>
        Configure with env variables: KEEPVERITY TWOSTAGEINIT

  split <input>
    Split image.*-dtb into kernel + kernel_dtb

  sha1 <file>
    Print the SHA1 checksum for <file>

  cleanup
    Cleanup the current working directory

  compress[=method] <infile> [outfile]
    Compress <infile> with [method] (default: gzip), optionally to [outfile]
    <infile>/[outfile] can be '-' to be STDIN/STDOUT
    Supported methods: bzip2 gzip lz4 lz4_legacy lzma xz

  decompress <infile> [outfile]
    Detect method and decompress <infile>, optionally to [outfile]
    <infile>/[outfile] can be '-' to be STDIN/STDOUT
    Supported methods: bzip2 gzip lz4 lz4_legacy lzma xz
```

### magiskinit
This binary will replace `init` in the ramdisk of a Magisk patched boot image. It is originally created for supporting devices using system-as-root, but the tool is extended to support all devices and became a crucial part of Magisk. More details can be found in the **Pre-Init** section in [Magisk Booting Process](details.md#magisk-booting-process).

### magiskpolicy
(This tool is aliased to `supolicy` for compatibility with SuperSU's sepolicy tool)

An applet of `magiskinit`. This tool could be used for advanced developers to modify SELinux policies. In common scenarios like Linux server admins, they would directly modify the SELinux policy sources (`*.te`) and recompile the `sepolicy` binary, but here on Android we directly patch the binary file (or runtime policies).

All processes spawned from the Magisk daemon, including root shells and all its forks, are running in the context `u:r:magisk:s0`. The rule used on all Magisk installed systems can be viewed as stock `sepolicy` with these patches: `magiskpolicy --magisk 'allow magisk * * *'`.

```
Usage: magiskpolicy [--options...] [policy statements...]

Options:
   --help            show help message for policy statements
   --load FILE       load policies from FILE
   --load-split      load from precompiled sepolicy or compile
                     split policies
   --compile-split   compile split cil policies
   --save FILE       save policies to FILE
   --live            directly apply sepolicy live
   --magisk          inject built-in rules for a minimal
                     Magisk selinux environment
   --apply FILE      apply rules from FILE, read and parsed
                     line by line as policy statements

If neither --load or --compile-split is specified, it will load
from current live policies (/sys/fs/selinux/policy)

One policy statement should be treated as one parameter;
this means a full policy statement should be enclosed in quotes.
Multiple policy statements can be provided in a single command.

The statements has a format of "<rule_name> [args...]"
Multiple types and permissions can be grouped into collections
wrapped in curly brackets.
'*' represents a collection containing all valid matches.

Supported policy statements:

Type 1:
"<rule_name> source_type target_type class perm_set"
Rules: allow, deny, auditallow, dontaudit

Type 2:
"<rule_name> source_type target_type class operation xperm_set"
Rules: allowxperm, auditallowxperm, dontauditxperm
* The only supported operation is ioctl
* The only supported xperm_set format is range ([low-high])

Type 3:
"<rule_name> class"
Rules: create, permissive, enforcing

Type 4:
"attradd class attribute"

Type 5:
"<rule_name> source_type target_type class default_type"
Rules: type_transition, type_change, type_member

Type 6:
"name_transition source_type target_type class default_type object_name"

Notes:
* Type 4 - 6 does not support collections
* Object classes cannot be collections
* source_type and target_type can also be attributes

Example: allow { s1 s2 } { t1 t2 } class *
Will be expanded to:

allow s1 t1 class { all-permissions }
allow s1 t2 class { all-permissions }
allow s2 t1 class { all-permissions }
allow s2 t2 class { all-permissions }
```


### magisk
When the magisk binary is called with the name `magisk`, it works as an utility tool with many helper functions and the entry points for several Magisk services.

```
Usage: magisk [applet [arguments]...]
   or: magisk [options]...

Options:
   -c                        print current binary version
   -v                        print running daemon version
   -V                        print running daemon version code
   --list                    list all available applets
   --daemon                  manually start magisk daemon
   --remove-modules          remove all modules and reboot
   --[init trigger]          start service for init trigger

Advanced Options (Internal APIs):
   --unlock-blocks           set BLKROSET flag to OFF for all block devices
   --restorecon              restore selinux context on Magisk files
   --clone-attr SRC DEST     clone permission, owner, and selinux context
   --clone SRC DEST          clone SRC to DEST
   --sqlite SQL              exec SQL commands to Magisk database

Supported init triggers:
   post-fs-data, service, boot-complete

Supported applets:
    su, resetprop, magiskhide
```

### su
An applet of `magisk`, the MagiskSU entry point. Good old `su` command.

```
Usage: su [options] [-] [user [argument...]]

Options:
  -c, --command COMMAND         pass COMMAND to the invoked shell
  -h, --help                    display this help message and exit
  -, -l, --login                pretend the shell to be a login shell
  -m, -p,
  --preserve-environment        preserve the entire environment
  -s, --shell SHELL             use SHELL instead of the default /system/bin/sh
  -v, --version                 display version number and exit
  -V                            display version code and exit
  -mm, -M,
  --mount-master                force run in the global mount namespace
```

Note: even though the `-Z, --context` option is not listed above, the option still exists for CLI compatibility with apps designed for SuperSU. However the option is silently ignored since it's no longer relevant.

### resetprop
An applet of `magisk`. An advanced system property manipulation utility. Check the [Resetprop Details](details.md#resetprop) for more background information.

```
Usage: resetprop [flags] [options...]

Options:
   -h, --help        show this message
   (no arguments)    print all properties
   NAME              get property
   NAME VALUE        set property entry NAME with VALUE
   --file FILE       load props from FILE
   --delete NAME     delete property

Flags:
   -v      print verbose output to stderr
   -n      set properties without init triggers
           only affects setprop
   -p      access actual persist storage
           only affects getprop and deleteprop
```

### magiskhide
An applet of `magisk`, the CLI to control MagiskHide. Use this tool to communicate with the daemon to change MagiskHide settings.

```
Usage: magiskhide [action [arguments...] ]

Actions:
   status          Return the status of magiskhide
   enable          Start magiskhide
   disable         Stop magiskhide
   add PKG [PROC]  Add a new target to the hide list
   rm PKG [PROC]   Remove target(s) from the hide list
   ls              Print the current hide list
   exec CMDs...    Execute commands in isolated mount
                   namespace and do all hide unmounts
```
