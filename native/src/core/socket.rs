use base::{ReadExt, ResultExt, WriteExt, libc, warn};
use bytemuck::{Zeroable, bytes_of, bytes_of_mut};
use std::io;
use std::io::{ErrorKind, IoSlice, IoSliceMut, Read, Write};
use std::mem::ManuallyDrop;
use std::os::fd::{FromRawFd, IntoRawFd, OwnedFd, RawFd};
use std::os::unix::net::{AncillaryData, SocketAncillary, UnixStream};

pub trait Encodable {
    fn encoded_len(&self) -> usize;
    fn encode(&self, w: &mut impl Write) -> io::Result<()>;
}

pub trait Decodable: Sized + Encodable {
    fn decode(r: &mut impl Read) -> io::Result<Self>;
}

macro_rules! impl_pod_encodable {
    ($($t:ty)*) => ($(
        impl Encodable for $t {
            #[inline(always)]
            fn encoded_len(&self) -> usize {
                size_of::<Self>()
            }

            #[inline(always)]
            fn encode(&self, w: &mut impl Write) -> io::Result<()> {
                w.write_pod(self)
            }
        }
        impl Decodable for $t {
            #[inline(always)]
            fn decode(r: &mut impl Read) -> io::Result<Self> {
                let mut val = Self::zeroed();
                r.read_pod(&mut val)?;
                Ok(val)
            }
        }
    )*)
}

impl_pod_encodable! { u8 u32 i32 usize }

impl Encodable for bool {
    #[inline(always)]
    fn encoded_len(&self) -> usize {
        size_of::<u8>()
    }

    #[inline(always)]
    fn encode(&self, w: &mut impl Write) -> io::Result<()> {
        match *self {
            true => 1u8.encode(w),
            false => 0u8.encode(w),
        }
    }
}

impl Decodable for bool {
    #[inline(always)]
    fn decode(r: &mut impl Read) -> io::Result<Self> {
        Ok(u8::decode(r)? != 0)
    }
}

impl<T: Decodable> Encodable for Vec<T> {
    fn encoded_len(&self) -> usize {
        size_of::<i32>() + size_of::<T>() * self.len()
    }

    fn encode(&self, w: &mut impl Write) -> io::Result<()> {
        (self.len() as i32).encode(w)?;
        self.iter().try_for_each(|e| e.encode(w))
    }
}

impl<T: Decodable> Decodable for Vec<T> {
    fn decode(r: &mut impl Read) -> io::Result<Self> {
        let len = i32::decode(r)?;
        let mut val = Vec::with_capacity(len as usize);
        for _ in 0..len {
            val.push(T::decode(r)?);
        }
        Ok(val)
    }
}

impl Encodable for str {
    fn encoded_len(&self) -> usize {
        size_of::<i32>() + self.len()
    }

    fn encode(&self, w: &mut impl Write) -> io::Result<()> {
        (self.len() as i32).encode(w)?;
        w.write_all(self.as_bytes())
    }
}

impl Encodable for String {
    fn encoded_len(&self) -> usize {
        self.as_str().encoded_len()
    }

    fn encode(&self, w: &mut impl Write) -> io::Result<()> {
        self.as_str().encode(w)
    }
}

impl Decodable for String {
    fn decode(r: &mut impl Read) -> io::Result<String> {
        let len = i32::decode(r)?;
        let mut val = String::with_capacity(len as usize);
        r.take(len as u64).read_to_string(&mut val)?;
        Ok(val)
    }
}

pub trait IpcRead {
    fn read_decodable<E: Decodable>(&mut self) -> io::Result<E>;
}

impl<T: Read> IpcRead for T {
    #[inline(always)]
    fn read_decodable<E: Decodable>(&mut self) -> io::Result<E> {
        E::decode(self)
    }
}

pub trait IpcWrite {
    fn write_encodable<E: Encodable + ?Sized>(&mut self, val: &E) -> io::Result<()>;
}

impl<T: Write> IpcWrite for T {
    #[inline(always)]
    fn write_encodable<E: Encodable + ?Sized>(&mut self, val: &E) -> io::Result<()> {
        val.encode(self)
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
            0 => self.write_pod(&0)?,
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
            // Actually consume the data
            self.read_pod(&mut fd_count)?;
            return Ok(None);
        }
        if fd_count > 1 {
            warn!(
                "Received unexpected number of fds: expected=1 actual={}",
                fd_count
            );
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
    if fd < 0 {
        socket.send_fds(&[]).log().is_ok()
    } else {
        socket.send_fds(&[fd]).log().is_ok()
    }
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
