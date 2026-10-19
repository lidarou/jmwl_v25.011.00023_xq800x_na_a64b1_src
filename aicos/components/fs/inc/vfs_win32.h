/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system win32 define
 */

#ifndef __AIC_FS_WIN32_H__
#define __AIC_FS_WIN32_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <stdarg.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define PATH_MAX        255
#define OPEN_MAX        64

#define FXFFS_TYPE      0x46584653
#define ROMFS_TYPE      0x524F4D46

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
#undef FIONREAD
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
#undef FIONBIO
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

#define	_SYS__DEFAULT_FCNTL_H_
#define	_FOPEN		(-1)	/* from sys/file.h, kernel use only */
#define	_FREAD		0x0001	/* read enabled */
#define	_FWRITE		0x0002	/* write enabled */
#define	_FAPPEND	0x0008	/* append (writes guaranteed at the end) */
#define	_FMARK		0x0010	/* internal; mark during gc() */
#define	_FDEFER		0x0020	/* internal; defer for next gc pass */
#define	_FASYNC		0x0040	/* signal pgrp when data ready */
#define	_FSHLOCK	0x0080	/* BSD flock() shared lock present */
#define	_FEXLOCK	0x0100	/* BSD flock() exclusive lock present */
#define	_FCREAT		0x0200	/* open with file create */
#define	_FTRUNC		0x0400	/* open with truncation */
#define	_FEXCL		0x0800	/* error on open if file exists */
#define	_FNBIO		0x1000	/* non blocking I/O (sys5 style) */
#define	_FSYNC		0x2000	/* do all writes synchronously */
#define	_FNONBLOCK	0x4000	/* non blocking I/O (POSIX style) */
#define	_FNDELAY	_FNONBLOCK	/* non blocking I/O (4.2 style) */
#define	_FNOCTTY	0x8000	/* don't assign a ctty on this open */
#if 1//defined (__CYGWIN__)
#define	_FBINARY	0x10000
#define	_FTEXT		0x20000
#endif
#define	_FNOINHERIT	0x40000
#define	_FDIRECT	0x80000
#define	_FNOFOLLOW	0x100000
#define	_FDIRECTORY	0x200000
#define	_FEXECSRCH	0x400000
#if 1//defined (__CYGWIN__)
#define	_FTMPFILE	0x800000
#define	_FNOATIME	0x1000000
#define	_FPATH		0x2000000
#endif

#define	O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)

/*
 * Flag values for open(2) and fcntl(2)
 * The kernel adds 1 to the open modes to turn it into some
 * combination of FREAD and FWRITE.
 */
#define	O_RDONLY	0		/* +1 == FREAD */
#define	O_WRONLY	1		/* +1 == FWRITE */
#define	O_RDWR		2		/* +1 == FREAD|FWRITE */
#define	O_APPEND	_FAPPEND
#define	O_CREAT		_FCREAT
#define	O_TRUNC		_FTRUNC
#define	O_EXCL		_FEXCL
#define O_SYNC		_FSYNC
/*	O_NDELAY	_FNDELAY 	set in include/fcntl.h */
/*	O_NDELAY	_FNBIO 		set in include/fcntl.h */
#define	O_NONBLOCK	_FNONBLOCK
#define	O_NOCTTY	_FNOCTTY

#define	O_CLOEXEC	_FNOINHERIT
#define	O_NOFOLLOW	_FNOFOLLOW
#define	O_DIRECTORY	_FDIRECTORY
#define	O_EXEC		_FEXECSRCH
#define	O_SEARCH	_FEXECSRCH

#if 1//__BSD_VISIBLE
#define	O_DIRECT	_FDIRECT
#endif

#if 1//defined (__CYGWIN__)
#define O_BINARY	_FBINARY
#define O_TEXT		_FTEXT
#define O_DSYNC         _FSYNC
#define O_RSYNC         _FSYNC

/* Linux-specific flags */
#if 1//__GNU_VISIBLE
#define O_TMPFILE	_FTMPFILE
#define O_NOATIME	_FNOATIME
#define O_PATH		_FPATH
#endif
#endif

#define O_LARGEFILE 0x2000

#if 1//__MISC_VISIBLE

/*
 * Flags that work for fcntl(fd, F_SETFL, FXXXX)
 */
#define	FAPPEND		_FAPPEND
#define	FSYNC		_FSYNC
#define	FASYNC		_FASYNC
#define	FNBIO		_FNBIO
#define	FNONBIO		_FNONBLOCK	/* XXX fix to be NONBLOCK everywhere */
#define	FNDELAY		_FNDELAY

/*
 * Flags that are disallowed for fcntl's (FCNTLCANT);
 * used for opens, internal state, or locking.
 */
#define	FREAD		_FREAD
#define	FWRITE		_FWRITE
#define	FMARK		_FMARK
#define	FDEFER		_FDEFER
#define	FSHLOCK		_FSHLOCK
#define	FEXLOCK		_FEXLOCK

/*
 * The rest of the flags, used only for opens
 */
#define	FOPEN		_FOPEN
#define	FCREAT		_FCREAT
#define	FTRUNC		_FTRUNC
#define	FEXCL		_FEXCL
#define	FNOCTTY		_FNOCTTY

#endif	/* __MISC_VISIBLE */

#if 1//__BSD_VISIBLE
#define	FNONBLOCK	_FNONBLOCK
#endif	/* __BSD_VISIBLE */

/* XXX close on exec request; must match UF_EXCLOSE in user.h */
#define	FD_CLOEXEC	1	/* posix */

/* fcntl(2) requests */
#define	F_DUPFD		0	/* Duplicate fildes */
#define	F_GETFD		1	/* Get fildes flags (close on exec) */
#define	F_SETFD		2	/* Set fildes flags (close on exec) */
#define	F_GETFL		3	/* Get file flags */
#define	F_SETFL		4	/* Set file flags */
#if 1//__BSD_VISIBLE || __POSIX_VISIBLE >= 200112
#define	F_GETOWN 	5	/* Get owner - for ASYNC */
#define	F_SETOWN 	6	/* Set owner - for ASYNC */
#endif /* __BSD_VISIBLE || __POSIX_VISIBLE >= 200112 */
#define	F_GETLK  	7	/* Get record-locking information */
#define	F_SETLK  	8	/* Set or Clear a record-lock (Non-Blocking) */
#define	F_SETLKW 	9	/* Set or Clear a record-lock (Blocking) */
#if 1//__MISC_VISIBLE
#define	F_RGETLK 	10	/* Test a remote lock to see if it is blocked */
#define	F_RSETLK 	11	/* Set or unlock a remote lock */
#define	F_CNVT 		12	/* Convert a fhandle to an open fd */
#define	F_RSETLKW 	13	/* Set or Clear remote record-lock(Blocking) */
#endif	/* __MISC_VISIBLE */
#define	F_DUPFD_CLOEXEC	14	/* As F_DUPFD, but set close-on-exec flag */

#define F_GETPATH       15 /* Get the path of the file descriptor(BSD/macOS) */

#define FFCNTL      (FNONBLOCK | FNDELAY | FAPPEND | FSYNC | FASYNC)

#define	_IFMT		0170000	/* type of file */
#define		_IFDIR	0040000	/* directory */
#define		_IFCHR	0020000	/* character special */
#define		_IFBLK	0060000	/* block special */
#define		_IFREG	0100000	/* regular */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
#define		_IFIFO	0010000	/* fifo */

#define 	S_BLKSIZE  1024 /* size of a block */

#define	S_ISUID		0004000	/* set user id on execution */
#define	S_ISGID		0002000	/* set group id on execution */
#define	S_ISVTX		0001000	/* save swapped text even after use */
#if 1//__BSD_VISIBLE
#define	S_IREAD		0000400	/* read permission, owner */
#define	S_IWRITE 	0000200	/* write permission, owner */
#define	S_IEXEC		0000100	/* execute/search permission, owner */
#define	S_ENFMT 	0002000	/* enforcement-mode locking */
#endif	/* !_BSD_VISIBLE */

#define	S_IFMT		_IFMT
#define	S_IFDIR		_IFDIR
#define	S_IFCHR		_IFCHR
#define	S_IFBLK		_IFBLK
#define	S_IFREG		_IFREG
#define	S_IFLNK		_IFLNK
#define	S_IFSOCK	_IFSOCK
#define	S_IFIFO		_IFIFO

#define	S_IRWXU 	(S_IRUSR | S_IWUSR | S_IXUSR)
#define		S_IRUSR	0000400	/* read permission, owner */
#define		S_IWUSR	0000200	/* write permission, owner */
#define		S_IXUSR 0000100/* execute/search permission, owner */
#define	S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)
#define		S_IRGRP	0000040	/* read permission, group */
#define		S_IWGRP	0000020	/* write permission, grougroup */
#define		S_IXGRP 0000010/* execute/search permission, group */
#define	S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)
#define		S_IROTH	0000004	/* read permission, other */
#define		S_IWOTH	0000002	/* write permission, other */
#define		S_IXOTH 0000001/* execute/search permission, other */

#if 1//__BSD_VISIBLE
#define ACCESSPERMS (S_IRWXU | S_IRWXG | S_IRWXO) /* 0777 */
#define ALLPERMS (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO) /* 07777 */
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) /* 0666 */
#endif

#define	S_ISBLK(m)	(((m)&_IFMT) == _IFBLK)
#define	S_ISCHR(m)	(((m)&_IFMT) == _IFCHR)
#define	S_ISDIR(m)	(((m)&_IFMT) == _IFDIR)
#define	S_ISFIFO(m)	(((m)&_IFMT) == _IFIFO)
#define	S_ISREG(m)	(((m)&_IFMT) == _IFREG)
#define	S_ISLNK(m)	(((m)&_IFMT) == _IFLNK)
#define	S_ISSOCK(m)	(((m)&_IFMT) == _IFSOCK)

/* File types */
#define FT_REGULAR           0
#define FT_SOCKET            1
#define FT_DIRECTORY         2
#define FT_USER              3

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

typedef u32                  dev_t;
typedef u32                  mode_t;
typedef signed int           ssize_t;
typedef int                  off_t;

#define __restrict

#define NAME_MAX             255 /* max bytes in a file name */

typedef unsigned short ino_t;
typedef unsigned short nlink_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;
typedef long blksize_t;
typedef long blkcnt_t;

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
struct tm
{
  int	tm_sec;
  int	tm_min;
  int	tm_hour;
  int	tm_mday;
  int	tm_mon;
  int	tm_year;
  int	tm_wday;
  int	tm_yday;
  int	tm_isdst;
#ifdef __TM_GMTOFF
  long	__TM_GMTOFF;
#endif
#ifdef __TM_ZONE
  const char *__TM_ZONE;
#endif
};

struct timespec {
    time_t  tv_sec;     /* seconds */
    long    tv_nsec;    /* and nanoseconds */
};

struct stat 
{
    dev_t           st_dev;
    ino_t           st_ino;
    mode_t          st_mode;
    nlink_t         st_nlink;
    uid_t           st_uid;
    gid_t           st_gid;
    dev_t           st_rdev;
    off_t           st_size;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    blksize_t       st_blksize;
    blkcnt_t        st_blocks;
    long            st_spare4[2];
};

struct statfs {
   long    f_type;     /* type of filesystem */
   long    f_bsize;    /* optimal transfer block size */
   long    f_blocks;   /* total data blocks in file system */
   long    f_bfree;    /* free blocks in fs */
   long    f_bavail;   /* free blocks avail to non-superuser */
   long    f_files;    /* total file nodes in file system */
   long    f_ffree;    /* free file nodes in fs */
   long    f_fsid;     /* file system id */
   long    f_namelen;  /* maximum length of filenames */
   long    f_spare[6]; /* spare for later */
};

struct rtc_time {
    u32 tm_year;
    u32 tm_mon;
    u32 tm_wday;
    u32 tm_day;
    u32 tm_hour;
    u32 tm_min;
    u32 tm_sec;
    u32 tm_msec;
    u32 tm_usec;
};

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


struct dirent {
    ino_t           d_ino;                  /* file inode number of entry */
    off_t           d_off;                  /* directory offset of entry */
    unsigned short  d_reclen;               /* length of this record */
    unsigned char   d_type;                 /* file type, see below */
    char            d_name[NAME_MAX + 1];   /* name must be no longer than this */
};

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
#if 1
/* fs posix interface. */
int open(const char *path, int flags, ...);
int creat(const char *path, mode_t mode);
ssize_t read(int fd, void *buf, size_t nbytes);
ssize_t write(int fd, const void *buf, size_t nbytes);
off_t lseek(int fd, off_t offset, int whence);
int ftruncate(int fd, off_t length);
int fsync(int fildes);
int close(int fd);
int rename(const char *old_file, const char *new_file);
int unlink(const char *pathname);
int stat(const char *file, struct stat *buf);
int fstat(int fildes, struct stat *buf);
int fcntl(int fildes, int cmd, ...);
int ioctl(int fildes, int cmd, ...);
int statfs(const char *path, struct statfs *buf);
int access(const char *path, int amode);
void setcwd(char *buf);
char *getcwd(char *buf, size_t size);
int mkdir(const char *path, mode_t mode);
int rmdir(const char *pathname);
DIR *opendir(const char *name);
struct dirent *readdir(DIR *d);
int closedir(DIR *d);
int chdir(const char *path);

/* fs libc interface. */
FILE *fopen(const char *__restrict filename,
            const char *__restrict mode);
size_t fread(void *__restrict buffer, size_t size, size_t count,
             FILE *__restrict stream);
size_t fwrite(const void *__restrict buffer, size_t size, size_t count,
              FILE *__restrict stream);
int fclose(FILE *__restrict stream);
#endif

int mount (const char *, const char *, const char *, unsigned long , const void *);
int umount (const char *);
int mkfs(const char *, const char *);

void seekdir(DIR *, long);
void rewinddir(DIR *);
long telldir(DIR *);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_FS_WIN32_H__ */

