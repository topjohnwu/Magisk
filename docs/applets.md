## Available Applets
Magisk has a core binary which acts as a multi-call program with many applets. Here is an introduction to all available applets.

### magisk
The magisk binary itself provides a lot of utility functions when called with the name `magisk`. They are used in both Magisk installation and module installation. The entry point for `init` to invoke magisk's boot procedures are also listed here.

Command help message:

```
Usage: magisk [applet [arguments]...]
   or: magisk --install [SOURCE] DIR
       if SOURCE not provided, will link itself
   or: magisk --list
   or: magisk --createimg IMG SIZE
       create ext4 image, SIZE is interpreted in MB
   or: magisk --imgsize IMG
   or: magisk --resizeimg IMG SIZE
       SIZE is interpreted in MB
   or: magisk --mountimg IMG PATH
       mount IMG to PATH and prints the loop device
   or: magisk --umountimg PATH LOOP
   or: magisk --[boot stage]
       start boot stage service
   or: magisk [options]
   or: applet [arguments]...

Supported boot stages:
   post-fs, post-fs-data, service

Options:
   -c          print client version
   -v          print daemon version
   -V          print daemon version code

Supported applets:
    su, resetprop, magiskpolicy, supolicy, sepolicy-inject, magiskhide
```

### su
The MagiskSU entrypoint. Call `su` to gain a root shell.

Command help message:

```
Usage: su [options] [--] [-] [LOGIN] [--] [args...]

Options:
  -c, --command COMMAND         pass COMMAND to the invoked shell
  -h, --help                    display this help message and exit
  -, -l, --login                pretend the shell to be a login shell
  -m, -p,
  --preserve-environment        do not change environment variables
  -s, --shell SHELL             use SHELL instead of the default /system/bin/sh
  -u                            display the multiuser mode and exit
  -v, --version                 display version number and exit
  -V                            display version code and exit,
                                this is used almost exclusively by Superuser.apk
  -mm, -M,
  --mount-master                run in the global mount namespace,
                                use if you need to publicly apply mounts
```

Note: even though the `-Z, --context` option is not listed above, it actually still exists. However MagiskSU will silently ignore the option since it's not needed anymore. It is still left over because some apps still request root shell with specific contexts as an option.

### resetprop
An advanced system prop manipulation utility; you can arbitrarily alter system props using this tool. Here's some background knowledge:

> System props are stored in a hybrid trie/binary tree data structure in memory; it was originally designed to only support adding nodes, and no nodes should be removed. Props can be read by many processes (e.g. via the `getprop` command); however, only the `init` process have write access to the property data. `init` provides a `property_service` to accept property update requests, so all property changes are monitored and controlled by `init` Restrictions such as **read-only** props (props that starts with `ro.`), which can only be set once and cannot be changed afterwards, is therefore implemented in `init`.

**resetprop** acts just like `init`: directly access the data structure, bypassing the whole `property_service` part. Doing so, we gain **arbitrary modification** power, including altering read-only props and deleting properties. Delete properties, which was stated *"forbidden"* in the data structure, is implemented through some tricks in the data structure.

One subtle thing to be aware of is that if we change props by directly modifying the data structure, `on property:foo=bar` triggers registered in `*.rc` scripts will not be triggered properly. This may be a good thing or a bad thing, depending on what behavior you expect. I made the default behavior to match the original setprop command, which **WILL** trigger events, but of course I provide a flag (`-n`) to disable it if you need this special behavior.

Command help message:

```
Usage: resetprop [options] [args...]

Options:
   -v      show verbose output
   -n      only modify property in memory

resetprop  NAME VALUE        set property entry NAME with VALUE
resetprop  --file FILE       load props from FILE
resetprop  --delete NAME     remove prop entry NAME
```

### magiskpolicy
(This tool is aliased to `supolicy` and `sepolicy-injection` for legacy reasons)

A tool to patch `sepolicy`. **magiskpolicy** also comes with built-in rules to unleash restrictions to make Magisk work properly. `sepolicy` is a compiled binary containing SELinux rules; we directly patch rules in the binary format since we don't have access to the SELinux policy source (`*.te`) files.

The Magisk daemon itself, the root shell, and all processes spawned from the daemon and root shell are all running in the context `u:r:su:s0`. This context is not only patched to be permissive, but also patched to allow any transition from `u:r:su:s0` to any domain. This was done because Samsung devices do not support permissive out of the box.

The built in patches are split to 3 parts: minimal, medium, and large. The full patch will result in a huge policy file, which might cause the `sepolicy` file unable to fit in `boot.img`.

- The minimal patch is just enough to start Magisk daemon and allow the daemon to further patch the policy during boot time (which is called **live patch**). It is done at installation and directly into `boot.img`. 
- The medium patch covers most common operations, and is live patched as soon as Magisk daemon is started (blocking boot process).
- The large patch contains the full patch. Due to the concern of greatly increasing the boot time, it is designed to run in the background until it's joined in the non-blocking late_start bootstage.

What this all means is that **only late_start service mode is guaranteed to run in a fully patched environment**. If any script is not time critical, it is **highly recommended to run those scripts in late_start service mode**.

Command help message:

```
Usage: magiskpolicy [--options...] [policystatements...]

Options:
  --live                directly load patched policy to device
  --minimal             minimal patches, used for boot image patches
  --load <infile>       load policies from <infile>
                        (load from live policies if not specified)
  --save <outfile>      save policies to <outfile>

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

### magiskhide
This is the CLI to control the state of MagiskHide.

Command help message: 

```
Usage: magiskhide [--options [arguments...] ]

Options:
  --enable          Start magiskhide
  --disable         Stop magiskhide
  --add PROCESS     Add PROCESS to the hide list
  --rm PROCESS      Remove PROCESS from the hide list
  --ls              Print out the current hide list
```
