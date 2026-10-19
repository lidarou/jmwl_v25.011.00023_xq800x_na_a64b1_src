/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC LIST time function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1a591a
#define LOG_MODULE    AMOD_AOS

#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#include <aic_osa.h>
#include <aos_mem.h>
#include <aos_list.h>
#include <aos_config.h>

#define GET_MUTEX(MUTEX_PTR) \
    if (MUTEX_PTR) \
        AOS_ASSERT(AOS_SUCCESS == aos_get_mutex(MUTEX_PTR, AOS_WAIT_FOREVER));

#define PUT_MUTEX(MUTEX_PTR) \
    if (MUTEX_PTR) \
        aos_put_mutex(MUTEX_PTR);

aos_list_index aos_hash_default_key_func(aos_hash_key hash_key,
                                         aos_list_index hash_len)
{
    AOS_ASSERT(hash_len);
    return (aos_list_index)(hash_key % hash_len);
}

aos_hash_tbl *aos_create_hash_tbl(aos_hash_length hash_len,
                                  aos_hash_key_func func_key,
                                  boolean enable_mutex)
{
    aos_hash_tcb*   tbl_ptr;
    aos_hash_node*  node_ptr;
    aos_hash_length index;

    AOS_ASSERT(hash_len > 0);

    /* allocate for hash tcb and hash node */
    tbl_ptr = (aos_hash_tcb *)aos_get_mem(AOS_COMMON_POOL_ID,
                                          sizeof(aos_hash_tcb) +
                                          hash_len * (sizeof(aos_hash_used_list)
                                          + sizeof(aos_hash_node)));

    if (enable_mutex) {
        tbl_ptr->mutex_ptr = aos_create_mutex("hash_tbl_mutex");
        if (!tbl_ptr->mutex_ptr) {
            aos_free_mem(tbl_ptr);
            return AOS_NULL;
        }
    }
    else
        tbl_ptr->mutex_ptr = AOS_NULL;

    /* initialize tbl ptr */
    node_ptr = (aos_hash_node*)(((u8 *)tbl_ptr) + sizeof(aos_hash_tcb) +
                                hash_len * sizeof(aos_hash_used_list));
    tbl_ptr->first_node_ptr = node_ptr;
    tbl_ptr->max_hash_item_nums = hash_len;
    tbl_ptr->list = (aos_hash_used_list*)(((u8*)tbl_ptr) +
                                          sizeof(aos_hash_tcb));

    if (!func_key)
        tbl_ptr->key_func = aos_hash_default_key_func;
    else
        tbl_ptr->key_func = func_key;

    /* initialize node */
    tbl_ptr->free_head = 0;
    for (index = 0; index < hash_len; index++) {
        node_ptr->next = index + 1;
        node_ptr++;
    }

    /* initialize index */
    for (index = 0; index < hash_len; index++)
        tbl_ptr->list[index].head = hash_len;

    return (aos_hash_tbl*)tbl_ptr;
}

aos_status aos_insert_hash_item(aos_hash_tbl *hash_tbl_ptr,
                                aos_hash_key hash_key,
                                aos_hash_item hash_item)
{
    aos_hash_tcb* tbl_ptr = (aos_hash_tcb*)hash_tbl_ptr;
    aos_list_index factor;
    aos_list_index index;
    aos_hash_node* node_ptr;
    aos_status ret_status = AOS_ERROR;

    if (!tbl_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_ERROR;
    }

    GET_MUTEX(tbl_ptr->mutex_ptr)

    if (tbl_ptr->free_head < tbl_ptr->max_hash_item_nums) {
        factor = (*tbl_ptr->key_func)(hash_key, tbl_ptr->max_hash_item_nums);
        if (factor < tbl_ptr->max_hash_item_nums) {
            /* find free node */
            index = tbl_ptr->free_head;
            node_ptr = tbl_ptr->first_node_ptr + index;
            tbl_ptr->free_head = node_ptr->next;

            /* save key and item */
            node_ptr->item = hash_item;
            node_ptr->key = hash_key;

            /* insert node to list */
            node_ptr->next = tbl_ptr->list[factor].head;
            tbl_ptr->list[factor].head = index;
            ret_status = AOS_SUCCESS;
        }
    }

    PUT_MUTEX(tbl_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_find_hash_item(aos_hash_tbl *hash_tbl_ptr,
                              aos_hash_key hash_key,
                              aos_hash_item *hash_item_ptr)
{
    aos_hash_tcb* tbl_ptr = (aos_hash_tcb*)hash_tbl_ptr;
    aos_list_index factor, index;
    aos_hash_node* node_ptr;
    aos_status ret_status = AOS_ERROR;

    if (!tbl_ptr || !hash_item_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_ERROR;
    }

    GET_MUTEX(tbl_ptr->mutex_ptr)

    factor = (*tbl_ptr->key_func)(hash_key, tbl_ptr->max_hash_item_nums);
    if (factor < tbl_ptr->max_hash_item_nums) {
        index = tbl_ptr->list[factor].head;
        while (index != tbl_ptr->max_hash_item_nums) {
            node_ptr = tbl_ptr->first_node_ptr + index;
            if (node_ptr->key == hash_key) {
                *hash_item_ptr = node_ptr->item;
                ret_status = AOS_SUCCESS;
                break;
            }
            index = node_ptr->next;
        }
    }

    PUT_MUTEX(tbl_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_replace_hash_item(aos_hash_tbl*   hash_tbl_ptr,
                                 aos_hash_key    hash_key,
                                 aos_hash_item   hash_item,
                                 aos_hash_item*  old_item_ptr)
{
    aos_hash_tcb* tbl_ptr = (aos_hash_tcb*)hash_tbl_ptr;
    aos_list_index factor, index;
    aos_hash_node* node_ptr;
    aos_status ret_status = AOS_ERROR;

    if (!tbl_ptr || !old_item_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_ERROR;
    }

    GET_MUTEX(tbl_ptr->mutex_ptr)

    factor = (*tbl_ptr->key_func)(hash_key,tbl_ptr->max_hash_item_nums);
    if (factor < tbl_ptr->max_hash_item_nums) {
        index = tbl_ptr->list[factor].head;
        while (index != tbl_ptr->max_hash_item_nums) {
            node_ptr = tbl_ptr->first_node_ptr + index;
            if (node_ptr->key == hash_key) {
                *old_item_ptr = node_ptr->item;
                node_ptr->item = hash_item;
                ret_status = AOS_SUCCESS;
            }
            index = node_ptr->next;
        }
    }

    PUT_MUTEX(tbl_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_remove_hash_item(aos_hash_tbl *hash_tbl_ptr,
                                aos_hash_key hash_key,
                                aos_hash_item *hash_item_ptr)
{
    aos_hash_tcb* tbl_ptr = (aos_hash_tcb*)hash_tbl_ptr;
    aos_list_index factor, cur_index, prev_index;
    aos_hash_node* node_ptr;
    aos_status ret_status = AOS_ERROR;

    AOS_ASSERT(tbl_ptr && hash_item_ptr);

    GET_MUTEX(tbl_ptr->mutex_ptr)

    factor = (*tbl_ptr->key_func)(hash_key,tbl_ptr->max_hash_item_nums);
    if (factor < tbl_ptr->max_hash_item_nums) {
        cur_index = tbl_ptr->list[factor].head;
        prev_index = cur_index;
        while (cur_index != tbl_ptr->max_hash_item_nums) {
            node_ptr = tbl_ptr->first_node_ptr + cur_index;
            if (node_ptr->key == hash_key) {
                if (cur_index == prev_index)
                    tbl_ptr->list[factor].head = node_ptr->next;
                else
                    (tbl_ptr->first_node_ptr + prev_index)->next =
                                               node_ptr->next;
                node_ptr->next = tbl_ptr->free_head;
                tbl_ptr->free_head = cur_index;
                *hash_item_ptr = node_ptr->item;
                ret_status = AOS_SUCCESS;
                break;
            }
            prev_index = cur_index;
            cur_index = node_ptr->next;
        }
    }

    PUT_MUTEX(tbl_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_delete_hash_tbl(aos_hash_tbl *hash_tbl_ptr,
                               aos_hash_free free_func)
{
    aos_hash_tcb* tbl_ptr = (aos_hash_tcb*)hash_tbl_ptr;
    aos_list_index index, index1;
    aos_hash_node* node_ptr;

    if (!tbl_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_ERROR;
    }

    GET_MUTEX(tbl_ptr->mutex_ptr)

    for (index = 0; index < tbl_ptr->max_hash_item_nums;index++) {
        if (free_func) {
            index1 = tbl_ptr->list[index].head;
            while (index1 != tbl_ptr->max_hash_item_nums) {
                node_ptr = tbl_ptr->first_node_ptr + index1;
                (*free_func)(node_ptr->item);
                index1 = node_ptr->next;
            }
        }
        tbl_ptr->list[index].head = tbl_ptr->max_hash_item_nums;
    }

    tbl_ptr->free_head = tbl_ptr->max_hash_item_nums;

    PUT_MUTEX(tbl_ptr->mutex_ptr)

    if (tbl_ptr->mutex_ptr) {
        aos_delete_mutex(tbl_ptr->mutex_ptr);
        tbl_ptr->mutex_ptr = AOS_NULL;
    }

    aos_free_mem(tbl_ptr);

    return AOS_SUCCESS;
}

aos_blist *aos_create_blist(u16 blist_len, boolean enable_mutex)
{
    aos_blist_index index;
    aos_blist *blist_ptr;

    blist_ptr = (aos_blist*)aos_malloc(sizeof(aos_blist) +
                                       blist_len * sizeof(aos_blist_node));
    if (!blist_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_NULL;
    }

    if (enable_mutex) {
        blist_ptr->mutex_ptr = aos_create_mutex("blist_mutex");
        if (!blist_ptr->mutex_ptr) {
            aos_free((u8 *)blist_ptr);
            return AOS_NULL;
        }
    }
    else
        blist_ptr->mutex_ptr = AOS_NULL;

    blist_ptr->free_head = 0;
    blist_ptr->head = blist_len;
    blist_ptr->tail = blist_len;
    blist_ptr->node = (aos_blist_node*)((u8*)blist_ptr + sizeof(aos_blist));

    for (index = 0; index < blist_len; index++) {
        blist_ptr->node[index].next = (aos_blist_index)(index + 1);
        blist_ptr->node[index].prev = (aos_blist_index)(index - 1);
        blist_ptr->node[index].item = 0;
    }
    blist_ptr->node[0].prev = blist_len;
    blist_ptr->max_node_nums = blist_len;

    return blist_ptr;
}

aos_status aos_add_tail_blist(aos_blist *blist_ptr, aos_blist_item item)
{
    aos_blist_index index;
    aos_status ret_status = AOS_SUCCESS;

    if (!blist_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_ERROR;
    }

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->free_head == blist_ptr->max_node_nums)
        ret_status = AOS_ERROR;
    else {
        /* insert node to tail */
        index = blist_ptr->free_head;
        blist_ptr->free_head = blist_ptr->node[index].next;
        blist_ptr->node[index].item = item;
        blist_ptr->node[index].next = blist_ptr->max_node_nums;
        if (blist_ptr->tail == blist_ptr->max_node_nums) {
            blist_ptr->tail = index;
            blist_ptr->head = index;
            blist_ptr->node[index].prev = blist_ptr->max_node_nums;
        } else {
            blist_ptr->node[index].prev = blist_ptr->tail;
            blist_ptr->node[blist_ptr->tail].next = index;
            blist_ptr->tail = index;
        }
    }

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_add_head_blist(aos_blist *blist_ptr, aos_blist_item item)
{
    aos_blist_index index;
    aos_status ret_status = AOS_SUCCESS;

    if (!blist_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_ERROR;
    }

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->free_head == blist_ptr->max_node_nums)
        ret_status = AOS_ERROR;
    else {
        index = blist_ptr->free_head;
        blist_ptr->free_head = blist_ptr->node[index].next;
        blist_ptr->node[index].item = item;
        blist_ptr->node[index].prev = blist_ptr->max_node_nums;
        if (blist_ptr->head == blist_ptr->max_node_nums) {
            blist_ptr->head = index;
            blist_ptr->tail = index;
            blist_ptr->node[index].next = blist_ptr->max_node_nums;
        } else {
            blist_ptr->node[index].next = blist_ptr->head;
            blist_ptr->node[blist_ptr->head].prev = index;
            blist_ptr->head = index;
        }
    }

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_retrieve_tail_blist(aos_blist *blist_ptr,
                                   aos_blist_item *item_ptr)
{
    aos_blist_index index;
    aos_status ret_status = AOS_SUCCESS;

    AOS_ASSERT(blist_ptr && item_ptr);

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->tail == blist_ptr->max_node_nums)
        ret_status = AOS_ERROR;
    else {
        index = blist_ptr->tail;
        *item_ptr = blist_ptr->node[index].item;
        blist_ptr->tail = blist_ptr->node[index].prev;
        if (blist_ptr->tail != blist_ptr->max_node_nums)
            blist_ptr->node[blist_ptr->tail].next = blist_ptr->max_node_nums;
        else
            blist_ptr->head = blist_ptr->tail;

        blist_ptr->node[index].next = blist_ptr->free_head;
        blist_ptr->free_head = index;
    }

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_retrieve_head_blist(aos_blist *blist_ptr,
                                   aos_blist_item *item_ptr)
{
    aos_blist_index index;
    aos_status ret_status = AOS_SUCCESS;

    AOS_ASSERT(blist_ptr && item_ptr);

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->head == blist_ptr->max_node_nums)
        ret_status = AOS_ERROR;
    else {
        index = blist_ptr->head;
        *item_ptr = blist_ptr->node[index].item;
        blist_ptr->head = blist_ptr->node[index].next;
        if (blist_ptr->head != blist_ptr->max_node_nums)
            blist_ptr->node[blist_ptr->head].prev = blist_ptr->max_node_nums;
        else
            blist_ptr->tail = blist_ptr->head;

        blist_ptr->node[index].next = blist_ptr->free_head;
        blist_ptr->free_head = index;
    }

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return ret_status;
}

aos_blist_node *aos_head_blist_node(aos_blist *blist_ptr)
{
    aos_blist_node *node_ptr = AOS_NULL;

    if (!blist_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_NULL;
    }

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->head != blist_ptr->max_node_nums)
        node_ptr = &blist_ptr->node[blist_ptr->head];

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return node_ptr;
}

aos_blist_node *aos_next_blist_node(aos_blist *blist_ptr,
                                    aos_blist_node *node_ptr)
{
    aos_blist_node *next_node_ptr = AOS_NULL;

    AOS_ASSERT(blist_ptr && node_ptr);

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (node_ptr->next < blist_ptr->max_node_nums)
        next_node_ptr = &blist_ptr->node[node_ptr->next];

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return next_node_ptr;
}

aos_blist_node *aos_tail_blist_node(aos_blist *blist_ptr)
{
    aos_blist_node *node_ptr = AOS_NULL;

    if (!blist_ptr)
    {
        AOS_ASSERT(AOS_FALSE);
        return AOS_NULL;
    }

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->tail != blist_ptr->max_node_nums)
        node_ptr = &blist_ptr->node[blist_ptr->tail];

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return node_ptr;
}

aos_blist_node *aos_prev_blist_node(aos_blist *blist_ptr,
                                    aos_blist_node *node_ptr)
{
    aos_blist_node *prev_node_ptr = AOS_NULL;

    AOS_ASSERT(blist_ptr && node_ptr);

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (node_ptr->prev < blist_ptr->max_node_nums)
        prev_node_ptr = &blist_ptr->node[node_ptr->prev];

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return prev_node_ptr;
}

aos_blist_item aos_get_blist_item(aos_blist_node *node_ptr)
{
    if (!node_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return 0;
    }
    return node_ptr->item;
}

aos_status aos_replace_blist_item(aos_blist* blist_ptr,
                                  aos_blist_node* node_ptr,
                                  aos_blist_item  new_item,
                                  aos_blist_item* old_item_ptr)
{
    AOS_ASSERT(blist_ptr && node_ptr);

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (old_item_ptr)
        *old_item_ptr = node_ptr->item;

    node_ptr->item = new_item;

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return AOS_SUCCESS;
}

aos_status aos_insert_before_blist_node(aos_blist *blist_ptr,
                                        aos_blist_node *node_ptr,
                                        aos_blist_item item)
{
    aos_blist_index index;
    aos_status ret_status = AOS_SUCCESS;

    AOS_ASSERT(blist_ptr && node_ptr);

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->free_head == blist_ptr->max_node_nums)
        ret_status = AOS_ERROR;
    else {
        index = blist_ptr->free_head;
        blist_ptr->free_head = blist_ptr->node[index].next;
        blist_ptr->node[index].item = item;
        blist_ptr->node[index].prev = node_ptr->prev;
        if (node_ptr->prev != blist_ptr->max_node_nums) {
            blist_ptr->node[index].next = blist_ptr->node[node_ptr->prev].next;
            blist_ptr->node[node_ptr->prev].next = index;
        }
        else {
            blist_ptr->node[index].next = blist_ptr->head;
            blist_ptr->head = index;
        }
        node_ptr->prev = index;
    }

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_insert_after_blist_node(aos_blist *blist_ptr,
                                       aos_blist_node *node_ptr,
                                       aos_blist_item item)
{
    aos_blist_index index;
    aos_status ret_status = AOS_SUCCESS;

    AOS_ASSERT(blist_ptr && node_ptr);

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->free_head == blist_ptr->max_node_nums)
        ret_status = AOS_ERROR;
    else {
        index = blist_ptr->free_head;
        blist_ptr->free_head = blist_ptr->node[index].next;
        blist_ptr->node[index].item = item;
        blist_ptr->node[index].next = node_ptr->next;
        if (node_ptr->next != blist_ptr->max_node_nums) {
            blist_ptr->node[index].prev = blist_ptr->node[node_ptr->next].prev;
            blist_ptr->node[node_ptr->next].prev = index;
        } else {
            blist_ptr->node[index].prev = blist_ptr->tail;
            blist_ptr->tail = index;
        }
        node_ptr->next = index;
    }

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_remove_blist_node(aos_blist *blist_ptr, aos_blist_node *node_ptr)
{
    aos_blist_index index;
    aos_status ret_status = AOS_SUCCESS;

    AOS_ASSERT(blist_ptr && node_ptr);

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (node_ptr->prev == blist_ptr->max_node_nums) {
        index = blist_ptr->head;
        if (node_ptr->next == blist_ptr->max_node_nums) {
            blist_ptr->head = blist_ptr->max_node_nums;
            blist_ptr->tail = blist_ptr->max_node_nums;
        } else {
            blist_ptr->node[node_ptr->next].prev = blist_ptr->max_node_nums;
            blist_ptr->head = node_ptr->next;
        }
    } else {
        if (node_ptr->next == blist_ptr->max_node_nums) {
            index = blist_ptr->tail;
            blist_ptr->node[node_ptr->prev].next = blist_ptr->max_node_nums;
            blist_ptr->tail = node_ptr->prev;
        } else {
            index = blist_ptr->node[node_ptr->prev].next;
            blist_ptr->node[node_ptr->prev].next = node_ptr->next;
            blist_ptr->node[node_ptr->next].prev = node_ptr->prev;
        }
    }

    node_ptr->next = blist_ptr->free_head;
    blist_ptr->free_head = index;

    PUT_MUTEX(blist_ptr->mutex_ptr)

    return ret_status;
}

aos_status aos_delete_blist(aos_blist *blist_ptr, aos_blist_free item_free)
{
    aos_blist_index index;

    if (!blist_ptr) {
        AOS_ASSERT(AOS_FALSE);
        return AOS_ERROR;
    }

    GET_MUTEX(blist_ptr->mutex_ptr)

    if (item_free) {
        index = blist_ptr->head;
        while (index != blist_ptr->max_node_nums) {
            (*item_free)(blist_ptr->node[index].item);
            index = blist_ptr->node[index].next;
        }
    }

    blist_ptr->head = blist_ptr->max_node_nums;
    blist_ptr->tail = blist_ptr->max_node_nums;
    blist_ptr->free_head = blist_ptr->max_node_nums;

    PUT_MUTEX(blist_ptr->mutex_ptr)

    if (blist_ptr->mutex_ptr) {
        aos_delete_mutex(blist_ptr->mutex_ptr);
        blist_ptr->mutex_ptr = AOS_NULL;
    }

    aos_free((u8 *)blist_ptr);

    return AOS_SUCCESS;
}

/* read and write API can only be used in one thread */
aos_ring_buffer_bsptr *aos_ring_buffer_bsptr_init(u32 uiNum)
{
    aos_ring_buffer_bsptr *pstRingBuff;

    AOS_ASSERT(uiNum);

    pstRingBuff =
    (aos_ring_buffer_bsptr *)aos_get_mem(AOS_COMMON_POOL_ID,
                                         uiNum * sizeof(u8 *) +
                                         sizeof(aos_ring_buffer_bsptr));

    pstRingBuff->uiBllockNum = uiNum;
    pstRingBuff->ppucBuffer = (u8**)(pstRingBuff + 1);
    pstRingBuff->uiWriteCount = 0;
    pstRingBuff->uiWriteIndex = 0;
    pstRingBuff->uiReadCount = 0;
    pstRingBuff->uiReadIndex = 0;

    return pstRingBuff;
}

u32 aos_ring_buffer_bsptr_read(aos_ring_buffer_bsptr *pstRingBuffer,
                               u8 **ppucReceiveBuff,
                               u32 uiReceiveNum)
{
    u32  uiActReceNum = 0;

    AOS_ASSERT(pstRingBuffer && ppucReceiveBuff);

    while (uiActReceNum < uiReceiveNum) {
        if (pstRingBuffer->uiWriteCount != pstRingBuffer->uiReadCount) {
            ppucReceiveBuff[uiActReceNum] =
                pstRingBuffer->ppucBuffer[pstRingBuffer->uiReadIndex];

            uiActReceNum++;

            pstRingBuffer->uiReadCount++;
            pstRingBuffer->uiReadIndex++;
            if (pstRingBuffer->uiReadIndex >= pstRingBuffer->uiBllockNum)
                pstRingBuffer->uiReadIndex = 0;
        }
        else
            /* no data */
            break;
    }

    return uiActReceNum;
}

aos_status aos_ring_buffer_bsptr_write(aos_ring_buffer_bsptr *pstRingBuffer,
                                       u8 *pucData, boolean *pbNullFlag)
{
    AOS_ASSERT(pstRingBuffer && pucData);

    if (pstRingBuffer->uiWriteCount - pstRingBuffer->uiReadCount <
       pstRingBuffer->uiBllockNum) {
        pstRingBuffer->ppucBuffer[pstRingBuffer->uiWriteIndex] = pucData;
        pstRingBuffer->uiWriteCount++;
        pstRingBuffer->uiWriteIndex++;
        if (pstRingBuffer->uiWriteIndex >= pstRingBuffer->uiBllockNum)
            pstRingBuffer->uiWriteIndex = 0;

        if (pstRingBuffer->uiWriteCount - pstRingBuffer->uiReadCount == 1)
            *pbNullFlag = AOS_TRUE;
        else
            *pbNullFlag = AOS_FALSE;

        return AOS_SUCCESS;
    }

    /* no free buffer */
    return AOS_ERROR;
}

void aos_ring_buffer_bsptr_delete(aos_ring_buffer_bsptr *pstRingBuffer)
{
    AOS_ASSERT(pstRingBuffer);

    aos_memset((void*)pstRingBuffer, 0, sizeof(aos_ring_buffer_bsptr));

    aos_free_mem(pstRingBuffer);
}

aos_plmn_freq_tab aos_create_plmn_freq_tab(u32  uiMaxCount)
{
    aos_plmn_freq *pstTempTab;
    u32 loop;

    AOS_ASSERT(uiMaxCount > 1);

    pstTempTab = (aos_plmn_freq*)aos_get_mem(AOS_COMMON_POOL_ID,
                                             sizeof(aos_plmn_freq));
    AOS_ASSERT(pstTempTab);

    pstTempTab->uiMaxCount = uiMaxCount;
    pstTempTab->uiUsedCount = 0;

    pstTempTab->pstPlmnFreqInfo =
        (aos_plmn_freq_info*)aos_get_mem(AOS_COMMON_POOL_ID,
                                         sizeof(aos_plmn_freq_info) *
                                         uiMaxCount);
    AOS_ASSERT(pstTempTab->pstPlmnFreqInfo);

    pstTempTab->pstUsedHeadNode = pstTempTab->pstUsedTailNode = AOS_NULL;
    pstTempTab->pstFreeHeadNode = pstTempTab->pstPlmnFreqInfo;
    pstTempTab->pstPlmnFreqInfo[0].pstPrev = AOS_NULL;

    for (loop = 0; loop < uiMaxCount - 1; loop++) {
        pstTempTab->pstPlmnFreqInfo[loop].pstNext =
            &pstTempTab->pstPlmnFreqInfo[loop+1];
        pstTempTab->pstPlmnFreqInfo[loop+1].pstPrev =
            &pstTempTab->pstPlmnFreqInfo[loop];
    }
    pstTempTab->pstPlmnFreqInfo[loop].pstNext = AOS_NULL;

    /* plmn sort table */
    pstTempTab->puiPlmnSort =
        (aos_plmn_sort *)aos_get_mem(AOS_COMMON_POOL_ID,
                                     sizeof(aos_plmn_sort) * uiMaxCount);
    AOS_ASSERT(pstTempTab->puiPlmnSort);

    /* freq sort table */
    pstTempTab->puiFreqSort =
        (aos_freq_sort *)aos_get_mem(AOS_COMMON_POOL_ID,
                                     sizeof(aos_freq_sort) * uiMaxCount);
    AOS_ASSERT(pstTempTab->puiFreqSort);

    return (aos_plmn_freq_tab)pstTempTab;
}

aos_status aos_insert_plmn_freq(aos_plmn_freq_tab pstPlmnFreqTab,
                                u32 uiPlmn, u16 usFreq)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;
    aos_status ret;
    u32 uiTempSortIndex, uiTabIndex;

    AOS_ASSERT(pstTempTab);

    ret = aos_find_plmn_sort(uiPlmn, pstTempTab->puiPlmnSort,
                             pstTempTab->uiUsedCount, &uiTempSortIndex);
    if (ret == AOS_SUCCESS) {
        for (; uiTempSortIndex > 0; uiTempSortIndex--) {
            if (pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn == uiPlmn)
                continue;
            break;
        }

        for (; uiTempSortIndex < pstTempTab->uiUsedCount; uiTempSortIndex++) {
            uiTabIndex = pstTempTab->puiPlmnSort[uiTempSortIndex].uiTabIndex;
            if (pstTempTab->pstPlmnFreqInfo[uiTabIndex].uiPlmn ==
                   uiPlmn &&
                   pstTempTab->pstPlmnFreqInfo[uiTabIndex].usFreq == usFreq)
                aos_delete_plmn_freq_sub(pstTempTab,
                    &pstTempTab->pstPlmnFreqInfo[uiTabIndex]);
        }
    }

    if (pstTempTab->uiUsedCount == pstTempTab->uiMaxCount) {
        aos_delete_plmn_freq_sub(pstTempTab, pstTempTab->pstUsedHeadNode);
        aos_insert_plmn_freq_sub(pstTempTab, uiPlmn, usFreq);
    } else
        /*½ڵ*/
        aos_insert_plmn_freq_sub(pstTempTab, uiPlmn, usFreq);

    return AOS_SUCCESS;
}

aos_status aos_delete_plmn(aos_plmn_freq_tab pstPlmnFreqTab, u32 uiPlmn)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;
    aos_status ret;
    u32 uiTempSortIndex, uiTabIndex;
    boolean bDeleteFlag;

    AOS_ASSERT(pstTempTab);

    ret = aos_find_plmn_sort(uiPlmn, pstTempTab->puiPlmnSort,
                             pstTempTab->uiUsedCount, &uiTempSortIndex);
    if (ret == AOS_SUCCESS) {
        for (;uiTempSortIndex > 0; uiTempSortIndex--) {
            if (pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn == uiPlmn)
                continue;
            break;
        }
        bDeleteFlag = AOS_FALSE;
        while (uiTempSortIndex < pstTempTab->uiUsedCount) {
            uiTabIndex = pstTempTab->puiPlmnSort[uiTempSortIndex].uiTabIndex;
            if (pstTempTab->pstPlmnFreqInfo[uiTabIndex].uiPlmn == uiPlmn) {
                aos_delete_plmn_freq_sub(pstTempTab, &pstTempTab->pstPlmnFreqInfo[uiTabIndex]);
                bDeleteFlag = AOS_TRUE;
                continue;
            }
            if (bDeleteFlag == AOS_TRUE)
                break;

            uiTempSortIndex++;
        }
    }

    return AOS_SUCCESS;
}

aos_status aos_delete_freq(aos_plmn_freq_tab pstPlmnFreqTab, u16 usFreq)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;
    aos_status ret;
    u32 uiTempSortIndex, uiTabIndex;
    boolean bDeleteFlag;

    AOS_ASSERT(pstTempTab);

    ret = aos_find_freq_sort(usFreq, pstTempTab->puiFreqSort,
                             pstTempTab->uiUsedCount, &uiTempSortIndex);
    if (ret == AOS_SUCCESS) {
        for (; uiTempSortIndex > 0; uiTempSortIndex--) {
            if (pstTempTab->puiFreqSort[uiTempSortIndex].usFreq == usFreq)
                continue;
            break;
        }
        bDeleteFlag = AOS_FALSE;
        while (uiTempSortIndex < pstTempTab->uiUsedCount) {
            uiTabIndex = pstTempTab->puiPlmnSort[uiTempSortIndex].uiTabIndex;
            if (pstTempTab->pstPlmnFreqInfo[uiTabIndex].usFreq == usFreq) {
                aos_delete_plmn_freq_sub(pstTempTab,
                    &pstTempTab->pstPlmnFreqInfo[uiTabIndex]);
                bDeleteFlag = AOS_TRUE;

                continue;
            }

            if (bDeleteFlag)
                break;

            uiTempSortIndex++;
        }
    }

    return AOS_SUCCESS;
}

aos_status aos_delete_freq_area(aos_plmn_freq_tab pstPlmnFreqTab,
                                u16 usFreqMin, u16 usFreqMax)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;
    u32 uiTempSortStartIndex;
    boolean bDeleteFlag;
    aos_status ret;

    AOS_ASSERT(pstTempTab);

    if (pstTempTab->uiUsedCount == 0)
        return AOS_SUCCESS;

    ret = aos_find_freq_sort(usFreqMin, pstTempTab->puiFreqSort,
                       pstTempTab->uiUsedCount, &uiTempSortStartIndex);

    if (ret != AOS_SUCCESS)
        return ret;

    for (; uiTempSortStartIndex > 0; uiTempSortStartIndex--) {
        if (pstTempTab->puiFreqSort[uiTempSortStartIndex].usFreq < usFreqMin)
            break;
    }

    bDeleteFlag = AOS_FALSE;
    while (uiTempSortStartIndex < pstTempTab->uiUsedCount) {
        if (pstTempTab->puiFreqSort[uiTempSortStartIndex].usFreq >= usFreqMin &&
            pstTempTab->puiFreqSort[uiTempSortStartIndex].usFreq <= usFreqMax) {
            aos_delete_plmn_freq_sub(pstTempTab,
                &pstTempTab->pstPlmnFreqInfo[pstTempTab->puiFreqSort[uiTempSortStartIndex].uiTabIndex]);

            bDeleteFlag = AOS_TRUE;

            continue;
        }

        if (bDeleteFlag == AOS_TRUE)
            break;

        uiTempSortStartIndex++;
    }

    return AOS_SUCCESS;
}

aos_status aos_get_spec_plmn_freq(aos_plmn_freq_tab pstPlmnFreqTab,
                                  u32 uiPlmn,u16 *pauiOutInfo,
                                  u32 uiMaxCount, u32 *puiOutNum)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;
    u32 uiTempSortIndex, uiTabIndex;
    u32 uiCurrentIndex;
    boolean bFindFlag;

    AOS_ASSERT(pstTempTab);

    if (aos_find_plmn_sort(uiPlmn, pstTempTab->puiPlmnSort,
                           pstTempTab->uiUsedCount, &uiTempSortIndex)
                           == AOS_SUCCESS) {
        for ( ;uiTempSortIndex > 0; uiTempSortIndex--) {
            if (pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn == uiPlmn)
                continue;
            break;
        }

        bFindFlag = AOS_FALSE;
        for (uiCurrentIndex = 0;uiTempSortIndex < pstTempTab->uiUsedCount;
             uiTempSortIndex++) {
            uiTabIndex = pstTempTab->puiPlmnSort[uiTempSortIndex].uiTabIndex;
            if (pstTempTab->pstPlmnFreqInfo[uiTabIndex].uiPlmn == uiPlmn) {
                if (uiCurrentIndex < uiMaxCount) {
                    pauiOutInfo[uiCurrentIndex] = pstTempTab->pstPlmnFreqInfo[uiTabIndex].usFreq;
                    uiCurrentIndex++;
                }
                else
                    break;
                bFindFlag = AOS_TRUE;
            }
            else {
                if (bFindFlag == AOS_TRUE)
                    break;
            }
        }

        *puiOutNum = uiCurrentIndex;
        return AOS_SUCCESS;
    }

    return AOS_ERROR;
}

aos_status aos_get_all_freq(aos_plmn_freq_tab pstPlmnFreqTab,u16 *pauiOutInfo,
                            u32 uiMaxCount, u32 *puiOutNum)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;
    aos_plmn_freq_info *pstTempNode;
    u32 uiTempIndex, uiCurrentOutIndex;
    u16 usTempFreq;
    aos_status ret;

    uiCurrentOutIndex = 0;

    pstTempNode = pstTempTab->pstUsedTailNode;
    while (pstTempNode != AOS_NULL && uiCurrentOutIndex < uiMaxCount) {
        usTempFreq = pstTempNode->usFreq;
        ret = aos_find_freq_sort(usTempFreq, pstTempTab->puiFreqSort,
                                 pstTempTab->uiUsedCount, &uiTempIndex);
        AOS_ASSERT(ret == AOS_SUCCESS);

        for (; uiTempIndex != 0; uiTempIndex--) {
            if (pstTempTab->puiFreqSort[uiTempIndex].usFreq == usTempFreq)
                continue;
            break;
        }

        for (;uiTempIndex < pstTempTab->uiUsedCount; uiTempIndex++)
        {
            if (pstTempTab->puiFreqSort[uiTempIndex].uiTabIndex ==
                (u32)(pstTempNode - pstTempTab->pstPlmnFreqInfo))
                break;
        }

        if (uiTempIndex < pstTempTab->uiUsedCount - 1) {
            if (pstTempTab->puiFreqSort[uiTempIndex + 1].usFreq != usTempFreq) {
                pauiOutInfo[uiCurrentOutIndex] = usTempFreq;
                uiCurrentOutIndex++;
            }
        }
        else {
            pauiOutInfo[uiCurrentOutIndex] = usTempFreq;
            uiCurrentOutIndex++;
        }

        pstTempNode = pstTempNode->pstPrev;
    }

    *puiOutNum = uiCurrentOutIndex;
    return AOS_SUCCESS;
}

aos_status aos_get_all_plmn(aos_plmn_freq_tab pstPlmnFreqTab,u32 *pauiOutInfo,
                            u32 uiMaxCount, u32 *puiOutNum)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;
    aos_plmn_freq_info *pstTempNode;
    u32 uiTempIndex ,uiCurrentOutIndex;
    u32 usTempPlmn;
    aos_status ret;

    uiCurrentOutIndex = 0;

    pstTempNode = pstTempTab->pstUsedTailNode;
    while (pstTempNode != AOS_NULL && uiCurrentOutIndex < uiMaxCount) {
        usTempPlmn = pstTempNode->uiPlmn;
        ret = aos_find_plmn_sort(usTempPlmn, pstTempTab->puiPlmnSort,
                                 pstTempTab->uiUsedCount, &uiTempIndex);
        AOS_ASSERT(ret == AOS_SUCCESS);

        for (; uiTempIndex != 0; uiTempIndex--) {
            if (pstTempTab->puiPlmnSort[uiTempIndex].uiPlmn == usTempPlmn)
                continue;
            break;
        }

        for (; uiTempIndex < pstTempTab->uiUsedCount; uiTempIndex++) {
            if (pstTempTab->puiPlmnSort[uiTempIndex].uiTabIndex ==
                (u32)(pstTempNode - pstTempTab->pstPlmnFreqInfo))
                break;
        }

        if (uiTempIndex < pstTempTab->uiUsedCount - 1) {
            if (pstTempTab->puiPlmnSort[uiTempIndex + 1].uiPlmn != usTempPlmn) {
                pauiOutInfo[uiCurrentOutIndex] = usTempPlmn;
                uiCurrentOutIndex++;
            }
        }
        else {
            pauiOutInfo[uiCurrentOutIndex] = usTempPlmn;
            uiCurrentOutIndex++;
        }

        pstTempNode = pstTempNode->pstPrev;
    }

    *puiOutNum = uiCurrentOutIndex;
    return AOS_SUCCESS;
}

aos_status aos_clean_plmn_freq(aos_plmn_freq_tab pstPlmnFreqTab)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;
    u32 loop;

    pstTempTab->uiUsedCount = 0;

    pstTempTab->pstUsedHeadNode = pstTempTab->pstUsedTailNode = AOS_NULL;
    pstTempTab->pstFreeHeadNode = pstTempTab->pstPlmnFreqInfo;
    pstTempTab->pstPlmnFreqInfo[0].pstPrev = AOS_NULL;
    for (loop = 0; loop < pstTempTab->uiMaxCount - 1; loop++) {
        pstTempTab->pstPlmnFreqInfo[loop].pstNext =
            &pstTempTab->pstPlmnFreqInfo[loop+1];
        pstTempTab->pstPlmnFreqInfo[loop+1].pstPrev =
            &pstTempTab->pstPlmnFreqInfo[loop];
    }
    pstTempTab->pstPlmnFreqInfo[loop].pstNext = AOS_NULL;

    return AOS_SUCCESS;
}

aos_status aos_delete_plmn_freq(aos_plmn_freq_tab pstPlmnFreqTab)
{
    aos_plmn_freq *pstTempTab = (aos_plmn_freq *)pstPlmnFreqTab;

    aos_clean_plmn_freq(pstPlmnFreqTab);
    aos_free_mem(pstTempTab->pstPlmnFreqInfo);
    aos_free_mem(pstTempTab->puiPlmnSort);
    aos_free_mem(pstTempTab->puiFreqSort);
    aos_free_mem(pstTempTab);

    return AOS_SUCCESS;
}

void aos_delete_plmn_freq_sub(aos_plmn_freq *pstTempTab,
                              aos_plmn_freq_info *pstPlmnFreqNode)
{
    u32 uiTempSortIndex, uiPlmnFreqTanIndex;
    aos_status ret;

    if (pstTempTab->uiUsedCount == 1) {
        pstTempTab->pstUsedHeadNode = AOS_NULL;
        pstTempTab->pstUsedTailNode = AOS_NULL;
    } else {
        if (pstPlmnFreqNode->pstPrev == AOS_NULL) {
            pstTempTab->pstUsedHeadNode = pstPlmnFreqNode->pstNext;
            pstTempTab->pstUsedHeadNode->pstPrev = AOS_NULL;
        } else if (pstPlmnFreqNode->pstNext == AOS_NULL) {
            pstTempTab->pstUsedTailNode = pstPlmnFreqNode->pstPrev;
            pstTempTab->pstUsedTailNode->pstNext = AOS_NULL;
        }
        else {
            pstPlmnFreqNode->pstPrev->pstNext = pstPlmnFreqNode->pstNext;
            pstPlmnFreqNode->pstNext->pstPrev = pstPlmnFreqNode->pstPrev;
        }
    }

    if (pstTempTab->pstFreeHeadNode == AOS_NULL) {
        pstTempTab->pstFreeHeadNode = pstPlmnFreqNode;
        pstPlmnFreqNode->pstNext = pstPlmnFreqNode->pstPrev = AOS_NULL;
    }
    else {
        pstPlmnFreqNode->pstNext = pstTempTab->pstFreeHeadNode;
        pstTempTab->pstFreeHeadNode->pstPrev = pstPlmnFreqNode;
        pstPlmnFreqNode->pstPrev = AOS_NULL;
        pstTempTab->pstFreeHeadNode= pstPlmnFreqNode;
    }

    if (pstTempTab->uiUsedCount > 1) {
        ret = aos_find_plmn_sort(pstPlmnFreqNode->uiPlmn,
                                 pstTempTab->puiPlmnSort,
                                 pstTempTab->uiUsedCount,
                                 &uiTempSortIndex);
        AOS_ASSERT(ret == AOS_SUCCESS);

        for (; uiTempSortIndex > 0; uiTempSortIndex--) {
            if (pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn ==
                pstPlmnFreqNode->uiPlmn)
                continue;
            break;
        }
        uiPlmnFreqTanIndex = pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
        ret = AOS_ERROR;

        for (; uiTempSortIndex < pstTempTab->uiUsedCount; uiTempSortIndex++) {
            if (pstTempTab->puiPlmnSort[uiTempSortIndex].uiTabIndex ==
                uiPlmnFreqTanIndex) {
                if (uiTempSortIndex != pstTempTab->uiUsedCount - 1) {
                    aos_memmove(&pstTempTab->puiPlmnSort[uiTempSortIndex],
                                &pstTempTab->puiPlmnSort[uiTempSortIndex+1],
                                (pstTempTab->uiUsedCount-uiTempSortIndex-1) *
                                sizeof(aos_plmn_sort));
                }
                ret = AOS_SUCCESS;
                break;
            }
        }
        AOS_ASSERT(ret == AOS_SUCCESS);

        ret = aos_find_freq_sort(pstPlmnFreqNode->usFreq,
                                 pstTempTab->puiFreqSort,
                                 pstTempTab->uiUsedCount,
                                 &uiTempSortIndex);
        AOS_ASSERT(ret == AOS_SUCCESS);

        for (; uiTempSortIndex > 0; uiTempSortIndex--) {
            if (pstTempTab->puiFreqSort[uiTempSortIndex].usFreq ==
                pstPlmnFreqNode->usFreq)
                continue;
            break;
        }

        ret = AOS_ERROR;

        for (; uiTempSortIndex < pstTempTab->uiUsedCount; uiTempSortIndex++) {
            if (pstTempTab->puiFreqSort[uiTempSortIndex].uiTabIndex ==
                uiPlmnFreqTanIndex) {
                if (uiTempSortIndex != pstTempTab->uiUsedCount - 1) {
                    aos_memmove(&pstTempTab->puiFreqSort[uiTempSortIndex],
                                &pstTempTab->puiFreqSort[uiTempSortIndex+1],
                                (pstTempTab->uiUsedCount-uiTempSortIndex-1) *
                                sizeof(aos_plmn_sort));
                }
                ret = AOS_SUCCESS;
                break;
            }
        }
        AOS_ASSERT(ret == AOS_SUCCESS);
    }
    pstTempTab->uiUsedCount--;
}

void aos_insert_plmn_freq_sub(aos_plmn_freq *pstTempTab, u32 uiPlmn, u16 usFreq)
{
    aos_plmn_freq_info  *pstPlmnFreqNode;
    aos_status ret;
    u32 uiTempSortIndex;

    AOS_ASSERT(pstTempTab->pstFreeHeadNode);
    pstPlmnFreqNode = pstTempTab->pstFreeHeadNode;
    pstTempTab->pstFreeHeadNode = pstPlmnFreqNode->pstNext;
    if (pstTempTab->pstFreeHeadNode)
        pstTempTab->pstFreeHeadNode->pstPrev = AOS_NULL;

    pstPlmnFreqNode->uiPlmn = uiPlmn;
    pstPlmnFreqNode->usFreq = usFreq;

    if (pstTempTab->pstUsedTailNode) {
        pstTempTab->pstUsedTailNode->pstNext = pstPlmnFreqNode;
        pstPlmnFreqNode->pstPrev = pstTempTab->pstUsedTailNode;
        pstTempTab->pstUsedTailNode = pstPlmnFreqNode;
        pstPlmnFreqNode->pstNext = AOS_NULL;
    } else {
        pstTempTab->pstUsedHeadNode = pstPlmnFreqNode;
        pstTempTab->pstUsedTailNode = pstPlmnFreqNode;
        pstPlmnFreqNode->pstNext = AOS_NULL;
        pstPlmnFreqNode->pstPrev = AOS_NULL;
    }

    if (pstTempTab->uiUsedCount == 0) {
        pstTempTab->puiPlmnSort[0].uiPlmn = uiPlmn;
        pstTempTab->puiPlmnSort[0].uiTabIndex = pstPlmnFreqNode -
                                                pstTempTab->pstPlmnFreqInfo;
        pstTempTab->puiFreqSort[0].usFreq = usFreq;
        pstTempTab->puiFreqSort[0].uiTabIndex = pstPlmnFreqNode -
                                                pstTempTab->pstPlmnFreqInfo;
    }
    else {
        ret = aos_find_plmn_sort(uiPlmn, pstTempTab->puiPlmnSort,
                                 pstTempTab->uiUsedCount, &uiTempSortIndex);
        if (ret == AOS_SUCCESS) {
            for (;uiTempSortIndex < pstTempTab->uiUsedCount;uiTempSortIndex++) {
                if (pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn == uiPlmn)
                    continue;
                break;
            }
            if (uiTempSortIndex < pstTempTab->uiUsedCount) {
                aos_memmove(&pstTempTab->puiPlmnSort[uiTempSortIndex+1],
                            &pstTempTab->puiPlmnSort[uiTempSortIndex],
                            (pstTempTab->uiUsedCount - uiTempSortIndex) *
                            sizeof(aos_plmn_sort));
            }
            pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn = uiPlmn;
            pstTempTab->puiPlmnSort[uiTempSortIndex].uiTabIndex =
                pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
        }
        else {
            if (pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn < uiPlmn) {
                for (uiTempSortIndex = uiTempSortIndex+1;
                     uiTempSortIndex < pstTempTab->uiUsedCount;
                     uiTempSortIndex++) {
                    if (pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn >
                        uiPlmn)
                        break;
                }

                if (uiTempSortIndex < pstTempTab->uiUsedCount) {
                    aos_memmove(&pstTempTab->puiPlmnSort[uiTempSortIndex+1],
                                &pstTempTab->puiPlmnSort[uiTempSortIndex],
                                (pstTempTab->uiUsedCount - uiTempSortIndex) *
                                sizeof(aos_plmn_sort));
                }
                pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn = uiPlmn;
                pstTempTab->puiPlmnSort[uiTempSortIndex].uiTabIndex =
                    pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
            }
            else {
                while (1) {
                    if(pstTempTab->puiPlmnSort[uiTempSortIndex].uiPlmn<uiPlmn) {
                        if (uiTempSortIndex != pstTempTab->uiUsedCount-1) {
                            aos_memmove(&pstTempTab->puiPlmnSort[uiTempSortIndex+2],
                                        &pstTempTab->puiPlmnSort[uiTempSortIndex+1],
                                        (pstTempTab->uiUsedCount - uiTempSortIndex - 1) *
                                        sizeof(aos_plmn_sort));
                        }
                        pstTempTab->puiPlmnSort[uiTempSortIndex+1].uiPlmn = uiPlmn;
                        pstTempTab->puiPlmnSort[uiTempSortIndex+1].uiTabIndex =
                            pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
                        break;
                    }

                    if (uiTempSortIndex == 0) {
                        aos_memmove(&pstTempTab->puiPlmnSort[1],
                                    &pstTempTab->puiPlmnSort[0],
                                    pstTempTab->uiUsedCount *
                                    sizeof(aos_plmn_sort));
                        pstTempTab->puiPlmnSort[0].uiPlmn = uiPlmn;
                        pstTempTab->puiPlmnSort[0].uiTabIndex =
                            pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
                        break;
                    }
                    uiTempSortIndex--;
                }
            }
        }

        ret = aos_find_freq_sort(usFreq, pstTempTab->puiFreqSort,
                                 pstTempTab->uiUsedCount, &uiTempSortIndex);
        if (ret == AOS_SUCCESS) {
            for(;uiTempSortIndex < pstTempTab->uiUsedCount; uiTempSortIndex++) {
                if (pstTempTab->puiFreqSort[uiTempSortIndex].usFreq == usFreq)
                    continue;
                break;
            }
            if (uiTempSortIndex < pstTempTab->uiUsedCount) {
                aos_memmove(&pstTempTab->puiFreqSort[uiTempSortIndex+1],
                            &pstTempTab->puiFreqSort[uiTempSortIndex],
                            (pstTempTab->uiUsedCount - uiTempSortIndex) *
                            sizeof(aos_freq_sort));
            }
            pstTempTab->puiFreqSort[uiTempSortIndex].usFreq = usFreq;
            pstTempTab->puiFreqSort[uiTempSortIndex].uiTabIndex =
                pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
        }
        else {
            if (pstTempTab->puiFreqSort[uiTempSortIndex].usFreq < usFreq) {
                for (uiTempSortIndex = uiTempSortIndex+1;
                     uiTempSortIndex < pstTempTab->uiUsedCount;
                     uiTempSortIndex++) {
                    if(pstTempTab->puiFreqSort[uiTempSortIndex].usFreq > usFreq)
                        break;
                }

                if (uiTempSortIndex < pstTempTab->uiUsedCount) {
                    aos_memmove(&pstTempTab->puiFreqSort[uiTempSortIndex+1],
                                &pstTempTab->puiFreqSort[uiTempSortIndex],
                                (pstTempTab->uiUsedCount - uiTempSortIndex) *
                                sizeof(aos_freq_sort));
                }
                pstTempTab->puiFreqSort[uiTempSortIndex].usFreq = usFreq;
                pstTempTab->puiFreqSort[uiTempSortIndex].uiTabIndex =
                    pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
            } else {
                while (1) {
                    if (pstTempTab->puiFreqSort[uiTempSortIndex].usFreq<usFreq){
                        if (uiTempSortIndex != pstTempTab->uiUsedCount-1) {
                            aos_memmove(&pstTempTab->puiFreqSort[uiTempSortIndex+2],
                                        &pstTempTab->puiFreqSort[uiTempSortIndex+1],
                                        (pstTempTab->uiUsedCount - uiTempSortIndex - 1) *
                                        sizeof(aos_freq_sort));
                        }
                        pstTempTab->puiFreqSort[uiTempSortIndex+1].usFreq = usFreq;
                        pstTempTab->puiFreqSort[uiTempSortIndex+1].uiTabIndex =
                            pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
                        break;
                    }
                    if (uiTempSortIndex == 0) {
                        aos_memmove(&pstTempTab->puiFreqSort[1],
                                    &pstTempTab->puiFreqSort[0],
                                    pstTempTab->uiUsedCount *
                                    sizeof(aos_freq_sort));
                        pstTempTab->puiFreqSort[0].usFreq = usFreq;
                        pstTempTab->puiFreqSort[0].uiTabIndex =
                            pstPlmnFreqNode - pstTempTab->pstPlmnFreqInfo;
                        break;
                    }
                    uiTempSortIndex--;
                }
            }
        }
    }

    pstTempTab->uiUsedCount++;
}

aos_status aos_find_plmn_sort(u32 uiPlmnValue, aos_plmn_sort* auiSort,
                              u32 uiFindLen,u32 *puiIndex)
{
    u32 uiSortMid;
    u32 uiStartIndex = 0;

    while (uiFindLen > 1) {
        uiSortMid = uiFindLen/2;
        if (uiPlmnValue == auiSort[uiStartIndex + uiSortMid -1].uiPlmn) {
            *puiIndex = uiStartIndex + uiSortMid - 1;
            return AOS_SUCCESS;
        }
        else if (uiPlmnValue > auiSort[uiStartIndex + uiSortMid -1].uiPlmn) {
            uiStartIndex = uiStartIndex+uiSortMid;
            uiFindLen = uiFindLen-uiSortMid;
        }
        else
            uiFindLen = uiSortMid;
    }

    if (uiFindLen == 1) {
        *puiIndex = uiStartIndex;
        if (uiPlmnValue == auiSort[uiStartIndex].uiPlmn)
            return AOS_SUCCESS;
        else
            return AOS_ERROR;
    }
    *puiIndex = 0;
    return AOS_ERROR;
}

aos_status aos_find_freq_sort(u16 uiFreqValue, aos_freq_sort* auiSort,
                              u32 uiFindLen,u32 *puiIndex)
{
    u32 uiSortMid;
    u32 uiStartIndex = 0;

    while (uiFindLen > 1) {
        uiSortMid = uiFindLen/2;

        if (uiFreqValue == auiSort[uiStartIndex + uiSortMid -1].usFreq) {
            *puiIndex = uiStartIndex + uiSortMid -1;
            return AOS_SUCCESS;
        }
        else if (uiFreqValue > auiSort[uiStartIndex + uiSortMid -1].usFreq) {
            uiStartIndex = uiStartIndex+uiSortMid;
            uiFindLen = uiFindLen-uiSortMid;
        } else
            uiFindLen = uiSortMid;
    }

    if (uiFindLen == 1) {
        *puiIndex = uiStartIndex;
        if (uiFreqValue == auiSort[uiStartIndex].usFreq) {
            return AOS_SUCCESS;
        }
        else {
            return AOS_ERROR;
        }
    }
    *puiIndex = 0;
    return AOS_ERROR;
}

