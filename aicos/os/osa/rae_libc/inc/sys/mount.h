/* sys/mount.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#define BLOCK_SIZE 1024
#define BLOCK_SIZE_BITS	10

#ifdef __cplusplus
extern "C" {
#endif

int mount (const char *, const char *, const char *, unsigned long , const void *);
int umount (const char *);
int mkfs(const char *, const char *);

#ifdef __cplusplus
};
#endif

#endif /* _SYS_MOUNT_H */
