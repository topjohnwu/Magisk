use base::{error, libc, warn};
use libc::{
    POLLIN, SFD_CLOEXEC, SIG_BLOCK, SIGWINCH, STDIN_FILENO, STDOUT_FILENO, TCSADRAIN, TCSAFLUSH,
    TIOCGWINSZ, TIOCSWINSZ, cfmakeraw, close, pipe, poll, pollfd, raise, read, sigaddset,
    sigemptyset, signalfd, signalfd_siginfo, sigprocmask, sigset_t, splice, tcgetattr, tcsetattr,
    termios, winsize,
};
use std::{ffi::c_int, mem::MaybeUninit, ptr::null_mut};

static mut OLD_STDIN: Option<termios> = None;
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

fn pump_via_pipe(infd: i32, outfd: i32, pipe: &[c_int; 2]) -> bool {
    // usize::MAX will EINVAL in some kernels, use i32::MAX in case
    let s = unsafe { splice(infd, null_mut(), pipe[1], null_mut(), i32::MAX as _, 0) };
    if s < 0 {
        error!("splice error");
        return false;
    }
    if s == 0 {
        return true;
    }
    let s = unsafe { splice(pipe[0], null_mut(), outfd, null_mut(), s as usize, 0) };
    if s < 0 {
        error!("splice error");
        return false;
    }
    true
}

pub fn pump_tty(infd: i32, outfd: i32) {
    set_stdin_raw();

    let sfd = unsafe {
        let mut mask: sigset_t = std::mem::zeroed();
        sigemptyset(&mut mask);
        sigaddset(&mut mask, SIGWINCH);
        if sigprocmask(SIG_BLOCK, &mask, null_mut()) < 0 {
            error!("sigprocmask");
        }
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
            fd: sfd,
            events: POLLIN,
            revents: 0,
        },
    ];

    let mut p: [c_int; 2] = [0; 2];
    if unsafe { pipe(&mut p as *mut c_int) } < 0 {
        error!("pipe error");
        return;
    }
    'poll: loop {
        let ready = unsafe { poll(pfds.as_mut_ptr(), pfds.len() as _, -1) };

        if ready < 0 {
            error!("poll error");
            break;
        }

        for pfd in &pfds {
            if pfd.revents & POLLIN != 0 {
                let res = if pfd.fd == STDIN_FILENO {
                    pump_via_pipe(pfd.fd, outfd, &p)
                } else if pfd.fd == infd {
                    pump_via_pipe(pfd.fd, STDOUT_FILENO, &p)
                } else if pfd.fd == sfd {
                    resize_pty(outfd);
                    let mut buf = [MaybeUninit::<u8>::uninit(); size_of::<signalfd_siginfo>()];
                    if unsafe { read(pfd.fd, buf.as_mut_ptr() as *mut _, buf.len()) } < 0 {
                        error!("read error");
                        false
                    } else {
                        true
                    }
                } else {
                    false
                };
                if !res {
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
