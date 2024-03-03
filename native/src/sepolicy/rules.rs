use crate::{ffi::Xperm, sepolicy, SepolicyMagisk};
use base::{set_log_level_state, LogLevel};
use std::pin::Pin;

macro_rules! rules {
    (@args all) => {
        vec![]
    };
    (@args xall) => {
        vec![Xperm { low: 0x0000, high: 0xFFFF, reset: false }]
    };
    (@args [proc]) => {
        vec!["magisk"]
    };
    (@args [file]) => {
        vec!["magisk_file"]
    };
    (@args [log]) => {
        vec!["magisk_log_file"]
    };
    (@args proc) => {
        "magisk"
    };
    (@args file) => {
        "magisk_file"
    };
    (@args log) => {
        "magisk_log_file"
    };
    (@args [$($arg:tt)*]) => {
        vec![$($arg)*]
    };
    (@args $arg:expr) => {
        $arg
    };
    (@stmt $self:ident) => {};
    (@stmt $self:ident $action:ident($($args:tt),*); $($res:tt)*) => {
        $self.as_mut().$action($(rules!(@args $args)),*);
        rules!{@stmt $self $($res)* }
    };
    (use $self:ident; $($res:tt)*) => {{
        rules!{@stmt $self $($res)* }
    }};
}

impl SepolicyMagisk for sepolicy {
    fn magisk_rules(mut self: Pin<&mut Self>) {
        // Temp suppress warnings
        set_log_level_state(LogLevel::Warn, false);
        rules! {
            use self;
            allow(all, ["kernel"], ["security"], ["load_policy"]);
            type_(proc, ["domain"]);
            permissive([proc]);
            typeattribute([proc], ["mlstrustedsubject", "netdomain", "bluetoothdomain"]);
            type_(file, ["file_type"]);
            typeattribute([file], ["mlstrustedobject"]);
            type_(log, ["file_type"]);
            typeattribute([log], ["mlstrustedobject"]);

            // Make our root domain unconstrained
            allow([proc], all, all, all);

            // Allow us to do any ioctl
            allowxperm(["magisk"], all, ["blk_file", "fifo_file", "chr_file"], xall);

            // Create unconstrained file type
            allow(all, [file], ["file", "dir", "fifo_file", "chr_file", "lnk_file", "sock_file"], all);

            // Only allow zygote to open log pipe
            allow(["zygote"], [log], ["fifo_file"], ["open", "read"]);

            // Allow all processes to output logs

            allow(["domain"], [log], ["fifo_file"], ["write"]);

            // Allow these processes to access MagiskSU and output logs
            allow(["zygote", "shell", "system_app", "priv_app", "untrusted_app", "untrusted_app_all"],
                       [proc], ["unix_stream_socket"], ["connectto", "getopt"]);

            // Let everyone access tmpfs files (for SAR sbin overlay)
            allow(all, ["tmpfs"], ["file"], all);

            // Allow magiskinit daemon to handle mock selinuxfs
            allow(["kernel"], ["tmpfs"], ["fifo_file"], ["write"]);

            // For relabelling files
            allow(["rootfs"], ["labeledfs"], ["filesystem"], ["associate"]);
            allow([file], ["pipefs", "devpts"], ["filesystem"], ["associate"]);

            // Let init transit to SEPOL_PROC_DOMAIN
            allow(["kernel"], ["kernel"], ["process"], ["setcurrent"]);
            allow(["kernel"], [proc], ["process"], ["dyntransition"]);

            // Let init run stuffs
            allow(["kernel"], [proc], ["fd"], ["use"]);
            allow(["init"], [proc], ["process"], all);

            // suRights
            allow(["servicemanager"], [proc], ["dir"], ["search", "read"]);
            allow(["servicemanager"], [proc], ["file"], ["open", "read"]);
            allow(["servicemanager"], [proc], ["process"], ["getattr"]);
            allow(all, [proc], ["process"], ["sigchld"]);

            // allowLog
            allow(["logd"], [proc], ["dir"], ["search"]);
            allow(["logd"], [proc], ["file"], ["read", "open", "getattr"]);

            // dumpsys
            allow(all, [proc], ["fd"], ["use"]);
            allow(all, [proc], ["fifo_file"], ["write", "read", "open", "getattr"]);

            // bootctl
            allow(["hwservicemanager"], [proc], ["dir"], ["search"]);
            allow(["hwservicemanager"], [proc], ["file"], ["read", "open"]);
            allow(["hwservicemanager"], [proc], ["process"], ["getattr"]);

            // For mounting loop devices, mirrors, tmpfs
            allow(["kernel"], all, ["file"], ["read", "write"]);

            // Allow all binder transactions
            allow(all, [proc], ["binder"], all);

            // For changing file context
            allow(["rootfs"], ["tmpfs"], ["filesystem"], ["associate"]);

            // Zygisk rules
            allow(["zygote"], ["zygote"], ["process"], ["execmem"]);
            allow(["zygote"], ["fs_type"], ["filesystem"], ["unmount"]);
            allow(["system_server"], ["system_server"], ["process"], ["execmem"]);

            // Shut llkd up
            dontaudit(["llkd"], [proc], ["process"], ["ptrace"]);

            // Keep /data/adb/* context
            deny(["init"], ["adb_data_file"], ["dir"], ["search"]);
            deny(["vendor_init"], ["adb_data_file"], ["dir"], ["search"]);
        }
        // Remove all dontaudit in debug mode
        #[cfg(debug_assertions)]
        self.as_mut().strip_dontaudit();

        set_log_level_state(LogLevel::Warn, true);
    }
}
