/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the common error numbers.
 */

#ifndef __AIC_ERRNO_H__
#define __AIC_ERRNO_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#define AIC_SUCCESS                 0      /* Success */

#define AIC_EPERM                   -1     /* Not owner */
#define AIC_ENOENT                  -2     /* No such file or directory */
#define AIC_ESRCH                   -3     /* No such context */
#define AIC_EINTR                   -4     /* Interrupted system call */
#define AIC_EIO                     -5     /* I/O error */
#define AIC_ENXIO                   -6     /* No such device or address */
#define AIC_E2BIG                   -7     /* Arg list too long */
#define AIC_ENOEXEC                 -8     /* Exec format error */
#define AIC_EBADF                   -9     /* Bad file number */
#define AIC_ECHILD                  -10    /* No children */
#define AIC_EAGAIN                  -11    /* No more contexts */
#define AIC_ENOMEM                  -12    /* Not enough memory */
#define AIC_EACCES                  -13    /* Permission denied */
#define AIC_EFAULT                  -14    /* Bad address */
#define AIC_ENOTBLK                 -15    /* Block device required */
#define AIC_EBUSY                   -16    /* Mount device busy */
#define AIC_EEXIST                  -17    /* File exists */
#define AIC_EXDEV                   -18    /* Cross-device link */
#define AIC_ENODEV                  -19    /* No such device */
#define AIC_ENOTDIR                 -20    /* Not a directory */
#define AIC_EISDIR                  -21    /* Is a directory */
#define AIC_EINVAL                  -22    /* Invalid argument */
#define AIC_ENFILE                  -23    /* File table overflow */
#define AIC_EMFILE                  -24    /* Too many open files */
#define AIC_ENOTTY                  -25    /* Not a typewriter */
#define AIC_ETXTBSY                 -26    /* Text file busy */
#define AIC_EFBIG                   -27    /* File too large */
#define AIC_ENOSPC                  -28    /* No space left on device */
#define AIC_ESPIPE                  -29    /* Illegal seek */
#define AIC_EROFS                   -30    /* Read-only file system */
#define AIC_EMLINK                  -31    /* Too many links */
#define AIC_EPIPE                   -32    /* Broken pipe */
#define AIC_EDOM                    -33    /* Argument too large */
#define AIC_ERANGE                  -34    /* Result too large */
#define AIC_ENOMSG                  -35    /* Unexpected message type */
#define AIC_EDEADLK                 -45    /* Resource deadlock avoided */
#define AIC_ENOLCK                  -46    /* No locks available */
#define AIC_ENOSTR                  -60    /* STREAMS device required */
#define AIC_ENODATA                 -61    /* Missing expected message data */
#define AIC_ETIME                   -62    /* STREAMS timeout occurred */
#define AIC_ENOSR                   -63    /* Insufficient memory */
#define AIC_EPROTO                  -71    /* Generic STREAMS error */
#define AIC_EBADMSG                 -77    /* Invalid STREAMS message */
#define AIC_ENOSYS                  -88    /* Function not implemented */
#define AIC_ENOTEMPTY               -90    /* Directory not empty */
#define AIC_ENAMETOOLONG            -91    /* File name too long */
#define AIC_ELOOP                   -92    /* Too many levels of symbolic links */
#define AIC_EOPNOTSUPP              -95    /* Operation not supported on socket */
#define AIC_EPFNOSUPPORT            -96    /* Protocol family not supported */
#define AIC_ECONNRESET              -104   /* Connection reset by peer */
#define AIC_ENOBUFS                 -105   /* No buffer space available */
#define AIC_EAFNOSUPPORT            -106   /* Addr family not supported */
#define AIC_EPROTOTYPE              -107   /* Protocol wrong type for socket */
#define AIC_ENOTSOCK                -108   /* Socket operation on non-socket */
#define AIC_ENOPROTOOPT             -109   /* Protocol not available */
#define AIC_ESHUTDOWN               -110   /* Can't send after socket shutdown */
#define AIC_ECONNREFUSED            -111   /* Connection refused */
#define AIC_EADDRINUSE              -112   /* Address already in use */
#define AIC_ECONNABORTED            -113   /* Software caused connection abort */
#define AIC_ENETUNREACH             -114   /* Network is unreachable */
#define AIC_ENETDOWN                -115   /* Network is down */
#define AIC_ETIMEDOUT               -116   /* Connection timed out */
#define AIC_EHOSTDOWN               -117   /* Host is down */
#define AIC_EHOSTUNREACH            -118   /* No route to host */
#define AIC_EINPROGRESS             -119   /* Operation now in progress */
#define AIC_EALREADY                -120   /* Operation already in progress */
#define AIC_EDESTADDRREQ            -121   /* Destination address required */
#define AIC_EMSGSIZE                -122   /* Message size */
#define AIC_EPROTONOSUPPORT         -123   /* Protocol not supported */
#define AIC_ESOCKTNOSUPPORT         -124   /* Socket type not supported */
#define AIC_EADDRNOTAVAIL           -125   /* Can't assign requested address */
#define AIC_ENETRESET               -126   /* Network dropped connection on reset */
#define AIC_EISCONN                 -127   /* Socket is already connected */
#define AIC_ENOTCONN                -128   /* Socket is not connected */
#define AIC_ETOOMANYREFS            -129   /* Too many references: can't splice */
#define AIC_ENOTSUP                 -134   /* Unsupported value */
#define AIC_EILSEQ                  -138   /* Illegal byte sequence */
#define AIC_EOVERFLOW               -139   /* Value overflow */
#define AIC_ECANCELED               -140   /* Operation canceled */

#define AIC_EWOULDBLOCK             EAGAIN /* Operation would block */

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_ERRNO_H__ */

