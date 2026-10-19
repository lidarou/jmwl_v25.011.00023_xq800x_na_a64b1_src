/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    this file is use_no_semihosting for arm compile retarget .
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ab18d
#define LOG_MODULE    AMOD_AOS

#if defined ( __CC_ARM ) || (__ARMCC_VERSION)

#include <aic_assert.h>
#include <rt_sys.h>
#include <rt_misc.h>

#ifdef CONFIG_AIC_NO_USE_SEMIHOST
extern u32 __use_no_semihosting;

/*
 * Open a file. May return -1 if the file failed to open.
 */
int _sys_open(const char *name, int open_mode)
//FILEHANDLE _sys_open(const char *name, int open_mode)
{
    return 1;
}

/*
 * Close a file. Should return 0 on success or a negative value on error.
 */
int _sys_close(FILEHANDLE fh)
{
    return 0;
}

/*
 * Write to a file. Returns 0 on success, negative on error, and
 * the number of characters _not_ written on partial success.
 * `mode' exists for historical reasons and must be ignored.
 */
int _sys_write(FILEHANDLE fh, const unsigned char *buf,
               unsigned len, int mode)
{
    return len;
}

/*
 * Read from a file. Can return:
 *  - zero if the read was completely successful
 *  - the number of bytes _not_ read, if the read was partially successful
 *  - the number of bytes not read, plus the top bit set (0x80000000), if
 *    the read was partially successful due to end of file
 *  - -1 if some error other than EOF occurred
 *
 * It is also legal to signal EOF by returning no data but
 * signalling no error (i.e. the top-bit-set mechanism need never
 * be used).
 *
 * So if (for example) the user is trying to read 8 bytes at a time
 * from a file in which only 5 remain, this routine can do three
 * equally valid things:
 *
 *  - it can return 0x80000003 (3 bytes not read due to EOF)
 *  - OR it can return 3 (3 bytes not read), and then return
 *    0x80000008 (8 bytes not read due to EOF) on the next attempt
 *  - OR it can return 3 (3 bytes not read), and then return
 *    8 (8 bytes not read, meaning 0 read, meaning EOF) on the next
 *    attempt
 *
 * `mode' exists for historical reasons and must be ignored.
 */
int _sys_read(FILEHANDLE fh, unsigned char *buf,
              unsigned len, int mode)
{
    return 0;
}

/*
 * Write a character to the output channel. This function is used
 * for last-resort error message output.
 */
void _ttywrch(int ch)
{

}

/*
 * Return non-zero if the argument file is connected to a terminal.
 */
int _sys_istty(FILEHANDLE fh)
{
    return 0;
}

/*
 * Move the file position to a given offset from the file start.
 * Returns >=0 on success, <0 on failure.
 */
int _sys_seek(FILEHANDLE fh, long pos)
{
    return 0;
}

/*
 * Flush any OS buffers associated with fh, ensuring that the file
 * is up to date on disk. Result is >=0 if OK, negative for an
 * error.
 */
int _sys_ensure(FILEHANDLE fh)
{
    return 0;
}

/*
 * Return the current length of a file, or <0 if an error occurred.
 * _sys_flen is allowed to reposition the file pointer (so Unix can
 * implement it with a single lseek, for example), since it is only
 * called when processing SEEK_END relative fseeks, and therefore a
 * call to _sys_flen is always followed by a call to _sys_seek.
 */
long _sys_flen(FILEHANDLE fh)
{
    return 0;
}

/*
 * Return the name for temporary file number sig in the buffer
 * name. Returns 0 on failure. maxlen is the maximum name length
 * allowed.
 */
int _sys_tmpnam(char *name, int sig, unsigned maxlen)
{
    return 1;
}

/*
 * Terminate the program, passing a return code back to the user.
 * This function never returns.
 */
void _sys_exit(int returncode)
{

}

/*
 * Return a pointer to the command line used to invoke the program.
 * The supplied buffer may be used to store the string, but need
 * not be.
 */
char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}
#else
/*
 * Open a file. May return -1 if the file failed to open.
 */
int $Sub$$_sys_open(const char *name, int open_mode)
//FILEHANDLE _sys_open(const char *name, int open_mode)
{
    return 1;
}

/*
 * Close a file. Should return 0 on success or a negative value on error.
 */
int $Sub$$_sys_close(FILEHANDLE fh)
{
    return 0;
}

/*
 * Write to a file. Returns 0 on success, negative on error, and
 * the number of characters _not_ written on partial success.
 * `mode' exists for historical reasons and must be ignored.
 */
int $Sub$$_sys_write(FILEHANDLE fh, const unsigned char *buf,
               unsigned len, int mode)
{
    return len;
}

/*
 * Read from a file. Can return:
 *  - zero if the read was completely successful
 *  - the number of bytes _not_ read, if the read was partially successful
 *  - the number of bytes not read, plus the top bit set (0x80000000), if
 *    the read was partially successful due to end of file
 *  - -1 if some error other than EOF occurred
 *
 * It is also legal to signal EOF by returning no data but
 * signalling no error (i.e. the top-bit-set mechanism need never
 * be used).
 *
 * So if (for example) the user is trying to read 8 bytes at a time
 * from a file in which only 5 remain, this routine can do three
 * equally valid things:
 *
 *  - it can return 0x80000003 (3 bytes not read due to EOF)
 *  - OR it can return 3 (3 bytes not read), and then return
 *    0x80000008 (8 bytes not read due to EOF) on the next attempt
 *  - OR it can return 3 (3 bytes not read), and then return
 *    8 (8 bytes not read, meaning 0 read, meaning EOF) on the next
 *    attempt
 *
 * `mode' exists for historical reasons and must be ignored.
 */
int $Sub$$_sys_read(FILEHANDLE fh, unsigned char *buf,
              unsigned len, int mode)
{
    return 0;
}

/*
 * Write a character to the output channel. This function is used
 * for last-resort error message output.
 */
void $Sub$$_ttywrch(int ch)
{

}

/*
 * Return non-zero if the argument file is connected to a terminal.
 */
int $Sub$$_sys_istty(FILEHANDLE fh)
{
    return 0;
}

/*
 * Move the file position to a given offset from the file start.
 * Returns >=0 on success, <0 on failure.
 */
int $Sub$$_sys_seek(FILEHANDLE fh, long pos)
{
    return 0;
}

/*
 * Flush any OS buffers associated with fh, ensuring that the file
 * is up to date on disk. Result is >=0 if OK, negative for an
 * error.
 */
int $Sub$$_sys_ensure(FILEHANDLE fh)
{
    return 0;
}

/*
 * Return the current length of a file, or <0 if an error occurred.
 * _sys_flen is allowed to reposition the file pointer (so Unix can
 * implement it with a single lseek, for example), since it is only
 * called when processing SEEK_END relative fseeks, and therefore a
 * call to _sys_flen is always followed by a call to _sys_seek.
 */
long $Sub$$_sys_flen(FILEHANDLE fh)
{
    return 0;
}

/*
 * Return the name for temporary file number sig in the buffer
 * name. Returns 0 on failure. maxlen is the maximum name length
 * allowed.
 */
int $Sub$$_sys_tmpnam(char *name, int sig, unsigned maxlen)
{
    return 1;
}

/*
 * Terminate the program, passing a return code back to the user.
 * This function never returns.
 */
void $Sub$$_sys_exit(int returncode)
{

}

/*
 * Return a pointer to the command line used to invoke the program.
 * The supplied buffer may be used to store the string, but need
 * not be.
 */
char *$Sub$$_sys_command_string(char *cmd, int len)
{
    return NULL;
}
#endif /* CONFIG_AIC_NO_USE_SEMIHOST */

int aic_rt_no_semihost_init(void)
{
    /* open __use_no_semihosting */
    //__asm(".global __use_no_semihosting\n\t");

    return 0;
}

/*
 * This can be defined to override the standard memory models' way
 * of determining where to put the initial stack and heap.
 *
 * The input parameters R0 and R2 contain nothing useful. The input
 * parameters SP and SL are the values that were in SP and SL when
 * the program began execution (so you can return them if you want
 * to keep that stack).
 *
 * The two `limit' fields in the return structure are ignored if
 * you are using the one-region memory model: the memory region is
 * taken to be all the space between heap_base and stack_base.
 */
__value_in_regs struct __initial_stackheap
__user_initial_stackheap(__uintptr R0, __uintptr SP,
                         __uintptr R2, __uintptr SL)
{
    struct __initial_stackheap init_statckheap;

    /* no heap */
    init_statckheap.heap_base = 0;
    init_statckheap.heap_limit = 0;

    /* keep that stack */
    init_statckheap.stack_base = SP;
    init_statckheap.stack_limit = SL;

    return init_statckheap;
}
#endif

