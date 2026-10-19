/* <dirent.h> includes <sys/dirent.h>, which is this file.  On a
   system which supports <dirent.h>, this file is overridden by
   dirent.h in the libc/sys/.../sys directory.  On a system which does
   not support <dirent.h>, we will get this file which uses #error to force
   an error.  */

#ifndef _SYS_DIRENT_H_
#define _SYS_DIRENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/syslimits.h>

struct __dirstream
{
    void    *__fd;
    char    *__data;
    int     __entry_data;
    char    *ptr;
    int     __entry_ptr;
    size_t  __allocation;
    size_t  __size;
};

typedef struct __dirstream DIR;

/*
 * The dirent structure defines the format of directory entries returned by
 * the getdirentries(2) system call.
 *
 * A directory entry has a struct dirent at the front of it, containing its
 * inode number, the length of the entry, and the length of the name
 * contained in the entry.  These are followed by the name padded to an 8
 * byte boundary with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is MAXNAMLEN.
 *
 * Explicit padding between the last member of the header (d_namlen) and
 * d_name avoids ABI padding at the end of dirent on LP64 architectures.
 * There is code depending on d_name being last.
 */

struct dirent {
    ino_t           d_ino;                  /* file inode number of entry */
    off_t           d_off;                  /* directory offset of entry */
    unsigned short  d_reclen;               /* length of this record */
    unsigned char   d_type;                 /* file type, see below */
    char            d_name[NAME_MAX + 1];   /* name must be no longer than this */
};

#define MAXNAMLEN           NAME_MAX

/*
 * File types
 */
#define DT_UNKNOWN          0
#define DT_FIFO             1
#define DT_CHR              2
#define DT_SEM              3
#define DT_DIR              4
#define DT_MQ               5
#define DT_BLK              6
#define DT_SHM              7
#define DT_REG              8
#define DT_MTD              9
#define DT_LNK              10
#define DT_SOCK             12
#define DT_WHT              14

/*
 * Convert between stat structure types and directory types.
 */
#define IFTODT(mode)        (((mode) & 0170000) >> 12)
#define DTTOIF(dirtype)     ((dirtype) << 12)

/* File types */
#define FT_REGULAR          0
#define FT_SOCKET           1
#define FT_DIRECTORY        2
#define FT_USER             3

/* file system attribute */
#define FA_NORMAL           0x00
#define FA_RDONLY           0x01
#define FA_HIDDEN           0x02
#define FA_SYSTEM           0x04
#define FA_LABEL            0x08
#define FA_DIREC            0x10
#define FA_ARCH             0x20

#define FD_YEAR_SHIFT       9
#define FD_MONTH_SHIFT      5
#define FT_HOUR_SHIFT       11
#define FT_MINUTE_SHIFT     5
#define FD_BASE_YEAR        1970

#ifdef __cplusplus
}
#endif

#endif /*_SYS_DIRENT_H_*/
