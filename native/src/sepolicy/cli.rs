use crate::ffi::SePolicy;
use crate::statement::format_statement_help;
use argh::FromArgs;
use base::libc::umask;
use base::{
    CmdArgs, EarlyExitExt, FmtAdaptor, LoggedResult, Utf8CString, argh, cmdline_logging, cstr,
    log_err,
};
use std::ffi::c_char;
use std::io::stderr;

#[derive(FromArgs)]
struct Cli {
    #[argh(switch)]
    live: bool,

    #[argh(switch)]
    magisk: bool,

    #[argh(switch)]
    compile_split: bool,

    #[argh(switch)]
    load_split: bool,

    #[argh(switch)]
    print_rules: bool,

    #[argh(option)]
    load: Option<Utf8CString>,

    #[argh(option)]
    save: Option<Utf8CString>,

    #[argh(option)]
    apply: Vec<Utf8CString>,

    #[argh(positional)]
    polices: Vec<String>,
}

fn print_usage(cmd: &str) {
    eprintln!(
        r#"MagiskPolicy - SELinux Policy Patch Tool

Usage: {cmd} [--options...] [policy statements...]

Options:
   --help            show help message for policy statements
   --load FILE       load monolithic sepolicy from FILE
   --load-split      load from precompiled sepolicy or compile
                     split cil policies
   --compile-split   compile split cil policies
   --save FILE       dump monolithic sepolicy to FILE
   --live            immediately load sepolicy into the kernel
   --magisk          apply built-in Magisk sepolicy rules
   --apply FILE      apply rules from FILE, read and parsed
                     line by line as policy statements
                     (multiple --apply are allowed)
   --print-rules     print all rules in the loaded sepolicy

If neither --load, --load-split, nor --compile-split is specified,
it will load from current live policies (/sys/fs/selinux/policy)
"#
    );

    format_statement_help(&mut FmtAdaptor(&mut stderr())).ok();
    eprintln!();
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn main(
    argc: i32,
    argv: *const *const c_char,
    _envp: *const *const c_char,
) -> i32 {
    cmdline_logging();
    unsafe {
        umask(0);
    }

    let res: LoggedResult<()> = try {
        let cmds = CmdArgs::new(argc, argv);
        let cmds = cmds.as_slice();
        if argc < 2 {
            print_usage(cmds.first().unwrap_or(&"magiskpolicy"));
            return 1;
        }
        let cli = Cli::from_args(&[cmds[0]], &cmds[1..]).on_early_exit(|| print_usage(cmds[0]));

        let mut sepol = match (cli.load, cli.load_split, cli.compile_split) {
            (Some(file), false, false) => SePolicy::from_file(&file),
            (None, true, false) => SePolicy::from_split(),
            (None, false, true) => SePolicy::compile_split(),
            (None, false, false) => SePolicy::from_file(cstr!("/sys/fs/selinux/policy")),
            _ => log_err!("Multiple load source supplied")?,
        };
        if sepol._impl.is_null() {
            log_err!("Cannot load policy")?;
        }

        if cli.print_rules {
            if cli.magisk
                || !cli.apply.is_empty()
                || !cli.polices.is_empty()
                || cli.live
                || cli.save.is_some()
            {
                log_err!("Cannot print rules with other options")?;
            }
            sepol.print_rules();
            return 0;
        }

        if cli.magisk {
            sepol.magisk_rules();
        }

        for file in cli.apply {
            sepol.load_rule_file(&file);
        }

        for statement in &cli.polices {
            sepol.load_rules(statement);
        }

        if cli.live && !sepol.to_file(cstr!("/sys/fs/selinux/load")) {
            log_err!("Cannot apply policy")?;
        }

        if let Some(file) = cli.save
            && !sepol.to_file(&file)
        {
            log_err!("Cannot dump policy to {}", file)?;
        }
    };
    if res.is_ok() { 0 } else { 1 }
}
