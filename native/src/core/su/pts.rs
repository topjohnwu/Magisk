use base::{
    error,
    libc::{
        POLLIN, SFD_CLOEXEC, SIG_BLOCK, SIGWINCH, STDOUT_FILENO, TCSADRAIN, TCSAFLUSH, TIOCGWINSZ,
        TIOCSWINSZ, cfmakeraw, close, poll, pollfd, raise, read, sigaddset, sigemptyset, signalfd,
        sigprocmask, sigset_t, tcsetattr, winsize, write,
    },
    libc::{STDIN_FILENO, ioctl, tcgetattr, termios},
    warn,
};
use std::ptr::null_mut;

static mut OLD_STDIN: Option<termios> = None;

pub fn get_pty_num(fd: i32) -> i32 {
    let mut pty_num = -1i32;
    if unsafe { ioctl(fd, 0x80045430u32 as _, &mut pty_num) } != 0 {
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
        if let Some(ref termios) = OLD_STDIN {
            if tcsetattr(STDIN_FILENO, TCSAFLUSH, termios) < 0
                && tcsetattr(STDIN_FILENO, TCSADRAIN, termios) < 0
            {
                warn!("Failed to restore terminal attributes");
                return false;
            }
        }
        OLD_STDIN = None;
        true
    }
}

fn resize_pty(outfd: i32) {
    let mut ws: winsize = unsafe { std::mem::zeroed() };
    if unsafe { ioctl(STDIN_FILENO, TIOCGWINSZ, &mut ws) } >= 0 {
        unsafe { ioctl(outfd, TIOCSWINSZ, &ws) };
    }
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

    let mut buf = [0u8; 4096];
    'poll: loop {
        let ready = unsafe { poll(pfds.as_mut_ptr(), pfds.len() as _, -1) };

        if ready < 0 {
            error!("poll error");
            break;
        }

        for pfd in &pfds {
            if pfd.revents & POLLIN != 0 {
                let n = unsafe { read(pfd.fd, buf.as_mut_ptr() as _, buf.len()) };
                if n <= 0 {
                    error!("read error");
                    break 'poll;
                }
                if pfd.fd == STDIN_FILENO {
                    unsafe { write(outfd, buf.as_ptr() as _, n as _) };
                } else if pfd.fd == infd {
                    unsafe { write(STDOUT_FILENO, buf.as_ptr() as _, n as _) };
                } else if pfd.fd == sfd {
                    resize_pty(outfd);
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
