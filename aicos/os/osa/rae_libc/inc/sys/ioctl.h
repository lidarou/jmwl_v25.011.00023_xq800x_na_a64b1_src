/*
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __SYS_IOCTL_H__
#define __SYS_IOCTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define _TIOCBASE       (0x0100) /* Terminal I/O ioctl commands */
#define _WDIOCBASE      (0x0200) /* Watchdog driver ioctl commands */
#define _FIOCBASE       (0x0300) /* File system ioctl commands */

#define _IOC_MASK       (0x00ff)
#define _IOC_TYPE(cmd)  ((cmd) & ~_IOC_MASK)
#define _IOC_NR(cmd)    ((cmd) & _IOC_MASK)
                                           
#define _IOC(type,nr)   ((type)|(nr))
                                           
                                           /* Terminal I/O ioctl commands **********************************************/
                                           
#define _TIOCVALID(c)   (_IOC_TYPE(c)==_TIOCBASE)
#define _TIOC(nr)       _IOC(_TIOCBASE,nr)

#define _FIOCVALID(c)   (_IOC_TYPE(c)==_FIOCBASE)
#define _FIOC(nr)       _IOC(_FIOCBASE,nr)

#define FIOC_REFORMAT   _FIOC(0x0001)     /* IN:  None
                                           * OUT: None
                                           */
#define FIOC_OPTIMIZE   _FIOC(0x0002)     /* IN:  The number of bytes to recover
                                           *      (ignored on most file systems)
                                           * OUT: None
                                           */
#define FIOC_FILEPATH   _FIOC(0x0003)     /* IN:  FAR char *(length >= PATH_MAX)
                                           * OUT: The full file path
                                           */
#define FIOC_INTEGRITY  _FIOC(0x0004)     /* Run a consistency check on the
                                           *      file system media.
                                           * IN:  None
                                           * OUT: None
                                           */
#define FIOC_DUMP       _FIOC(0x0005)     /* Dump logical content of media.
                                           * IN:  None
                                           * OUT: None
                                           */
#define FIONREAD        _FIOC(0x0006)     /* IN:  Location to return value (int *)
                                           * OUT: Bytes readable from this fd
                                           */
#define FIONWRITE       _FIOC(0x0007)     /* IN:  Location to return value (int *)
                                           * OUT: Number bytes in send queue
                                           */
#define FIONSPACE       _FIOC(0x0008)     /* IN:  Location to return value (int *)
                                           * OUT: Free space in send queue.
                                           */
#define FIONUSERFS      _FIOC(0x0009)     /* IN:  Pointer to struct usefs_config_s
                                           *      holding userfs configuration.
                                           * OUT: Instance number is returned on
                                           *      success.
                                           */
#define FIONBIO         _FIOC(0x000a)     /* IN:  Boolean option takes an
                                           *      int value.
                                           * OUT: Origin option.
                                           */
#define FIOCLEX         _FIOC(0x000b)     /* IN:  None
                                           * OUT: None
                                           */
#define FIONCLEX        _FIOC(0x000c)     /* IN:  None
                                           * OUT: None
                                           */
#define FIOC_NOTIFY     _FIOC(0x000d)     /* IN:  Pointer to struct automount_notify_s
                                           *      holding automount notification
                                           *      configuration
                                           * OUT: None
                                           */

int ioctl(int fildes, int cmd, ...);

#ifdef __cplusplus
}
#endif

#endif
