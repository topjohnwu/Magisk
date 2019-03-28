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
imgtool -> magisk
```

Note: The Magisk zip you download only contains `magiskboot` and `magiskinit`. The binary `magisk` is compressed and embedded into `magiskinit`. Push `magiskinit` to your device and run `./magiskinit -x magisk <path>` to extract `magisk` out of the binary.

### magiskboot
A tool to unpack / repack boot images, parse / patch / extract cpio, patch dtb, hex patch binaries, and compress / decompress files with multiple algorithms.

`magiskboot` natively supports (which means it does not rely on external tools) common compression formats including `gzip`, `lz4`, `lz4_legacy` ([only used on LG](https://events.static.linuxfound.org/sites/events/files/lcjpcojp13_klee.pdf)), `lzma`, `xz`, and `bzip2`.

The concept of `magiskboot` is to make boot image modification much simpler. For unpacking, it parses the header and all sections in the image (kernel, ramdisk, second, dtb, extra), detect compression format used in each section, and decompress while extraction. Each extracted sections are raw data, ready for direct modification. For repacking, the original boot image is required so the original headers can be used, changing only the necessary entries such as section sizes and checksum, and finally compress each sections back with the original format.

The tool also supports a variaty of CPIO operations that can modify CPIO archives without any extracting and repacking involved.

```
Usage: ./magiskboot <action> [args...]

Supported actions:
  --unpack <bootimg>
    Unpack <bootimg> to kernel, ramdisk.cpio, and if available, second, dtb,
    and extra into the current directory. Return values:
    0:valid    1:error    2:chromeos    3:ELF32    4:ELF64

  --repack <origbootimg> [outbootimg]
    Repack kernel, ramdisk.cpio[.ext], second, dtb... from current directory
    to [outbootimg], or new-boot.img if not specified.
    It will compress ramdisk.cpio with the same method used in <origbootimg>,
    or attempt to find ramdisk.cpio.[ext], and repack directly with the
    compressed ramdisk file

  --hexpatch <file> <hexpattern1> <hexpattern2>
    Search <hexpattern1> in <file>, and replace with <hexpattern2>

  --cpio <incpio> [commands...]
    Do cpio commands to <incpio> (modifications are done directly)
    Each command is a single argument, use quotes if necessary
    Supported commands:
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
      patch KEEPVERITY KEEPFORCEENCRYPT
        Ramdisk patches. KEEP**** are boolean values
      backup ORIG [SHA1]
        Create ramdisk backups from ORIG
        SHA1 of stock boot image is optional
      restore
        Restore ramdisk from ramdisk backup stored within incpio
      magisk ORIG KEEPVERITY KEEPFORCEENCRYPT [SHA1]
        Do Magisk patches and backups all in one step
        Create ramdisk backups from ORIG
        KEEP**** are boolean values
        SHA1 of stock boot image is optional
      sha1
        Print stock boot SHA1 if previously stored

  --dtb-<cmd> <dtb>
    Do dtb related cmds to <dtb> (modifications are done directly)
    Supported commands:
      dump
        Dump all contents from dtb for debugging
      test
        Check if fstab has verity/avb flags
        Return values:
        0:no flags    1:flag exists
      patch
        Search for fstab and remove verity/avb

  --compress[=method] <infile> [outfile]
    Compress <infile> with [method] (default: gzip), optionally to [outfile]
    <infile>/[outfile] can be '-' to be STDIN/STDOUT
    Supported methods: gzip xz lzma bzip2 lz4 lz4_legacy 

  --decompress <infile> [outfile]
    Detect method and decompress <infile>, optionally to [outfile]
    <infile>/[outfile] can be '-' to be STDIN/STDOUT
    Supported methods: gzip xz lzma bzip2 lz4 lz4_legacy 

  --sha1 <file>
    Print the SHA1 checksum for <file>

  --cleanup
    Cleanup the current working directory
```

### magiskinit
This binary will replace `init` in the ramdisk of a Magisk patched boot image. It is required for supporting devices using system as root (most A/B devices, plus some odd-balls like Huawei EMUI 9), but the tool is extended to support all traditional devices so the same installation setup could be used on all devices. More details can be found in the **Pre-Init** section in [Magisk Booting Process](details.md#magisk-booting-process).

### magiskpolicy
(This tool is aliased to `supolicy` for compatibility with SuperSU's sepolicy tool)

An applet of `magiskinit`. This tool could be used for advanced developers to modify SELinux policies. In common scenarios like Linux server admins, they would directly modify the SELinux policy sources (`*.te`) and recompile the `sepolicy` binary, but here on Android we directly patch the binary file (or runtime policies).

All processes spawned from the Magisk daemon, including root shells and all its forks, are running in the context `u:r:magisk:s0`. The rule used on all Magisk installed systems can be viewed as stock `sepolicy` with these patches: `magiskpolicy --magisk 'allow magisk * * *'`.

```
Usage: magiskpolicy [--options...] [policy statements...]

Options:
   --live            directly apply sepolicy live
   --magisk          inject built-in rules for a minimal
                     Magisk selinux environment
   --load FILE       load policies from FILE
   --compile-split   compile and load split cil policies
                     from system and vendor just like init
   --save FILE       save policies to FILE

If neither --load or --compile-split is specified, it will load
from current live policies (/sys/fs/selinux/policy)

One policy statement should be treated as one parameter;
this means a full policy statement should be enclosed in quotes;
multiple policy statements can be provided in a single command

The statements has a format of "<action> [args...]"
Use '*' in args to represent every possible match.
Collections wrapped in curly brackets can also be used as args.

Supported policy statements:

Type 1:
"<action> source-class target-class permission-class permission"
Action: allow, deny, auditallow, auditdeny

Type 2:
"<action> source-class target-class permission-class ioctl range"
Action: allowxperm, auditallowxperm, dontauditxperm

Type 3:
"<action> class"
Action: create, permissive, enforcing

Type 4:
"attradd class attribute"

Type 5:
"typetrans source-class target-class permission-class default-class (optional: object-name)"

Notes:
- typetrans does not support the all match '*' syntax
- permission-class cannot be collections
- source-class and target-class can also be attributes

Example: allow { source1 source2 } { target1 target2 } permission-class *
Will be expanded to:

allow source1 target1 permission-class { all-permissions }
allow source1 target2 permission-class { all-permissions }
allow source2 target1 permission-class { all-permissions }
allow source2 target2 permission-class { all-permissions }
```


### magisk
When the magisk binary is called with the name `magisk`, it works as an utility tool with many helper functions and the entry points for `init` to start Magisk services.

```
Usage: magisk [applet [arguments]...]
   or: magisk [options]...

Options:
   -c                        print current binary version
   -v                        print running daemon version
   -V                        print running daemon version code
   --list                    list all available applets
   --install [SOURCE] DIR    symlink all applets to DIR. SOURCE is optional
   --daemon                  manually start magisk daemon
   --[init trigger]          start service for init trigger
   --unlock-blocks           set BLKROSET flag to OFF for all block devices
   --restorecon              fix selinux context on Magisk files and folders
   --clone-attr SRC DEST     clone permission, owner, and selinux context

Supported init triggers:
   startup, post-fs-data, service

Supported applets:
    magisk, su, resetprop, magiskhide, imgtool
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
Usage: magiskhide [--options [arguments...] ]

Options:
  --enable          Start magiskhide
  --disable         Stop magiskhide
  --add PROCESS     Add PROCESS to the hide list
  --rm PROCESS      Remove PROCESS from the hide list
  --ls              Print out the current hide list
```

### imgtool
An applet of `magisk`, a collection of common commands used to create and manage `ext4` images.

```
Usage: imgtool <action> [args...]

Actions:
   create IMG SIZE      create ext4 image. SIZE is interpreted in MB
   resize IMG SIZE      resize ext4 image. SIZE is interpreted in MB
   mount  IMG PATH      mount IMG to PATH and prints the loop device
   umount PATH LOOP     unmount PATH and delete LOOP device
   merge  SRC TGT       merge SRC to TGT
   trim   IMG           trim IMG to save space
```
