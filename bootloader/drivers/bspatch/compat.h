#ifndef u8
#define u8 unsigned char
#endif

#ifndef u16
#define u16 unsigned char
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef u64
#define u64 unsigned long long
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#define off_t   long long

#define errx  err
#define err(id,...)     \
{   \
    printf(##__VA_ARGS__);  \
    return -id; \
}

#define ssize_t			long

#define MAX(x,y) (((x)>(y)) ? (x) : (y))

long lseek(FILE *fd, long offset, int origin);

