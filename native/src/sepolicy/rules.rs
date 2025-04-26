use crate::consts::{SEPOL_FILE_TYPE, SEPOL_LOG_TYPE, SEPOL_PROC_DOMAIN};
use crate::{SePolicy, ffi::Xperm};
use base::{LogLevel, set_log_level_state};

macro_rules! rules {
    (@args all) => {
        vec![]
    };
    (@args xall) => {
        vec![Xperm { low: 0x0000, high: 0xFFFF, reset: false }]
    };
    (@args svcmgr) => {
        vec!["servicemanager", "vndservicemanager", "hwservicemanager"]
    };
    (@args [proc]) => {
        vec![SEPOL_PROC_DOMAIN]
    };
    (@args [file]) => {
        vec![SEPOL_FILE_TYPE]
    };
    (@args [log]) => {
        vec![SEPOL_LOG_TYPE]
    };
    (@args proc) => {
        SEPOL_PROC_DOMAIN
    };
    (@args file) => {
        SEPOL_FILE_TYPE
    };
    (@args log) => {
        SEPOL_LOG_TYPE
    };
    (@args [$($arg:tt)*]) => {
        vec![$($arg)*]
    };
    (@args $arg:expr) => {
        $arg
    };
    (@stmt $self:ident) => {};
    (@stmt $self:ident $action:ident($($args:tt),*); $($res:tt)*) => {
        $self.$action($(rules!(@args $args)),*);
        rules!{@stmt $self $($res)* }
    };
    (use $self:ident; $($res:tt)*) => {{
        rules!{@stmt $self $($res)* }
    }};
}

impl SePolicy {
    pub fn magisk_rules(&mut self) {
        // Temp suppress warnings
        set_log_level_state(LogLevel::Warn, false);
        rules! {
            use self;
            // Prevent anything to change sepolicy except ourselves
            deny(all, ["kernel"], ["security"], ["load_policy"]);
            type_(proc, ["domain"]);
            typeattribute([proc], ["mlstrustedsubject", "netdomain", "appdomain"]);
            type_(file, ["file_type"]);
            typeattribute([file], ["mlstrustedobject"]);
            type_(log, ["file_type"]);
            typeattribute([log], ["mlstrustedobject"]);

            // Create unconstrained file type
            allow(["domain"], [file],
                ["file", "dir", "fifo_file", "chr_file", "lnk_file", "sock_file"], all);

            // Only allow zygote to open log pipe
            allow(["zygote"], [log], ["fifo_file"], ["open", "read"]);
            // Allow all processes to output logs
            allow(["domain"], [log], ["fifo_file"], ["write"]);

            // Make our root domain unconstrained
            allow([proc], [
                "fs_type", "dev_type", "file_type", "domain",
                "service_manager_type", "hwservice_manager_type", "vndservice_manager_type",
                "port_type", "node_type", "property_type"
            ], all, all);

            // Just in case, make the domain permissive
            permissive([proc]);

            // Allow us to do any ioctl
            allowxperm([proc], ["fs_type", "dev_type", "file_type", "domain"],
                ["blk_file", "fifo_file", "chr_file"], xall);
            allowxperm([proc], [proc], ["tcp_socket", "udp_socket", "rawip_socket"], xall);

            // Let binder work with our processes
            allow(svcmgr, [proc], ["dir"], ["search"]);
            allow(svcmgr, [proc], ["file"], ["open", "read", "map"]);
            allow(svcmgr, [proc], ["process"], ["getattr"]);
            allow(["domain"], [proc], ["binder"], ["call", "transfer"]);

            // Other common IPC
            allow(["domain"], [proc], ["process"], ["sigchld"]);
            allow(["domain"], [proc], ["fd"], ["use"]);
            allow(["domain"], [proc], ["fifo_file"], ["write", "read", "open", "getattr"]);

            // Allow these processes to access MagiskSU and output logs
            allow(["zygote", "shell", "platform_app",
                "system_app", "priv_app", "untrusted_app", "untrusted_app_all"],
                [proc], ["unix_stream_socket"], ["connectto", "getopt"]);

            // Let selected domains access tmpfs files
            // For tmpfs overlay on 2SI, Zygisk on lower Android versions and AVD scripts
            allow(["init", "zygote", "shell"], ["tmpfs"], ["file"], all);

            // Allow magiskinit daemon to log to kmsg
            allow(["kernel"], ["rootfs", "tmpfs"], ["chr_file"], ["write"]);

            // Allow magiskinit daemon to handle mock selinuxfs
            allow(["kernel"], ["tmpfs"], ["fifo_file"], ["open", "read", "write"]);

            // For relabelling files
            allow(["rootfs"], ["labeledfs", "tmpfs"], ["filesystem"], ["associate"]);
            allow([file], ["pipefs", "devpts"], ["filesystem"], ["associate"]);
            allow(["kernel"], all, ["file"], ["relabelto"]);
            allow(["kernel"], ["tmpfs"], ["file"], ["relabelfrom"]);

            // Let init transit to SEPOL_PROC_DOMAIN
            allow(["kernel"], ["kernel"], ["process"], ["setcurrent"]);
            allow(["kernel"], [proc], ["process"], ["dyntransition"]);

            // Let init run stuffs
            allow(["init"], [proc], ["process"], all);

            // For mounting loop devices, mirrors, tmpfs
            allow(["kernel"], ["fs_type", "dev_type", "file_type"], ["file"], ["read", "write"]);

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

        #[cfg(any())]
        self.strip_dontaudit();

        set_log_level_state(LogLevel::Warn, true);
    }
}
