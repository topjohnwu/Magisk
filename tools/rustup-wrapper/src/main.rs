use std::env;
use std::path::Path;
use std::process::{Command, Stdio};

use home::cargo_home;

/********************************
 * Why do we need this wrapper?
 ********************************
 *
 * The command `rustup component list` does not work with custom toolchains:
 * > error: toolchain 'magisk' does not support components
 *
 * However, this command is used by several IDEs to determine component
 * availability, such as clippy, rustfmt etc.
 * In this program, we use the output of the command with the nightly
 * channel if any `component` command failed.
*/

fn main() -> std::io::Result<()> {
    let exe = env::args().next().unwrap();
    let exe = Path::new(&exe).file_name().unwrap().to_str().unwrap();
    let real_exe = cargo_home()?.join("bin").join(exe);
    let argv: Vec<String> = env::args().skip(1).collect();

    if exe.starts_with("rustup") && argv.iter().any(|s| s == "component") {
        let status = Command::new(&real_exe)
            .args(&argv)
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status()?;
        if !status.success() {
            let mut cmd = Command::new(&real_exe);
            // Hardcode to use the nightly channel
            cmd.arg("+nightly");
            // Remove any explicit channel specification
            cmd.args(argv.iter().filter(|s| !s.starts_with('+')));
            return cmd.status().map(|_| ());
        }
    }

    // Simply pass through
    Command::new(&real_exe).args(argv.iter()).status().map(|_| ())
}
