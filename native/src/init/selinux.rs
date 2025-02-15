use crate::ffi::MagiskInit;
use base::{cstr, debug, FsPath, Utf8CStr};
use magiskpolicy::ffi::SePolicy;

impl MagiskInit {
    pub(crate) fn patch_sepolicy(self: &MagiskInit, src: &Utf8CStr, out: &Utf8CStr) {
        debug!("Patching monolithic policy");
        let mut sepol = SePolicy::from_file(src);

        sepol.magisk_rules();

        // Custom rules
        let rule = FsPath::from(cstr!("/data/.magisk/preinit/sepolicy.rule"));
        if rule.exists() {
            debug!("Loading custom sepolicy patch: [{}]", rule);
            sepol.load_rule_file(rule);
        }

        debug!("Dumping sepolicy to: [{}]", out);
        sepol.to_file(out);

        // Remove OnePlus stupid debug sepolicy and use our own
        let sepol_debug = FsPath::from(cstr!("/sepolicy_debug"));
        if sepol_debug.exists() {
            sepol_debug.remove().ok();
            FsPath::from(cstr!("/sepolicy")).link_to(sepol_debug).ok();
        }
    }
}
