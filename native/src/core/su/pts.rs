use base::{FileOrStd, LibcReturn, LoggedResult, OsResult, libc, warn};
use libc::{
    STDIN_FILENO, STDOUT_FILENO, TCSADRAIN, TCSAFLUSH, TIOCGWINSZ, TIOCSWINSZ, c_int, cfmakeraw,
    ssize_t, tcgetattr, tcsetattr, termios, winsize,
};
use nix::{
    fcntl::OFlag,
    poll::{PollFd, PollFlags, PollTimeout},
    sys::signal::{SigSet, Signal},
    sys::signalfd::{SfdFlags, SignalFd},
};
use std::fs::File;
use std::io::{Read, Write};
use std::mem::{ManuallyDrop, MaybeUninit};
use std::os::fd::{AsFd, AsRawFd, FromRawFd, OwnedFd, RawFd};
use std::ptr::null_mut;
use std::sync::atomic::{AtomicBool, Ordering};

static mut OLD_STDIN: Option<termios> = None;
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

fn set_stdin_raw() -> bool {
    unsafe {
        let mut termios: termios = std::mem::zeroed();

        if tcgetattr(STDIN_FILENO, &mut termios) < 0 {
            return false;
        }

        let old_c_oflag = termios.c_oflag;
        OLD_STDIN = Some(termios);

        cfmakeraw(&mut termios);

        // don't modify output flags, since we are not setting stdout raw
        termios.c_oflag = old_c_oflag;

        if tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios) < 0
            && tcsetattr(STDIN_FILENO, TCSADRAIN, &termios) < 0
        {
            warn!("Failed to set terminal attributes");
            return false;
        }
    }

    true
}

pub fn restore_stdin() -> bool {
    unsafe {
        if let Some(ref termios) = OLD_STDIN
            && tcsetattr(STDIN_FILENO, TCSAFLUSH, termios) < 0
            && tcsetattr(STDIN_FILENO, TCSADRAIN, termios) < 0
        {
            warn!("Failed to restore terminal attributes");
            return false;
        }
        OLD_STDIN = None;
        true
    }
}

fn resize_pty(outfd: i32) {
    let mut ws: winsize = unsafe { std::mem::zeroed() };
    if unsafe { ioctl(STDIN_FILENO, TIOCGWINSZ as u32, &mut ws) } >= 0 {
        unsafe { ioctl(outfd, TIOCSWINSZ as u32, &ws) };
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

fn pump_tty_impl(infd: OwnedFd, raw_out: RawFd) -> LoggedResult<()> {
    let mut set = SigSet::empty();
    set.add(Signal::SIGWINCH);
    set.thread_block()
        .check_os_err("pthread_sigmask", None, None)?;
    let signal_fd =
        SignalFd::with_flags(&set, SfdFlags::SFD_CLOEXEC).into_os_result("signalfd", None, None)?;

    let raw_in = infd.as_raw_fd();
    let raw_sig = signal_fd.as_raw_fd();

    let mut poll_fds = Vec::with_capacity(3);
    poll_fds.push(PollFd::new(infd.as_fd(), PollFlags::POLLIN));
    poll_fds.push(PollFd::new(signal_fd.as_fd(), PollFlags::POLLIN));
    if raw_out >= 0 {
        poll_fds.push(PollFd::new(
            FileOrStd::StdIn.as_file().as_fd(),
            PollFlags::POLLIN,
        ));
    }

    // Any flag in this list indicates stop polling
    let stop_flags = PollFlags::POLLERR | PollFlags::POLLHUP | PollFlags::POLLNVAL;

    // Open a pipe to bypass userspace copy with splice
    let pipe_fd = nix::unistd::pipe2(OFlag::O_CLOEXEC).into_os_result("pipe2", None, None)?;

    'poll: loop {
        // Wait for event
        nix::poll::poll(&mut poll_fds, PollTimeout::NONE).check_os_err("poll", None, None)?;
        for pfd in &poll_fds {
            if pfd.all().unwrap_or(false) {
                let raw_fd = pfd.as_fd().as_raw_fd();
                if raw_fd == STDIN_FILENO {
                    pump_via_splice(STDIN_FILENO, raw_out, &pipe_fd)?;
                } else if raw_fd == raw_in {
                    pump_via_splice(raw_in, STDOUT_FILENO, &pipe_fd)?;
                } else if raw_fd == raw_sig {
                    resize_pty(raw_out);
                    signal_fd.read_signal()?;
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

pub fn pump_tty(infd: i32, outfd: i32) {
    set_stdin_raw();

    let infd = unsafe { OwnedFd::from_raw_fd(infd) };
    pump_tty_impl(infd, outfd).ok();

    restore_stdin();
    nix::sys::signal::raise(Signal::SIGWINCH).ok();
}
