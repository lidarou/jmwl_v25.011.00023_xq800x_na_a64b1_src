/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**   ThreadX Component                                                   */
/**                                                                       */
/**   POSIX Compliancy Wrapper (POSIX)                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  EKP DEFINITIONS                                        RELEASE        */
/*                                                                        */
/*    errno.h                                             PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the constants, structures, etc.needed to          */
/*    implement the Evacuation Kit for POSIX Users (POSIX)                */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     William E. Lamie         Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/

#ifndef _ERRNO_H
#define _ERRNO_H



#ifndef TX_POSIX_SOURCE
#ifndef errno
#define errno posix_errno
#endif
#endif

/* the POSIX standard does not impose particular values for errno.h        */
/* error codes between 200 and 1000 are not used by the Threadx wrapper    */
/* but supplied for completeness.                                          */


#ifndef E2BIG
#define E2BIG           200
#endif

#ifndef EACCES
#define EACCES          13
#endif

#ifndef EADDRINUSE
#define EADDRINUSE      201
#endif

#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL   202
#endif

#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT    203
#endif

#ifndef EAGAIN
#define EAGAIN          11
#endif

#ifndef EALREADY
#define EALREADY        204
#endif

#ifndef EBADF
#define EBADF           9
#endif

#ifndef EBADMSG
#define EBADMSG         205
#endif

#ifndef EBUSY
#define EBUSY           9999
#endif

#ifndef ECANCELED
#define ECANCELED       206
#endif

#ifndef ECHILD
#define ECHILD          207
#endif

#ifndef ECONNABORTED
#define ECONNABORTED    208
#endif

#ifndef ECONNREFUSED
#define ECONNREFUSED    209
#endif

#ifndef ECONNRESET
#define ECONNRESET      210
#endif

#ifndef EDEADLK
#define EDEADLK         3333
#endif

#ifndef EDESTADDRREQ
#define EDESTADDRREQ    211
#endif

#ifndef EDOM
#define EDOM            212
#endif

#ifndef EDQUOT
#define EDQUOT          213
#endif

#ifndef EEXIST
#define EEXIST          17      
#endif

#ifndef EFAULT
#define EFAULT          214
#endif

#ifndef EFBIG
#define EFBIG           215
#endif

#ifndef EHOSTUNREACH
#define EHOSTUNREACH    216
#endif

#ifndef EIDRM
#define EIDRM           217
#endif

#ifndef EILSEQ
#define EILSEQ          218
#endif

#ifndef EINPROGRESS
#define EINPROGRESS     219
#endif

#ifndef EINTR
#define EINTR           4
#endif

#ifndef EINVAL
#define EINVAL          22
#endif

#ifndef EIO
#define EIO             220
#endif

#ifndef EISCONN
#define EISCONN         221
#endif

#ifndef EISDIR
#define EISDIR          222
#endif

#ifndef ELOOP
#define ELOOP           223
#endif

#ifndef EMFILE
#define EMFILE          224
#endif

#ifndef EMLINK
#define EMLINK          225
#endif

#ifndef EMSGSIZE
#define EMSGSIZE        36
#endif

#ifndef EMULTIHOP
#define EMULTIHOP       226
#endif

#ifndef ENAMETOOLONG
#define ENAMETOOLONG    26
#endif

#ifndef ENETDOWN
#define ENETDOWN        227
#endif

#ifndef ENETRESET
#define ENETRESET       228
#endif

#ifndef ENETUNREACH
#define ENETUNREACH     229
#endif

#ifndef ENFILE
#define ENFILE          230
#endif

#ifndef ENOBUFS
#define ENOBUFS         231
#endif

#ifndef ENODATA
#define ENODATA         232
#endif

#ifndef ENODEV
#define ENODEV          233
#endif

#ifndef ENOENT
#define ENOENT          2
#endif

#ifndef ENOEXEC
#define ENOEXEC         234
#endif

#ifndef ENOLCK
#define ENOLCK          235
#endif

#ifndef ENOLINK
#define ENOLINK         236
#endif

#ifndef ENOMEM
#define ENOMEM          4444
#endif

#ifndef ENOMSG
#define ENOMSG          237
#endif

#ifndef ENOPROTOOPT
#define ENOPROTOOPT     238
#endif

#ifndef ENOSPC
#define ENOSPC          28
#endif

#ifndef ENOSR
#define ENOSR           239
#endif

#ifndef ENOSTR
#define ENOSTR          240
#endif

#ifndef ENOSYS
#define ENOSYS          71
#endif

#ifndef ENOTCONN
#define ENOTCONN        241
#endif

#ifndef ENOTDIR
#define ENOTDIR         242
#endif

#ifndef ENOTEMPTY
#define ENOTEMPTY       243
#endif

#ifndef ENOTSOCK
#define ENOTSOCK        244
#endif

#ifndef ENOTSUP
#define ENOTSUP         126
#endif

#ifndef ENOTTY
#define ENOTTY          245
#endif

#ifndef ENXIO
#define ENXIO           246
#endif

#ifndef EOPNOTSUPP
#define EOPNOTSUPP      247
#endif

#ifndef EOVERFLOW
#define EOVERFLOW       248
#endif

#ifndef EPERM
#define EPERM           2222
#endif

#ifndef EPIPE
#define EPIPE           249
#endif

#ifndef EPROTO
#define EPROTO          250
#endif

#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT 251
#endif

#ifndef EPROTOTYPE
#define EPROTOTYPE      252
#endif

#ifndef ERANGE
#define ERANGE          253
#endif

#ifndef EROFS
#define EROFS           254
#endif

#ifndef ESPIPE
#define ESPIPE          255
#endif

#ifndef ESRCH
#define ESRCH           3
#endif

#ifndef  ESTALE
#define ESTALE          256
#endif

#ifndef ETIME
#define ETIME           257
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT       5555
#endif

#ifndef ETXTBSY
#define ETXTBSY         258
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK     259
#endif

#ifndef EXDEV
#define EXDEV           260
#endif

#endif
