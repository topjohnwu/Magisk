use crate::consts::{PREINITMIRR, SELINUXMOCK};
use crate::ffi::{MagiskInit, preload_ack, preload_lib, preload_policy, split_plat_cil};
use base::const_format::concatcp;
use base::{
    BytesExt, LibcReturn, LoggedResult, MappedFile, ResultExt, Utf8CStr, cstr, debug, error, info,
    libc, raw_cstr,
};
use magiskpolicy::ffi::SePolicy;
use std::io::{Read, Write};
use std::ptr;
use std::thread::sleep;
use std::time::Duration;

const MOCK_VERSION: &Utf8CStr = cstr!(concatcp!(SELINUXMOCK, "/version"));
const MOCK_LOAD: &Utf8CStr = cstr!(concatcp!(SELINUXMOCK, "/load"));
const MOCK_ENFORCE: &Utf8CStr = cstr!(concatcp!(SELINUXMOCK, "/enforce"));
const MOCK_REQPROT: &Utf8CStr = cstr!(concatcp!(SELINUXMOCK, "/checkreqprot"));

const SELINUX_MNT: &str = "/sys/fs/selinux";
const SELINUX_ENFORCE: &Utf8CStr = cstr!(concatcp!(SELINUX_MNT, "/enforce"));
const SELINUX_LOAD: &Utf8CStr = cstr!(concatcp!(SELINUX_MNT, "/load"));
const SELINUX_REQPROT: &Utf8CStr = cstr!(concatcp!(SELINUX_MNT, "/checkreqprot"));

enum SePatchStrategy {
    // 2SI, Android 10+
    // On 2SI devices, the 2nd stage init is always a dynamic executable.
    // This meant that instead of going through convoluted hacks, we can just
    // LD_PRELOAD and replace security_load_policy with our own implementation.
    LdPreload,
    // Treble enabled, Android 8.0+
    // selinuxfs is mounted in init.cpp. Errors when mounting selinuxfs is ignored,
    // which means that we can directly mount selinuxfs ourselves and hijack nodes in it.
    SelinuxFs,
    // Dynamic patching, Android 6.0 - 7.1
    // selinuxfs is mounted in libselinux's selinux_android_load_policy(). Errors when
    // mounting selinuxfs is fatal, which means we need to block init's control flow after
    // it mounted selinuxfs for us, then we can hijack nodes in it.
    Legacy,
}

// Note for non-LD_PRELOAD strategy:
//
// We need to make sure the actual init process is blocked until sepolicy is loaded,
// or else restorecon will fail and re-exec won't change context, causing boot failure.
// We (ab)use the fact that init either reads the enforce node, or writes the checkreqprot
// node, and because both has been replaced with FIFO files, init will block until we
// handle it, effectively hijacking its control flow until the patched sepolicy is loaded.

fn mock_fifo(target: &Utf8CStr, mock: &Utf8CStr) -> LoggedResult<()> {
    debug!("Hijack [{}]", target);
    mock.mkfifo(0o666)?;
    mock.bind_mount_to(target, false).log()
}

fn mock_file(target: &Utf8CStr, mock: &Utf8CStr) -> LoggedResult<()> {
    debug!("Hijack [{}]", target);
    drop(mock.create(libc::O_RDONLY, 0o666)?);
    mock.bind_mount_to(target, false).log()
}

impl MagiskInit {
    pub(crate) fn handle_sepolicy(&mut self) {
        self.handle_sepolicy_impl().ok();
    }

    fn cleanup_and_load(&self, rules: &str) {
        // Cleanup the hijacks
        cstr!("/init").unmount().ok();
        SELINUX_LOAD.unmount().log_ok();
        SELINUX_ENFORCE.unmount().ok();
        SELINUX_REQPROT.unmount().ok();

        let mut sepol = SePolicy::from_file(MOCK_LOAD);
        sepol.magisk_rules();
        sepol.load_rules(rules);
        sepol.to_file(SELINUX_LOAD);

        // For some reason, restorecon on /init won't work in some cases
        cstr!("/init")
            .follow_link()
            .set_secontext(cstr!("u:object_r:init_exec:s0"))
            .ok();

        // restore mounted files' context after sepolicy loaded
        self.restore_overlay_contexts();
    }

    fn handle_sepolicy_impl(&mut self) -> LoggedResult<()> {
        cstr!(SELINUXMOCK).mkdir(0o711)?;

        let mut rules = String::new();
        let mut policy_ver = cstr!("/selinux_version");
        let rule_file = cstr!(concatcp!("/data/", PREINITMIRR, "/sepolicy.rule"));
        if rule_file.exists() {
            debug!("Loading custom sepolicy patch: [{}]", rule_file);
            rule_file.open(libc::O_RDONLY)?.read_to_string(&mut rules)?;
        }

        // Step 0: determine strategy

        let strat: SePatchStrategy;

        if cstr!("/system/bin/init").exists() {
            strat = SePatchStrategy::LdPreload;
        } else {
            let init = MappedFile::open(cstr!("/init"))?;
            if init.contains(split_plat_cil().as_str().as_bytes()) {
                // Supports split policy
                strat = SePatchStrategy::SelinuxFs;
            } else if init.contains(policy_ver.as_bytes()) {
                // Does not support split policy, hijack /selinux_version
                strat = SePatchStrategy::Legacy;
            } else if init.contains(cstr!("/sepolicy_version").as_bytes()) {
                // Samsung custom path
                policy_ver = cstr!("/sepolicy_version");
                strat = SePatchStrategy::Legacy;
            } else {
                error!("Unknown sepolicy setup, abort...");
                return Ok(());
            }
        }

        // Step 1: setup for intercepting init boot control flow

        match strat {
            SePatchStrategy::LdPreload => {
                info!("SePatchStrategy: LD_PRELOAD");

                cstr!("init-ld").copy_to(preload_lib())?;
                unsafe {
                    libc::setenv(raw_cstr!("LD_PRELOAD"), preload_lib().as_ptr(), 1);
                }
                preload_ack().mkfifo(0o666)?;
            }
            SePatchStrategy::SelinuxFs => {
                info!("SePatchStrategy: SELINUXFS");

                if !SELINUX_ENFORCE.exists() {
                    // selinuxfs was not already mounted, mount it ourselves

                    // Remount procfs with proper options
                    cstr!("/proc").remount_with_data(cstr!("hidepid=2,gid=3009"))?;

                    // Preserve sysfs and procfs
                    self.mount_list.retain(|s| s != "/proc" && s != "/sys");

                    // Mount selinuxfs
                    unsafe {
                        libc::mount(
                            raw_cstr!("selinuxfs"),
                            raw_cstr!(SELINUX_MNT),
                            raw_cstr!("selinuxfs"),
                            0,
                            ptr::null(),
                        )
                        .check_io_err()?;
                    }
                }

                mock_file(SELINUX_LOAD, MOCK_LOAD)?;
                mock_fifo(SELINUX_ENFORCE, MOCK_ENFORCE)?;
            }
            SePatchStrategy::Legacy => {
                info!("SePatchStrategy: LEGACY");

                if !policy_ver.exists() {
                    // The file does not exist, create one
                    drop(policy_ver.create(libc::O_RDONLY, 0o666)?);
                }

                // The only purpose of this is to block init's control flow after it mounts
                // selinuxfs and before it calls security_load_policy().
                // selinux_android_load_policy() -> set_policy_index() -> open(policy_ver)
                mock_fifo(policy_ver, MOCK_VERSION)?;
            }
        }

        // Create a new process waiting for init operations
        let pid = unsafe { libc::fork() };
        if pid != 0 {
            return Ok(());
        }

        // Step 2: wait for selinuxfs to be mounted (only for LEGACY)

        let wait = Duration::from_millis(100);

        if matches!(strat, SePatchStrategy::Legacy) {
            // Busy wait until selinuxfs is mounted
            while !SELINUX_ENFORCE.exists() {
                // Retry every 100ms
                sleep(wait);
            }

            // On Android 6.0, init does not call security_getenforce() first; instead it directly
            // call security_setenforce() after security_load_policy(). What's even worse, it opens
            // the enforce node with O_RDWR, which will not block when opening FIFO files.
            // As a workaround, we do not mock the enforce node, and block init with mocking
            // checkreqprot instead.
            // Android 7.0 - 7.1 doesn't have this issue, but for simplicity, let's just use the
            // same blocking strategy for both since it also works just fine.

            mock_file(SELINUX_LOAD, MOCK_LOAD)?;
            mock_fifo(SELINUX_REQPROT, MOCK_REQPROT)?;

            // This will unblock init at selinux_android_load_policy() -> set_policy_index().
            drop(MOCK_VERSION.open(libc::O_WRONLY)?);

            policy_ver.unmount()?;

            // libselinux does not read /selinux_version after open; instead it mmap the file,
            // which can never succeed on FIFO files. This is fine as set_policy_index() will just
            // fallback to the default index 0.
        }

        // Step 3: obtain sepolicy, patch, and load the patched sepolicy

        match strat {
            SePatchStrategy::LdPreload => {
                // This open will block until preload.so finish writing the sepolicy
                let mut ack_fd = preload_ack().open(libc::O_WRONLY)?;

                let mut sepol = SePolicy::from_file(preload_policy());

                // Remove the files before loading the policy
                preload_policy().remove()?;
                preload_ack().remove()?;

                sepol.magisk_rules();
                sepol.load_rules(&rules);
                sepol.to_file(SELINUX_LOAD);

                self.restore_overlay_contexts();

                // Write ack to restore preload.so's control flow
                ack_fd.write_all("0".as_bytes())?;
            }
            SePatchStrategy::SelinuxFs => {
                // This open will block until init calls security_getenforce().
                let mut mock_enforce = MOCK_ENFORCE.open(libc::O_WRONLY)?;

                self.cleanup_and_load(&rules);

                // security_getenforce was called, read from real and redirect to mock
                let mut data = vec![];
                SELINUX_ENFORCE
                    .open(libc::O_RDONLY)?
                    .read_to_end(&mut data)?;
                mock_enforce.write_all(&data)?;
            }
            SePatchStrategy::Legacy => {
                let mut sz = 0_usize;
                // Busy wait until sepolicy is fully written.
                loop {
                    let attr = MOCK_LOAD.get_attr()?;
                    if sz != 0 && sz == attr.st.st_size as usize {
                        break;
                    }
                    sz = attr.st.st_size as usize;
                    // Check every 100ms
                    sleep(wait);
                }

                self.cleanup_and_load(&rules);

                // init is blocked on checkreqprot, write to the real node first, then
                // unblock init by opening the mock FIFO.
                SELINUX_REQPROT
                    .open(libc::O_WRONLY)?
                    .write_all("0".as_bytes())?;
                let mut v = vec![];
                MOCK_REQPROT.open(libc::O_RDONLY)?.read_to_end(&mut v)?;
            }
        }

        // At this point, the init process will be unblocked
        // and continue on with restorecon + re-exec.

        // Terminate process
        std::process::exit(0);
    }
}
