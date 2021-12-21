//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>

// enum errc {...}

#include <system_error>

int main()
{
    static_assert(static_cast<int>(std::errc::address_family_not_supported) == EAFNOSUPPORT, "");
    static_assert(static_cast<int>(std::errc::address_in_use) == EADDRINUSE, "");
    static_assert(static_cast<int>(std::errc::address_not_available) == EADDRNOTAVAIL, "");
    static_assert(static_cast<int>(std::errc::already_connected) == EISCONN, "");
    static_assert(static_cast<int>(std::errc::argument_list_too_long) == E2BIG, "");
    static_assert(static_cast<int>(std::errc::argument_out_of_domain) == EDOM, "");
    static_assert(static_cast<int>(std::errc::bad_address) == EFAULT, "");
    static_assert(static_cast<int>(std::errc::bad_file_descriptor) == EBADF, "");
    static_assert(static_cast<int>(std::errc::bad_message) == EBADMSG, "");
    static_assert(static_cast<int>(std::errc::broken_pipe) == EPIPE, "");
    static_assert(static_cast<int>(std::errc::connection_aborted) == ECONNABORTED, "");
    static_assert(static_cast<int>(std::errc::connection_already_in_progress) == EALREADY, "");
    static_assert(static_cast<int>(std::errc::connection_refused) == ECONNREFUSED, "");
    static_assert(static_cast<int>(std::errc::connection_reset) == ECONNRESET, "");
    static_assert(static_cast<int>(std::errc::cross_device_link) == EXDEV, "");
    static_assert(static_cast<int>(std::errc::destination_address_required) == EDESTADDRREQ, "");
    static_assert(static_cast<int>(std::errc::device_or_resource_busy) == EBUSY, "");
    static_assert(static_cast<int>(std::errc::directory_not_empty) == ENOTEMPTY, "");
    static_assert(static_cast<int>(std::errc::executable_format_error) == ENOEXEC, "");
    static_assert(static_cast<int>(std::errc::file_exists) == EEXIST, "");
    static_assert(static_cast<int>(std::errc::file_too_large) == EFBIG, "");
    static_assert(static_cast<int>(std::errc::filename_too_long) == ENAMETOOLONG, "");
    static_assert(static_cast<int>(std::errc::function_not_supported) == ENOSYS, "");
    static_assert(static_cast<int>(std::errc::host_unreachable) == EHOSTUNREACH, "");
    static_assert(static_cast<int>(std::errc::identifier_removed) == EIDRM, "");
    static_assert(static_cast<int>(std::errc::illegal_byte_sequence) == EILSEQ, "");
    static_assert(static_cast<int>(std::errc::inappropriate_io_control_operation) == ENOTTY, "");
    static_assert(static_cast<int>(std::errc::interrupted) == EINTR, "");
    static_assert(static_cast<int>(std::errc::invalid_argument) == EINVAL, "");
    static_assert(static_cast<int>(std::errc::invalid_seek) == ESPIPE, "");
    static_assert(static_cast<int>(std::errc::io_error) == EIO, "");
    static_assert(static_cast<int>(std::errc::is_a_directory) == EISDIR, "");
    static_assert(static_cast<int>(std::errc::message_size) == EMSGSIZE, "");
    static_assert(static_cast<int>(std::errc::network_down) == ENETDOWN, "");
    static_assert(static_cast<int>(std::errc::network_reset) == ENETRESET, "");
    static_assert(static_cast<int>(std::errc::network_unreachable) == ENETUNREACH, "");
    static_assert(static_cast<int>(std::errc::no_buffer_space) == ENOBUFS, "");
    static_assert(static_cast<int>(std::errc::no_child_process) == ECHILD, "");
    static_assert(static_cast<int>(std::errc::no_link) == ENOLINK, "");
    static_assert(static_cast<int>(std::errc::no_lock_available) == ENOLCK, "");
#if (defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1)
    static_assert(static_cast<int>(std::errc::no_message_available) == ENODATA, "");
#endif
    static_assert(static_cast<int>(std::errc::no_message) == ENOMSG, "");
    static_assert(static_cast<int>(std::errc::no_protocol_option) == ENOPROTOOPT, "");
    static_assert(static_cast<int>(std::errc::no_space_on_device) == ENOSPC, "");
#if (defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1)
    static_assert(static_cast<int>(std::errc::no_stream_resources) == ENOSR, "");
#endif
    static_assert(static_cast<int>(std::errc::no_such_device_or_address) == ENXIO, "");
    static_assert(static_cast<int>(std::errc::no_such_device) == ENODEV, "");
    static_assert(static_cast<int>(std::errc::no_such_file_or_directory) == ENOENT, "");
    static_assert(static_cast<int>(std::errc::no_such_process) == ESRCH, "");
    static_assert(static_cast<int>(std::errc::not_a_directory) == ENOTDIR, "");
    static_assert(static_cast<int>(std::errc::not_a_socket) == ENOTSOCK, "");
#if (defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1)
    static_assert(static_cast<int>(std::errc::not_a_stream) == ENOSTR, "");
#endif
    static_assert(static_cast<int>(std::errc::not_connected) == ENOTCONN, "");
    static_assert(static_cast<int>(std::errc::not_enough_memory) == ENOMEM, "");
    static_assert(static_cast<int>(std::errc::not_supported) == ENOTSUP, "");
    static_assert(static_cast<int>(std::errc::operation_canceled) == ECANCELED, "");
    static_assert(static_cast<int>(std::errc::operation_in_progress) == EINPROGRESS, "");
    static_assert(static_cast<int>(std::errc::operation_not_permitted) == EPERM, "");
    static_assert(static_cast<int>(std::errc::operation_not_supported) == EOPNOTSUPP, "");
    static_assert(static_cast<int>(std::errc::operation_would_block) == EWOULDBLOCK, "");
    static_assert(static_cast<int>(std::errc::owner_dead) == EOWNERDEAD, "");
    static_assert(static_cast<int>(std::errc::permission_denied) == EACCES, "");
    static_assert(static_cast<int>(std::errc::protocol_error) == EPROTO, "");
    static_assert(static_cast<int>(std::errc::protocol_not_supported) == EPROTONOSUPPORT, "");
    static_assert(static_cast<int>(std::errc::read_only_file_system) == EROFS, "");
    static_assert(static_cast<int>(std::errc::resource_deadlock_would_occur) == EDEADLK, "");
    static_assert(static_cast<int>(std::errc::resource_unavailable_try_again) == EAGAIN, "");
    static_assert(static_cast<int>(std::errc::result_out_of_range) == ERANGE, "");
    static_assert(static_cast<int>(std::errc::state_not_recoverable) == ENOTRECOVERABLE, "");
#if (defined(_XOPEN_STREAMS) && _XOPEN_STREAMS != -1)
    static_assert(static_cast<int>(std::errc::stream_timeout) == ETIME, "");
#endif
    static_assert(static_cast<int>(std::errc::text_file_busy) == ETXTBSY, "");
    static_assert(static_cast<int>(std::errc::timed_out) == ETIMEDOUT, "");
    static_assert(static_cast<int>(std::errc::too_many_files_open_in_system) == ENFILE, "");
    static_assert(static_cast<int>(std::errc::too_many_files_open) == EMFILE, "");
    static_assert(static_cast<int>(std::errc::too_many_links) == EMLINK, "");
    static_assert(static_cast<int>(std::errc::too_many_symbolic_link_levels) == ELOOP, "");
    static_assert(static_cast<int>(std::errc::value_too_large) == EOVERFLOW, "");
    static_assert(static_cast<int>(std::errc::wrong_protocol_type) == EPROTOTYPE, "");
}
