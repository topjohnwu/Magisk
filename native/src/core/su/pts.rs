use base::{FileOrStd, LibcReturn, LoggedResult, OsResult, ResultExt, libc, warn};
use libc::{STDIN_FILENO, STDOUT_FILENO, TIOCGWINSZ, TIOCSWINSZ, c_int, ssize_t, winsize};
use nix::{
    fcntl::OFlag,
    poll::{PollFd, PollFlags, PollTimeout, poll},
    sys::signal::{SigSet, Signal, raise},
    sys::signalfd::{SfdFlags, SignalFd},
    sys::termios::{SetArg, Termios, cfmakeraw, tcgetattr, tcsetattr},
    unistd::pipe2,
};
use std::fs::File;
use std::io::{Read, Write};
use std::mem::{ManuallyDrop, MaybeUninit};
use std::os::fd::{AsFd, AsRawFd, FromRawFd, OwnedFd, RawFd};
use std::ptr::null_mut;
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

fn splice(fd_in: RawFd, fd_out: RawFd, len: usize, flags: u32) -> OsResult<'static, ssize_t> {
    unsafe { libc::splice(fd_in, null_mut(), fd_out, null_mut(), len, flags) }
        .into_os_result("splice", None, None)
}

fn pump_via_copy(infd: RawFd, outfd: RawFd) -> LoggedResult<()> {
    let mut buf = MaybeUninit::<[u8; 4096]>::uninit();
    let buf = unsafe { buf.assume_init_mut() };
    let mut infd = ManuallyDrop::new(unsafe { File::from_raw_fd(infd) });
    let mut outfd = ManuallyDrop::new(unsafe { File::from_raw_fd(outfd) });
    let len = infd.read(buf)?;
    outfd.write_all(&buf[..len])?;
    Ok(())
}

fn pump_via_splice(infd: RawFd, outfd: RawFd, pipe: &(OwnedFd, OwnedFd)) -> LoggedResult<()> {
    if !SHOULD_USE_SPLICE.load(Ordering::Acquire) {
        return pump_via_copy(infd, outfd);
    }

    // The pipe capacity is by default 16 pages, let's just use 65536
    let Ok(len) = splice(infd, pipe.1.as_raw_fd(), 65536_usize, 0) else {
        // If splice failed, stop using splice and fallback to userspace copy
        SHOULD_USE_SPLICE.store(false, Ordering::Release);
        return pump_via_copy(infd, outfd);
    };
    if len == 0 {
        return Ok(());
    }
    splice(pipe.0.as_raw_fd(), outfd, len as usize, 0)?;
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

fn pump_tty_impl(ptmx: OwnedFd, pump_stdin: bool) -> LoggedResult<()> {
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

    'poll: loop {
        // Wait for event
        poll(&mut poll_fds, PollTimeout::NONE).check_os_err("poll", None, None)?;
        for pfd in &poll_fds {
            if pfd.all().unwrap_or(false) {
                let raw_fd = pfd.as_fd().as_raw_fd();
                if raw_fd == STDIN_FILENO {
                    pump_via_splice(STDIN_FILENO, raw_ptmx, &pipe_fd)?;
                } else if raw_fd == raw_ptmx {
                    pump_via_splice(raw_ptmx, STDOUT_FILENO, &pipe_fd)?;
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

    let ptmx = unsafe { OwnedFd::from_raw_fd(ptmx) };
    pump_tty_impl(ptmx, pump_stdin).ok();

    if let Some(term) = old_term {
        restore_stdin(term).ok();
    }
    raise(Signal::SIGWINCH).ok();
}
