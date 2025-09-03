use base::libc::ssize_t;
use base::{
    LibcReturn, LoggedResult, OsResult, PipeFd, ReadExt, ResultExt, error, libc, log_err,
    make_pipe, warn,
};
use bytemuck::{Pod, Zeroable};
use libc::{
    O_CLOEXEC, POLLIN, SFD_CLOEXEC, SIG_BLOCK, SIGWINCH, STDIN_FILENO, STDOUT_FILENO, TCSADRAIN,
    TCSAFLUSH, TIOCGWINSZ, TIOCSWINSZ, cfmakeraw, close, poll, pollfd, raise, sigaddset,
    sigemptyset, signalfd, signalfd_siginfo, sigprocmask, sigset_t, tcgetattr, tcsetattr, termios,
    winsize,
};
use std::fs::File;
use std::io::{Read, Write};
use std::mem::{ManuallyDrop, MaybeUninit};
use std::os::fd::{AsRawFd, FromRawFd, RawFd};
use std::sync::atomic::{AtomicBool, Ordering};
use std::{ffi::c_int, ptr::null_mut};

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
        .as_os_result("splice", None, None)
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

fn pump_via_splice(infd: RawFd, outfd: RawFd, pipe: &PipeFd) -> LoggedResult<()> {
    if !SHOULD_USE_SPLICE.load(Ordering::Acquire) {
        return pump_via_copy(infd, outfd);
    }

    // The pipe capacity is by default 16 pages, let's just use 65536
    let Ok(len) = splice(infd, pipe.write.as_raw_fd(), 65536_usize, 0) else {
        // If splice failed, stop using splice and fallback to userspace copy
        SHOULD_USE_SPLICE.store(false, Ordering::Release);
        return pump_via_copy(infd, outfd);
    };
    if len == 0 {
        return Ok(());
    }
    splice(pipe.read.as_raw_fd(), outfd, len as usize, 0)?;
    Ok(())
}

#[derive(Copy, Clone)]
#[repr(transparent)]
struct SignalFdInfo(signalfd_siginfo);
unsafe impl Zeroable for SignalFdInfo {}
unsafe impl Pod for SignalFdInfo {}

pub fn pump_tty(infd: i32, outfd: i32) {
    set_stdin_raw();

    let signal_fd = unsafe {
        let mut mask: sigset_t = std::mem::zeroed();
        sigemptyset(&mut mask);
        sigaddset(&mut mask, SIGWINCH);
        sigprocmask(SIG_BLOCK, &mask, null_mut())
            .check_os_err("sigprocmask", None, None)
            .log_ok();
        signalfd(-1, &mask, SFD_CLOEXEC)
    };

    resize_pty(outfd);

    let mut pfds = [
        pollfd {
            fd: if outfd > 0 { STDIN_FILENO } else { -1 },
            events: POLLIN,
            revents: 0,
        },
        pollfd {
            fd: infd,
            events: POLLIN,
            revents: 0,
        },
        pollfd {
            fd: signal_fd,
            events: POLLIN,
            revents: 0,
        },
    ];

    let Ok(pipe_fd) = make_pipe(O_CLOEXEC).log() else {
        return;
    };

    'poll: loop {
        let ready = unsafe { poll(pfds.as_mut_ptr(), pfds.len() as _, -1) };

        if ready < 0 {
            error!("poll error");
            break;
        }

        for pfd in &pfds {
            if pfd.revents & POLLIN != 0 {
                let res = if pfd.fd == STDIN_FILENO {
                    pump_via_splice(STDIN_FILENO, outfd, &pipe_fd)
                } else if pfd.fd == infd {
                    pump_via_splice(infd, STDOUT_FILENO, &pipe_fd)
                } else if pfd.fd == signal_fd {
                    resize_pty(outfd);
                    let mut info = SignalFdInfo::zeroed();
                    let mut fd = ManuallyDrop::new(unsafe { File::from_raw_fd(signal_fd) });
                    fd.read_pod(&mut info).log()
                } else {
                    log_err!()
                };
                if res.is_err() {
                    break 'poll;
                }
            } else if pfd.revents != 0 && pfd.fd == infd {
                unsafe { close(pfd.fd) };
                break 'poll;
            }
        }
    }

    restore_stdin();
    unsafe { raise(SIGWINCH) };
}
