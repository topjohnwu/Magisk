use base::{FileOrStd, LibcReturn, LoggedResult, OsResult, ResultExt, libc, warn};
use libc::{STDIN_FILENO, TIOCGWINSZ, TIOCSWINSZ, c_int, winsize};
use nix::fcntl::{OFlag, SpliceFFlags};
use nix::poll::{PollFd, PollFlags, PollTimeout, poll};
use nix::sys::signal::{SigSet, Signal, raise};
use nix::sys::signalfd::{SfdFlags, SignalFd};
use nix::sys::termios::{SetArg, Termios, cfmakeraw, tcgetattr, tcsetattr};
use nix::unistd::pipe2;
use std::fs::File;
use std::io::{Read, Write};
use std::mem::MaybeUninit;
use std::os::fd::{AsFd, AsRawFd, FromRawFd, RawFd};
use std::sync::atomic::{AtomicBool, Ordering};

static SHOULD_USE_SPLICE: AtomicBool = AtomicBool::new(true);
const TIOCGPTN: u32 = 0x80045430;

unsafe extern "C" {
    // Don't use the declaration from the libc crate as request should be u32 not i32
    fn ioctl(fd: c_int, request: u32, ...) -> i32;
}

pub fn get_pty_num(fd: i32) -> i32 {
    let mut pty_num = -1i32;
    if unsafe { ioctl(fd, TIOCGPTN, &mut pty_num) } != 0 {
        warn!("Failed to get pty number");
    }
    pty_num
}

fn sync_winsize(ptmx: i32) {
    let mut ws: winsize = unsafe { std::mem::zeroed() };
    if unsafe { ioctl(STDIN_FILENO, TIOCGWINSZ as u32, &mut ws) } >= 0 {
        unsafe { ioctl(ptmx, TIOCSWINSZ as u32, &ws) };
    }
}

fn splice(fd_in: impl AsFd, fd_out: impl AsFd, len: usize) -> OsResult<'static, usize> {
    nix::fcntl::splice(fd_in, None, fd_out, None, len, SpliceFFlags::empty())
        .into_os_result("splice", None, None)
}

fn pump_via_copy(mut fd_in: &File, mut fd_out: &File) -> LoggedResult<()> {
    let mut buf = MaybeUninit::<[u8; 4096]>::uninit();
    let buf = unsafe { buf.assume_init_mut() };
    let len = fd_in.read(buf)?;
    fd_out.write_all(&buf[..len])?;
    Ok(())
}

fn pump_via_splice(fd_in: &File, fd_out: &File, pipe: &(File, File)) -> LoggedResult<()> {
    if !SHOULD_USE_SPLICE.load(Ordering::Relaxed) {
        return pump_via_copy(fd_in, fd_out);
    }

    // The pipe capacity is by default 16 pages, let's just use 65536
    let Ok(len) = splice(fd_in, &pipe.1, 65536) else {
        // If splice failed, stop using splice and fallback to userspace copy
        SHOULD_USE_SPLICE.store(false, Ordering::Relaxed);
        return pump_via_copy(fd_in, fd_out);
    };
    if len == 0 {
        return Ok(());
    }
    if splice(&pipe.0, fd_out, len).is_err() {
        // If splice failed, stop using splice and fallback to userspace copy
        SHOULD_USE_SPLICE.store(false, Ordering::Relaxed);
        return pump_via_copy(&pipe.0, fd_out);
    }
    Ok(())
}

fn set_stdin_raw() -> LoggedResult<Termios> {
    let mut term = tcgetattr(FileOrStd::StdIn.as_file())?;
    let old_term = term.clone();

    let old_output_flags = old_term.output_flags;
    cfmakeraw(&mut term);

    // Preserve output_flags, since we are not setting stdout raw
    term.output_flags = old_output_flags;

    tcsetattr(FileOrStd::StdIn.as_file(), SetArg::TCSAFLUSH, &term)
        .or_else(|_| tcsetattr(FileOrStd::StdIn.as_file(), SetArg::TCSADRAIN, &term))
        .check_os_err("tcsetattr", None, None)
        .log_with_msg(|w| w.write_str("Failed to set terminal attributes"))?;

    Ok(old_term)
}

fn restore_stdin(term: Termios) -> LoggedResult<()> {
    tcsetattr(FileOrStd::StdIn.as_file(), SetArg::TCSAFLUSH, &term)
        .or_else(|_| tcsetattr(FileOrStd::StdIn.as_file(), SetArg::TCSADRAIN, &term))
        .check_os_err("tcsetattr", None, None)
        .log_with_msg(|w| w.write_str("Failed to restore terminal attributes"))
}

fn pump_tty_impl(ptmx: File, pump_stdin: bool) -> LoggedResult<()> {
    let mut signal_fd: Option<SignalFd> = None;

    let raw_ptmx = ptmx.as_raw_fd();
    let mut raw_sig = -1;

    let mut poll_fds = Vec::with_capacity(3);
    poll_fds.push(PollFd::new(ptmx.as_fd(), PollFlags::POLLIN));
    if pump_stdin {
        // If stdin is tty, we need to monitor SIGWINCH
        let mut set = SigSet::empty();
        set.add(Signal::SIGWINCH);
        set.thread_block()
            .check_os_err("pthread_sigmask", None, None)?;
        let sig = SignalFd::with_flags(&set, SfdFlags::SFD_CLOEXEC)
            .into_os_result("signalfd", None, None)?;
        raw_sig = sig.as_raw_fd();
        signal_fd = Some(sig);
        poll_fds.push(PollFd::new(
            signal_fd.as_ref().unwrap().as_fd(),
            PollFlags::POLLIN,
        ));

        // We also need to pump stdin to ptmx
        poll_fds.push(PollFd::new(
            FileOrStd::StdIn.as_file().as_fd(),
            PollFlags::POLLIN,
        ));
    }

    // Any flag in this list indicates stop polling
    let stop_flags = PollFlags::POLLERR | PollFlags::POLLHUP | PollFlags::POLLNVAL;

    // Open a pipe to bypass userspace copy with splice
    let pipe_fd = pipe2(OFlag::O_CLOEXEC).into_os_result("pipe2", None, None)?;
    let pipe_fd = (File::from(pipe_fd.0), File::from(pipe_fd.1));

    'poll: loop {
        // Wait for event
        poll(&mut poll_fds, PollTimeout::NONE).check_os_err("poll", None, None)?;
        for pfd in &poll_fds {
            if pfd.all().unwrap_or(false) {
                let raw_fd = pfd.as_fd().as_raw_fd();
                if raw_fd == STDIN_FILENO {
                    pump_via_splice(FileOrStd::StdIn.as_file(), &ptmx, &pipe_fd)?;
                } else if raw_fd == raw_ptmx {
                    pump_via_splice(&ptmx, FileOrStd::StdOut.as_file(), &pipe_fd)?;
                } else if raw_fd == raw_sig {
                    sync_winsize(raw_ptmx);
                    signal_fd.as_ref().unwrap().read_signal()?;
                }
            } else if pfd
                .revents()
                .unwrap_or(PollFlags::POLLHUP)
                .intersects(stop_flags)
            {
                // If revents is None or contains any err_flags, stop polling
                break 'poll;
            }
        }
    }
    Ok(())
}

pub fn pump_tty(ptmx: RawFd, pump_stdin: bool) {
    let old_term = if pump_stdin {
        sync_winsize(ptmx);
        set_stdin_raw().ok()
    } else {
        None
    };

    let ptmx = unsafe { File::from_raw_fd(ptmx) };
    pump_tty_impl(ptmx, pump_stdin).ok();

    if let Some(term) = old_term {
        restore_stdin(term).ok();
    }
    raise(Signal::SIGWINCH).ok();
}
