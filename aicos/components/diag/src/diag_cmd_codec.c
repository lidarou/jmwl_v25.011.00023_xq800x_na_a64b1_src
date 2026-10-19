/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This diag codec source file
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1182ab
#define LOG_MODULE    AMOD_DIAG

#include <aic_assert.h>
#include <aic_math.h>
#include <aic_osa.h>
#include <diag_cmd_codec.h>
#include <diag_cmd_inner.h>

/*
 * The diag decode state machine define:
 * DIAG_DECODE_WAIT_MAGIC
 * ==> DIAG_DECODE_WAIT_HEAD
 * ==> DIAG_DECODE_WAIT_DATA
 * ==> DIAG_DECODE_WAIT_TAIL
 * ==> DIAG_DECODE_DONE
 * ==> DIAG_DECODE_WAIT_MAGIC
 */
enum {
    DIAG_DECODE_WAIT_MAGIC = 0,
    DIAG_DECODE_WAIT_HEAD,
    DIAG_DECODE_WAIT_DATA,
    DIAG_DECODE_WAIT_TAIL,
    DIAG_DECODE_DONE
};

/*
 * The struct of the diag decoder.
 *
 * @head_len: Save the len of received cmd header.
 * @state: save the state machine of the codec.
 * @data_len: Save the len of received cmd data.
 * @decode_cnt: record the received cmd cnt, increase 1 every time.
 * @encode_cnt: record the response cmd cnt, increase 1 every time.
 * @diag_cmd: the diag cmd pointer.
 * @diag_head: use to save the diag header.
 */
typedef struct diag_codec {
    u8      head_len;
    u8      state;
    u16     data_len;
    u32     decode_cnt;
    u32     encode_cnt;

    DIAG_CMD_HEAD_T diag_head;
    diag_recv_cmd_t *recv_cmd;
} diag_codec_t;

/* diag magic: 0x7e5a5a7e */
static const u8 s_diag_magic[DIAG_MAGIC_CNT] =
{
    DIAG_MAGIC0,
    DIAG_MAGIC1,
    DIAG_MAGIC2,
    DIAG_MAGIC3
};

void *diag_codec_create(void)
{
    diag_codec_t *codec = aos_malloc(sizeof(*codec));

    AIC_ASSERT(codec);

    /* init it */
    codec->state = DIAG_DECODE_WAIT_MAGIC;
    codec->head_len = 0;
    codec->data_len = 0;
    codec->decode_cnt = 0;
    codec->encode_cnt = 0;

    return codec;
}

void diag_codec_destroy(void *handle)
{
    aos_free(handle);
}

diag_recv_cmd_t *diag_decode_cmd(void *handle, void *data, size_t size, size_t *decode)
{
    diag_codec_t *codec = handle;
    DIAG_CMD_HEAD_T *diag_head = &codec->diag_head;
    u8 *p = data, *head = (u8 *)diag_head, *diag_data;
    size_t copy;
    u32 i = 0;

    /* try find magic */
    if (codec->state == DIAG_DECODE_WAIT_MAGIC) {
        for (i = 0; i < size; i++, p++) {
            /* if find the len magic, ++len */
            if (*p == s_diag_magic[codec->head_len]) {
                head[codec->head_len++] = *p;
            } else {
                /* reset len to 0. */
                codec->head_len = 0;
                diag_iddbg(0x0101b4, "", "magic error, reset wait magic.\n");
            }

            /* find all the magic */
            if (codec->head_len == DIAG_MAGIC_CNT) {
                diag_iddbg(0x083a67, "", "get the magic, wait head...\n");
                codec->state = DIAG_DECODE_WAIT_HEAD;
                /* to next char. */
                i++;
                p++;
                break;
            }
        }
    }

    /* wait the head */
    if (codec->state == DIAG_DECODE_WAIT_HEAD) {
        for (; i < size; i++, p++) {
            head[codec->head_len++] = *p;
            /* get the whole head */
            if (codec->head_len == sizeof(*diag_head)) {
                if (diag_head->data_len == 0) {
                    diag_iddbg(0x00272b, "", "get the head, no data.\n");
                    codec->state = DIAG_DECODE_WAIT_TAIL;
                } else {
                    diag_iddbg(0x152c23, "d", "get the head, wait data(len=%d)...\n", diag_head->data_len);
                    codec->state = DIAG_DECODE_WAIT_DATA;
                    codec->data_len = 0;
                }

                /* malloc the recv cmd */
                codec->recv_cmd = aos_malloc(sizeof(*(codec->recv_cmd)));
                AOS_ASSERT(codec->recv_cmd);

                /* malloc the data */
                if (diag_head->data_len) {
                    diag_data = aos_malloc(diag_head->data_len);
                    AOS_ASSERT(diag_data);
                } else {
                    diag_data = NULL;
                }

                diag_set_recv(codec->recv_cmd, diag_head->sn,
                              0, 0, diag_head->sys,
                              diag_head->cmd, diag_head->subcmd,
                              diag_data, diag_head->data_len);

                /* to next char. */
                i++;
                p++;
                break;
            }
        }
    }

    /* wait the data */
    if (codec->state == DIAG_DECODE_WAIT_DATA) {
        /* will copy the min of the buffer data and the left diag data. */
        copy = MIN((u32)(size - i), (u32)(diag_head->data_len - codec->data_len));
        diag_iddbg(0x07b32a, "xx", "get the data: total len=0x%x, get len=0x%x.\n",
                   diag_head->data_len, codec->data_len);
        if (copy > 0) {
            diag_data = (u8 *)(codec->recv_cmd->data);
            memcpy(diag_data + codec->data_len, p, copy);
            codec->data_len += copy;
            /* to next char. */
            i += copy;
            p += copy;
        }

        /* finish copy all data */
        if (codec->data_len == diag_head->data_len) {
            diag_iddbg(0x02149f, "", "get the data done.\n");
            codec->state = DIAG_DECODE_WAIT_TAIL;
        }
    }

    /* decode done */
    if (codec->state == DIAG_DECODE_WAIT_TAIL) {
        /* this packet over. */
        if (i >= size)
            return NULL;

        /* get a whole packet. */
        *decode = i + 1;
        codec->state = DIAG_DECODE_DONE;
        diag_iddbg(0x0ff31c, "xd", "get cmd done sn=0x%x, decode=%d\n", diag_head->sn, *decode);

        /* check tail. */
        if (*p != DIAG_TAIL) {
            diag_head->err_no = DIAG_RSP_TAIL_CHECK_FAIL;
            *decode = (*decode - 1);
            aicx_error(0x018edd, "x", "Tail[0x%x] != 0xed\n", *p);
        }

        codec->decode_cnt++;

        /* reset codec */
        codec->state = DIAG_DECODE_WAIT_MAGIC;
        codec->head_len = 0;
        codec->data_len = 0;
        memset(diag_head, 0, sizeof(*diag_head));

        return codec->recv_cmd;
    }

    return NULL;
}

DIAG_CMD_HEAD_T *diag_encode_cmd(diag_rsp_cmd_t *rsp_cmd)
{
    DIAG_CMD_HEAD_T *diag_cmd;
    u8 *tail;

    diag_cmd = aos_malloc(sizeof(*diag_cmd) + rsp_cmd->data_len + 1);
    AIC_ASSERT(diag_cmd);

    diag_cmd->magic = rsp_cmd->m2 ? DIAG_MAGIC_2 : DIAG_MAGIC;
    diag_cmd->sn = rsp_cmd->sn;
    diag_cmd->cmd = rsp_cmd->cmd;
    diag_cmd->subcmd = rsp_cmd->subcmd;
    diag_cmd->data_len = rsp_cmd->data_len;
    diag_cmd->err_no = rsp_cmd->err_no;
    diag_cmd->sys = rsp_cmd->sys;

    if (rsp_cmd->data)
        memcpy((void *)(diag_cmd + 1), rsp_cmd->data, rsp_cmd->data_len);
    else
        diag_cmd->data_len = 0;

    /* Add the tail */
    tail = (u8 *)(diag_cmd);
    tail += (sizeof(*diag_cmd) + rsp_cmd->data_len);
    *tail = DIAG_TAIL;

    return diag_cmd;
}

