// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <cerrno>

#include <cerrno>

#ifndef E2BIG
#error E2BIG not defined
#endif

#ifndef EACCES
#error EACCES not defined
#endif

#ifndef EACCES
#error EACCES not defined
#endif

#ifndef EADDRINUSE
#error EADDRINUSE not defined
#endif

#ifndef EADDRNOTAVAIL
#error EADDRNOTAVAIL not defined
#endif

#ifndef EAFNOSUPPORT
#error EAFNOSUPPORT not defined
#endif

#ifndef EAGAIN
#error EAGAIN not defined
#endif

#ifndef EALREADY
#error EALREADY not defined
#endif

#ifndef EBADF
#error EBADF not defined
#endif

#ifndef EBADMSG
#error EBADMSG not defined
#endif

#ifndef EBUSY
#error EBUSY not defined
#endif

#ifndef ECANCELED
#error ECANCELED not defined
#endif

#ifndef ECHILD
#error ECHILD not defined
#endif

#ifndef ECONNABORTED
#error ECONNABORTED not defined
#endif

#ifndef ECONNREFUSED
#error ECONNREFUSED not defined
#endif

#ifndef ECONNRESET
#error ECONNRESET not defined
#endif

#ifndef EDEADLK
#error EDEADLK not defined
#endif

#ifndef EDESTADDRREQ
#error EDESTADDRREQ not defined
#endif

#ifndef EDOM
#error EDOM not defined
#endif

#ifndef EEXIST
#error EEXIST not defined
#endif

#ifndef EFAULT
#error EFAULT not defined
#endif

#ifndef EFBIG
#error EFBIG not defined
#endif

#ifndef EHOSTUNREACH
#error EHOSTUNREACH not defined
#endif

#ifndef EIDRM
#error EIDRM not defined
#endif

#ifndef EILSEQ
#error EILSEQ not defined
#endif

#ifndef EINPROGRESS
#error EINPROGRESS not defined
#endif

#ifndef EINTR
#error EINTR not defined
#endif

#ifndef EINVAL
#error EINVAL not defined
#endif

#ifndef EIO
#error EIO not defined
#endif

#ifndef EISCONN
#error EISCONN not defined
#endif

#ifndef EISDIR
#error EISDIR not defined
#endif

#ifndef ELOOP
#error ELOOP not defined
#endif

#ifndef EMFILE
#error EMFILE not defined
#endif

#ifndef EMLINK
#error EMLINK not defined
#endif

#ifndef EMSGSIZE
#error EMSGSIZE not defined
#endif

#ifndef ENAMETOOLONG
#error ENAMETOOLONG not defined
#endif

#ifndef ENETDOWN
#error ENETDOWN not defined
#endif

#ifndef ENETRESET
#error ENETRESET not defined
#endif

#ifndef ENETUNREACH
#error ENETUNREACH not defined
#endif

#ifndef ENFILE
#error ENFILE not defined
#endif

#ifndef ENOBUFS
#error ENOBUFS not defined
#endif

#if (defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1)
#ifndef ENODATA
#error ENODATA not defined
#endif
#endif

#ifndef ENODEV
#error ENODEV not defined
#endif

#ifndef ENOENT
#error ENOENT not defined
#endif

#ifndef ENOEXEC
#error ENOEXEC not defined
#endif

#ifndef ENOLCK
#error ENOLCK not defined
#endif

#ifndef ENOLINK
#error ENOLINK not defined
#endif

#ifndef ENOMEM
#error ENOMEM not defined
#endif

#ifndef ENOMSG
#error ENOMSG not defined
#endif

#ifndef ENOPROTOOPT
#error ENOPROTOOPT not defined
#endif

#ifndef ENOSPC
#error ENOSPC not defined
#endif

#if (defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1)
#ifndef ENOSR
#error ENOSR not defined
#endif
#endif

#if (defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1)
#ifndef ENOSTR
#error ENOSTR not defined
#endif
#endif

#ifndef ENOSYS
#error ENOSYS not defined
#endif

#ifndef ENOTCONN
#error ENOTCONN not defined
#endif

#ifndef ENOTDIR
#error ENOTDIR not defined
#endif

#ifndef ENOTEMPTY
#error ENOTEMPTY not defined
#endif

#ifndef ENOTRECOVERABLE
#error ENOTRECOVERABLE not defined
#endif

#ifndef ENOTSOCK
#error ENOTSOCK not defined
#endif

#ifndef ENOTSUP
#error ENOTSUP not defined
#endif

#ifndef ENOTTY
#error ENOTTY not defined
#endif

#ifndef ENXIO
#error ENXIO not defined
#endif

#ifndef EOPNOTSUPP
#error EOPNOTSUPP not defined
#endif

#ifndef EOVERFLOW
#error EOVERFLOW not defined
#endif

#ifndef EOWNERDEAD
#error EOWNERDEAD not defined
#endif

#ifndef EPERM
#error EPERM not defined
#endif

#ifndef EPIPE
#error EPIPE not defined
#endif

#ifndef EPROTO
#error EPROTO not defined
#endif

#ifndef EPROTONOSUPPORT
#error EPROTONOSUPPORT not defined
#endif

#ifndef EPROTOTYPE
#error EPROTOTYPE not defined
#endif

#ifndef ERANGE
#error ERANGE not defined
#endif

#ifndef EROFS
#error EROFS not defined
#endif

#ifndef ESPIPE
#error ESPIPE not defined
#endif

#ifndef ESRCH
#error ESRCH not defined
#endif

#if (defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1)
#ifndef ETIME
#error ETIME not defined
#endif
#endif

#ifndef ETIMEDOUT
#error ETIMEDOUT not defined
#endif

#ifndef ETXTBSY
#error ETXTBSY not defined
#endif

#ifndef EWOULDBLOCK
#error EWOULDBLOCK not defined
#endif

#ifndef EXDEV
#error EXDEV not defined
#endif

#ifndef errno
#error errno not defined
#endif

int main()
{
}
