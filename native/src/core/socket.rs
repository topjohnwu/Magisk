use base::{libc, warn, ReadExt, ResultExt, WriteExt};
use bytemuck::{bytes_of, bytes_of_mut, Pod, Zeroable};
use std::io;
use std::io::{ErrorKind, IoSlice, IoSliceMut, Read, Write};
use std::mem::ManuallyDrop;
use std::os::fd::{FromRawFd, IntoRawFd, OwnedFd, RawFd};
use std::os::unix::net::{AncillaryData, SocketAncillary, UnixStream};

pub trait IpcRead {
    fn ipc_read_int(&mut self) -> io::Result<i32>;
    fn ipc_read_string(&mut self) -> io::Result<String>;
    fn ipc_read_vec<E: Pod>(&mut self) -> io::Result<Vec<E>>;
}

impl<T: Read> IpcRead for T {
    fn ipc_read_int(&mut self) -> io::Result<i32> {
        let mut val: i32 = 0;
        self.read_pod(&mut val)?;
        Ok(val)
    }

    fn ipc_read_string(&mut self) -> io::Result<String> {
        let len = self.ipc_read_int()?;
        let mut val = "".to_string();
        self.take(len as u64).read_to_string(&mut val)?;
        Ok(val)
    }

    fn ipc_read_vec<E: Pod>(&mut self) -> io::Result<Vec<E>> {
        let len = self.ipc_read_int()? as usize;
        let mut vec = Vec::new();
        let mut val: E = Zeroable::zeroed();
        for _ in 0..len {
            self.read_pod(&mut val)?;
            vec.push(val);
        }
        Ok(vec)
    }
}

pub trait IpcWrite {
    fn ipc_write_int(&mut self, val: i32) -> io::Result<()>;
    fn ipc_write_string(&mut self, val: &str) -> io::Result<()>;
}

impl<T: Write> IpcWrite for T {
    fn ipc_write_int(&mut self, val: i32) -> io::Result<()> {
        self.write_pod(&val)
    }

    fn ipc_write_string(&mut self, val: &str) -> io::Result<()> {
        self.ipc_write_int(val.len() as i32)?;
        self.write_all(val.as_bytes())
    }
}

pub trait UnixSocketExt {
    fn send_fds(&mut self, fd: &[RawFd]) -> io::Result<()>;
    fn recv_fd(&mut self) -> io::Result<Option<OwnedFd>>;
    fn recv_fds(&mut self) -> io::Result<Vec<OwnedFd>>;
}

impl UnixSocketExt for UnixStream {
    fn send_fds(&mut self, fds: &[RawFd]) -> io::Result<()> {
        match fds.len() {
            0 => self.ipc_write_int(-1)?,
            len => {
                // 4k buffer is reasonable enough
                let mut buf = [0u8; 4096];
                let mut ancillary = SocketAncillary::new(&mut buf);
                if !ancillary.add_fds(fds) {
                    return Err(ErrorKind::OutOfMemory.into());
                }
                let fd_count = len as i32;
                let iov = IoSlice::new(bytes_of(&fd_count));
                self.send_vectored_with_ancillary(&[iov], &mut ancillary)?;
            }
        };
        Ok(())
    }

    fn recv_fd(&mut self) -> io::Result<Option<OwnedFd>> {
        let mut fd_count = 0;
        self.peek(bytes_of_mut(&mut fd_count))?;
        if fd_count < 1 {
            return Ok(None);
        }

        // 4k buffer is reasonable enough
        let mut buf = [0u8; 4096];
        let mut ancillary = SocketAncillary::new(&mut buf);
        let iov = IoSliceMut::new(bytes_of_mut(&mut fd_count));
        self.recv_vectored_with_ancillary(&mut [iov], &mut ancillary)?;
        for msg in ancillary.messages().flatten() {
            if let AncillaryData::ScmRights(mut scm_rights) = msg {
                // We only want the first one
                let fd = if let Some(fd) = scm_rights.next() {
                    unsafe { OwnedFd::from_raw_fd(fd) }
                } else {
                    return Ok(None);
                };
                // Close all others
                for fd in scm_rights {
                    unsafe { libc::close(fd) };
                }
                return Ok(Some(fd));
            }
        }
        Ok(None)
    }

    fn recv_fds(&mut self) -> io::Result<Vec<OwnedFd>> {
        let mut fd_count = 0;
        // 4k buffer is reasonable enough
        let mut buf = [0u8; 4096];
        let mut ancillary = SocketAncillary::new(&mut buf);
        let iov = IoSliceMut::new(bytes_of_mut(&mut fd_count));
        self.recv_vectored_with_ancillary(&mut [iov], &mut ancillary)?;
        let mut fds: Vec<OwnedFd> = Vec::new();
        for msg in ancillary.messages().flatten() {
            if let AncillaryData::ScmRights(scm_rights) = msg {
                fds = scm_rights
                    .map(|fd| unsafe { OwnedFd::from_raw_fd(fd) })
                    .collect();
            }
        }
        if fd_count as usize != fds.len() {
            warn!(
                "Received unexpected number of fds: expected={} actual={}",
                fd_count,
                fds.len()
            );
        }
        Ok(fds)
    }
}

pub fn send_fd(socket: RawFd, fd: RawFd) -> bool {
    let mut socket = ManuallyDrop::new(unsafe { UnixStream::from_raw_fd(socket) });
    socket.send_fds(&[fd]).log().is_ok()
}

pub fn send_fds(socket: RawFd, fds: &[RawFd]) -> bool {
    let mut socket = ManuallyDrop::new(unsafe { UnixStream::from_raw_fd(socket) });
    socket.send_fds(fds).log().is_ok()
}

pub fn recv_fd(socket: RawFd) -> RawFd {
    let mut socket = ManuallyDrop::new(unsafe { UnixStream::from_raw_fd(socket) });
    socket
        .recv_fd()
        .log()
        .unwrap_or(None)
        .map_or(-1, IntoRawFd::into_raw_fd)
}

pub fn recv_fds(socket: RawFd) -> Vec<RawFd> {
    let mut socket = ManuallyDrop::new(unsafe { UnixStream::from_raw_fd(socket) });
    let fds = socket.recv_fds().log().unwrap_or(Vec::new());
    // SAFETY: OwnedFd and RawFd has the same layout
    unsafe { std::mem::transmute(fds) }
}
