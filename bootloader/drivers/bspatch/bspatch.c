/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bzip2/bzlib.h"
#include "compat.h"
#include "console/console.h"

#define BS_DEBUG
#ifdef BS_DEBUG
#define fota_info(fmt, ...) do {\
        log_print("fota: ");\
        log_print(fmt, ##__VA_ARGS__);\
    } while (0)
#else
#define fota_info(...)
#endif

#define FAILURE_RETURN do { \
    if (ret < 0) { \
        log_print("fota: Line=%d, FAILURE_RETURN ret=%d!\n", __LINE__, ret); \
        goto err_ret; \
    } \
} while (0);

/* BZ2 header size in bytes, see "File format" below */
#define BZ2_HDR_SIZE       32
/* A BZ2 control block is a 3-element tuple, each 8 bytes */
#define BZ2_CTLBLK_SIZE    24

static off_t offtin(unsigned char *buf)
{
    off_t y;

    y=buf[7]&0x7F;
    y=y*256;y+=buf[6];
    y=y*256;y+=buf[5];
    y=y*256;y+=buf[4];
    y=y*256;y+=buf[3];
    y=y*256;y+=buf[2];
    y=y*256;y+=buf[1];
    y=y*256;y+=buf[0];

    if(buf[7]&0x80) y=-y;

    return y;
}

/**
 * patch_partition() - Apply patch to specified partition image
 *
 * @p_orig   : Point to old partition start address
 * @origsz   : Old partition size in byte, not old image size
 * @p_diff   : Point to difference package start address
 * @diffsz   : Difference package size in byte
 * @p_new    : Point to buffer which store new image
 * @newsz    : Size of generated new image, auto set by function
 *
 * Apply difference patch to specified partition image. It decompress
 * all control tuples and calculates size of decompressed data
 * block and ext data block, then malloc buffer and decompress all
 * data.
 *
 * Return: AIC_SUCCESS if no error;errno if there is error.
 *
 */
s32 patch_partition        (u8   *p_orig,     /* old image pos */
                            u32  origsz,      /* old image size in bytes */
                            u8   *p_diff,     /* diff package pos */
                            u32  diffsz,      /* diff package size */
                            u8   *p_new,      /* buf pos to store new image */
                            u32  *newsz)      /* new image size in bytes after patch */
{
    s32 ret = 0;
    u32 bzctrllen = 0;         /* bz2 control data length */
    u32 bzdatalen = 0;         /* bz2 diff data length */
    u32 decdatalen = 0;        /* decompressed diff data length */
    u32 bzextlen = 0;          /* bz2 ext data length */
    u32 decextlen = 0;         /* decompressed ext data length */
    u32 oldpos = 0;            /* reference pos of old file */
    u32 newpos = 0;            /* patched new file reference pos */
    u32 newsize = 0;           /* patched new file size in bytes */
    u32 realen = 0;            /* real decompressed data length */
    long long ctrl[3];         /* control data tuple */
    u8 *cpfbz2 = NULL;         /* bz2 control data ptr */
    u8 *dpfbz2 = NULL;         /* bz2 diff data ptr */
    u8 *epfbz2 = NULL;         /* bz2 ext data ptr */
    char *decdp = NULL;        /* diff data decompress buffer ptr */
    char *decep = NULL;        /* ext data decompress buffer ptr */
    u8 *ptuple = NULL;         /* control tuple pointer */
    u32 trytms = 1;            /* try number */
    u32 tuplesz = 0;           /* control tuples size in byte */
    u32 opcnt = 0;             /* do patch count */

    /*
    File format:
        0         8      "BSDIFF40"
        8         8      X
        16        8      Y
        24        8      sizeof(newfile)
        32        X      bzip2(control block)
        32+X      Y      bzip2(diff block)
        32+X+Y    ???    bzip2(extra block)
    with control block a set of triples (x,y,z) meaning "add x bytes
    from oldfile to x bytes from the diff block; copy y bytes from the
    extra block; seek forwards in oldfile by z bytes".
    */
    if (!(p_diff[0] == 'B' && p_diff[1] == 'S' &&
          p_diff[2] == 'D' && p_diff[3] == 'I' &&
          p_diff[4] == 'F' && p_diff[5] == 'F' &&
          p_diff[6] == '4' && p_diff[7] == '0')) {
        ret = -1;
        goto err_ret;
    }

    /* get lengths from header */
    bzctrllen = offtin(p_diff + 8);
    bzdatalen = offtin(p_diff + 16);
    newsize   = offtin(p_diff + 24);

    /* set control block, diff block, extra block pos */
    cpfbz2 = p_diff + BZ2_HDR_SIZE;
    dpfbz2 = cpfbz2 + bzctrllen;
    epfbz2 = dpfbz2 + bzdatalen;
    bzextlen = diffsz - BZ2_HDR_SIZE - bzdatalen - bzctrllen;
    do {
        /* added every 1000 tuples, which is 24KB */
        tuplesz = BZ2_CTLBLK_SIZE * 1000 * trytms;
        trytms++;
        ptuple = malloc(tuplesz);
        if (!ptuple) {
            fota_info("fatal error, malloc 0x%x mem failed\n", tuplesz);
            ret = -1;
            FAILURE_RETURN
        } else {
            fota_info("Malloc, 0x%x~0x%x(0x%x)\n",
                       ptuple,
                       ptuple + tuplesz,
                       tuplesz);
        }
        realen = tuplesz;
        ret = BZ2_bzBuffToBuffDecompress ((char *)ptuple,
                                          &realen,
                                          (char *)cpfbz2,
                                          bzctrllen,
                                          realen,
                                          1,  /* small */
                                          0); /* verbosity */
        if (ret != 0) {
            fota_info("fatal error, decompress control tuple failed, errno(0x%x)!\n",
                       ret);
            ret = -2;
            FAILURE_RETURN
        }
        if (realen < tuplesz)
            break;
        free(ptuple);
        ptuple = NULL;
    } while (1);
    /*
     * Calculate uncompressed data block and ext block size
     * for allocating buffers.
     */
    if (realen % BZ2_CTLBLK_SIZE)
        fota_info("fatal error, control tuples are not coupled!\n");
    else
        fota_info("control tuple count 0x%x\n", realen / BZ2_CTLBLK_SIZE);
    for (int i = 0; i < realen / BZ2_CTLBLK_SIZE; i++) {
        ctrl[0] = offtin(ptuple + i * BZ2_CTLBLK_SIZE);
        ctrl[1] = offtin(ptuple + i * BZ2_CTLBLK_SIZE + 8);
        ctrl[2] = offtin(ptuple + i * BZ2_CTLBLK_SIZE + 16);
        decdatalen += ctrl[0];
        decextlen  += ctrl[1];
    }

    fota_info("decextlen=0x%x!\n", decextlen);

    /* malloc buffer for uncompressed diff data and ext data */
    decdp = malloc(decdatalen);
    if (decdp == NULL) {
        ret = -3;
        goto err_ret;
    }
    fota_info("Malloc, 0x%x~0x%x(0x%x)\n",
               decdp,
               decdp + decdatalen,
               decdatalen);
    if (decextlen != 0) {
        decep = malloc(decextlen);
        if (decep == NULL) {
            ret = -4;
            goto err_ret;
        }
        fota_info("Malloc, 0x%x~0x%x(0x%x)\n",
                   decep,
                   decep + decextlen,
                   decextlen);
    }
    /* decompress all diff data */
    ret = BZ2_bzBuffToBuffDecompress (decdp,
                                      &decdatalen,
                                      (char *)dpfbz2,
                                      bzdatalen,
                                      decdatalen,
                                      1,  /* small */
                                      0); /* verbosity */
    if (ret != 0) {
        fota_info("fatal error, decompress diff data failed, errno(0x%x)!\n",
                   ret);
        ret = -5;
        FAILURE_RETURN
    }

    if (decextlen != 0) {
        /* decompress all ext data */
        ret = BZ2_bzBuffToBuffDecompress (decep,
                                          &decextlen,
                                          (char *)epfbz2,
                                          bzextlen,
                                          decextlen,
                                          1,  /* small */
                                          0); /* verbosity */
        if (ret != 0) {
            fota_info("fatal error, decompress ext data failed, errno(0x%x)!\n",
                   ret);
            ret = -6;
            FAILURE_RETURN
        }
    }
    /* do patch */
    newpos     = 0;
    oldpos     = 0;
    decdatalen = 0;
    decextlen  = 0;
    while (newpos < newsize) {
        /* 1. contrl block */
        ctrl[0] = offtin(ptuple + opcnt * BZ2_CTLBLK_SIZE);
        ctrl[1] = offtin(ptuple + opcnt * BZ2_CTLBLK_SIZE + 8);
        ctrl[2] = offtin(ptuple + opcnt * BZ2_CTLBLK_SIZE + 16);
        if (newpos + ctrl[0] > newsize) {
            ret = -7;
            FAILURE_RETURN
        }
        fota_info("newpos = 0x%x, oldpos = 0x%x, 0x%ld-0x%ld-0x%ld\n",
                   newpos, oldpos, ctrl[0], ctrl[1], ctrl[2]);
        /* 2. data block */
        for (u32 i = 0; i < ctrl[0]; i++)
            p_new[newpos + i] = (unsigned char)decdp[decdatalen + i];
        for (u32 i = 0; i < ctrl[0]; i++) {
            if (oldpos + i < origsz)
                p_new[newpos + i] += p_orig[oldpos + i];
        }
        /* Adjust pointers */
        decdatalen += ctrl[0];
        newpos     += ctrl[0];
        oldpos     += ctrl[0];
        /* 3. extra data */
        if (newpos + ctrl[1] > newsize) {
            ret = -8;
            FAILURE_RETURN
        }
        for (u32 i = 0; i < ctrl[1]; i++)
            p_new[newpos + i] = (unsigned char)decep[decextlen + i];
        decextlen += ctrl[1];
        newpos    += ctrl[1];
        oldpos    += ctrl[2];
        opcnt++;
    }
    fota_info("opcnt 0x%x\n", opcnt);

err_ret:
    if (decdp != NULL) {
        free(decdp);
        decdp = NULL;
    }
    if (decep != NULL) {
        free(decep);
        decep = NULL;
    }
    if (ptuple != NULL) {
        free(ptuple);
        ptuple = NULL;
    }
    if (ret < 0)
        log_print("do patch failed, errno %d\n", ret);
    return ret;
}
