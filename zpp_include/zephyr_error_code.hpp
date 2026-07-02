// Copyright 2025 Haute école d'ingénierie et d'architecture de Fribourg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
 * @file zephyr_error_code.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration of zephyr error codes as an enum class
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// zephyr sdk
#include <zephyr/kernel.h>

namespace zpp_lib {

///
/// @brief scoped enum with zephyr error codes
///
enum class ZephyrErrorCode : uint8_t {
  Ok             = 0,                ///< 0 is used for no error
  Perm           = EPERM,            ///< Not owner
  Noent          = ENOENT,           ///< No such file or directory
  Srch           = ESRCH,            ///< No such context
  Intr           = EINTR,            ///< Interrupted system call
  Io             = EIO,              ///< I/O error
  Nxio           = ENXIO,            ///< No such device or address
  E2big          = E2BIG,            ///< Arg list too long
  Noexec         = ENOEXEC,          ///< Exec format error
  Badf           = EBADF,            ///< Bad file number
  Child          = ECHILD,           ///< No children
  Again          = EAGAIN,           ///< No more contexts
  Nomem          = ENOMEM,           ///< Not enough space
  Acces          = EACCES,           ///< Permission denied
  Fault          = EFAULT,           ///< Bad address
  Notblk         = ENOTBLK,          ///< Block device required
  Busy           = EBUSY,            ///< Mount device busy
  Exits          = EEXIST,           ///< File exists
  Xdev           = EXDEV,            ///< Cross-device link
  Nodev          = ENODEV,           ///< No such device
  Notdir         = ENOTDIR,          ///< Not a directory
  Isdir          = EISDIR,           ///< Is a directory
  Inval          = EINVAL,           ///< Invalid argument
  Nfile          = ENFILE,           ///< File table overflow
  Mfile          = EMFILE,           ///< Too many open files
  Notty          = ENOTTY,           ///< Not a typewriter
  Txtbsy         = ETXTBSY,          ///< Text file busy
  Fbig           = EFBIG,            ///< File too large
  Nospc          = ENOSPC,           ///< No space left on device
  Spipe          = ESPIPE,           ///< Illegal seek
  Rofs           = EROFS,            ///< Read-only file system
  Mlink          = EMLINK,           ///< Too many links
  Pipe           = EPIPE,            ///< Broken pipe
  Dom            = EDOM,             ///< Argument too large
  Range          = ERANGE,           ///< Result too large
  Nomsg          = ENOMSG,           ///< Unexpected message type
  Deadlk         = EDEADLK,          ///< Resource deadlock avoided
  Nolck          = ENOLCK,           ///< No locks available
  Nostr          = ENOSTR,           ///< STREAMS device required
  Nodata         = ENODATA,          ///< Missing expected message data
  Time           = ETIME,            ///< STREAMS timeout occurred
  Nosr           = ENOSR,            ///< No stream resources
  Proto          = EPROTO,           ///< Generic STREAMS error
  Badmsg         = EBADMSG,          ///< Invalid STREAMS message
  Nosys          = ENOSYS,           ///< Function not implemented
  Notempty       = ENOTEMPTY,        ///< Directory not empty
  Nametoolong    = ENAMETOOLONG,     ///< File name too long
  Loop           = ELOOP,            ///< Too many levels of symbolic links
  Opnotsupp      = EOPNOTSUPP,       ///< Operation not supported on socket
  Pfnosupport    = EPFNOSUPPORT,     ///< Protocol family not supported
  Connreset      = ECONNRESET,       ///< Connection reset by peer
  Nobufs         = ENOBUFS,          ///< No buffer space available
  Afnosupport    = EAFNOSUPPORT,     ///< Addr family not supported
  Prototype      = EPROTOTYPE,       ///< Protocol wrong type for socket
  Notsock        = ENOTSOCK,         ///< Socket operation on non-socket
  Noprotoopt     = ENOPROTOOPT,      ///< Protocol not available
  Shutdown       = ESHUTDOWN,        ///< Can’t send after socket shutdown
  Connrefused    = ECONNREFUSED,     ///< Connection refused
  Addrinuse      = EADDRINUSE,       ///< Address already in use
  Connaborted    = ECONNABORTED,     ///< Software caused connection abort
  Netunreach     = ENETUNREACH,      ///< Network is unreachable
  Netdown        = ENETDOWN,         ///< Network is down
  Timeout        = ETIMEDOUT,        ///< Connection timed out
  Hostdown       = EHOSTDOWN,        ///< Host is down
  Hostunreach    = EHOSTUNREACH,     ///< No route to host
  Inprogress     = EINPROGRESS,      ///< Operation now in progress
  Already        = EALREADY,         ///< Operation already in progress
  Destaddrreq    = EDESTADDRREQ,     ///< Destination address required
  Msgsize        = EMSGSIZE,         ///< Message size
  Protonosupport = EPROTONOSUPPORT,  ///< Protocol not supported
  Socktnosupport = ESOCKTNOSUPPORT,  ///< Socket type not supported
  Addrnotavail   = EADDRNOTAVAIL,    ///< Can’t assign requested address
  Netreset       = ENETRESET,        ///< Network dropped connection on reset
  Isconn         = EISCONN,          ///< Socket is already connected
  Notconn        = ENOTCONN,         ///< Socket is not connected
  Toomanyrefs    = ETOOMANYREFS,     ///< Too many references: can’t splice
  Notsup         = ENOTSUP,          ///< Unsupported value
  Ilseq          = EILSEQ,           ///< Illegal byte sequence
  Overflow       = EOVERFLOW,        ///< Value overflow
  Canceled       = ECANCELED,        ///< Operation canceled
  WouldBlock     = EWOULDBLOCK,      ///< Operation would block
};

constexpr ZephyrErrorCode zephyr_to_zpp_error_code(uint32_t v) noexcept {
  // This function will be called only with existing Zephyr error codes
  // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
  return static_cast<ZephyrErrorCode>(v);
}

}  // namespace zpp_lib
