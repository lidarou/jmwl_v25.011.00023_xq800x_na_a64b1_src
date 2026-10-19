/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AOS LIST Interface.
 */

#ifndef __AOS_LIST_H__
#define __AOS_LIST_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_osa.h>
#include "aos_mem.h"
#ifndef CONFIG_SDK_ENABLE
#include <../../../aicos/comm/ps/code/l2/inc/l2_pub_mem.h>
#endif


/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/
#define AOS_UNAVAILABLE_HASH_ITEM           ((aos_hash_item)-1)
#define AOS_IPDATA_RESERVE                  4


#define aos_get_next_mbuf_list(mbuf)                      \
  ((mbuf)->next_list)

#define aos_get_next_mbuf(mbuf)                           \
  ((mbuf)->next_ptr)

#define aos_get_mbuf_dataptr(mbuf)                        \
  ((mbuf)->data_ptr + (mbuf)->data_offset)

#define aos_get_mbuf_datalen(mbuf)                        \
  ((mbuf)->data_len)

#define aos_set_mbuf_datalen(mbuf, new_datalen)           \
  (aos_get_mbuf_datalen(mbuf) = new_datalen)

#define aos_set_mbuf_offset_datalen(mbuf, new_datalen)    \
  (aos_get_mbuf_datalen(mbuf) = AOS_IPDATA_RESERVE + (new_datalen))

#define aos_get_mbuf_offset_dataptr(mbuf)                 \
  (((mbuf)->data_ptr + (mbuf)->data_offset) + AOS_IPDATA_RESERVE)

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/
/* hash table */
typedef u32             aos_list_index;
typedef u32             aos_hash_length;
typedef u32             aos_hash_key;
typedef u32             aos_hash_item;
typedef u32             aos_hash_tbl;
typedef aos_list_index (*aos_hash_key_func)(aos_hash_key, aos_hash_length);
typedef void (*aos_hash_free)(aos_hash_item);

typedef struct aos_hash_used_list_st {
    aos_list_index      head;
} aos_hash_used_list;

typedef struct aos_hash_node_st {
    aos_list_index      next;
    aos_hash_key        key;
    aos_hash_item       item;
} aos_hash_node;

typedef struct aos_hash_tcb_st {
    aos_hash_key_func   key_func;
    aos_list_index      max_hash_item_nums;
    aos_list_index      free_head;
    aos_hash_node*      first_node_ptr;
    aos_mutex_ptr       mutex_ptr;
    aos_hash_used_list* list;
} aos_hash_tcb;

/* blist */
typedef u16             aos_blist_index;
typedef u32             aos_blist_item;
typedef void (*aos_blist_free)(aos_blist_item item);

typedef struct aos_blist_node_st {
    aos_blist_index     next, prev;
    aos_blist_item      item;
} aos_blist_node;

typedef struct aos_blist_st {
    aos_blist_index     free_head;
    aos_blist_index     head, tail;
    aos_blist_index     max_node_nums;
    aos_mutex_ptr       mutex_ptr;
    aos_blist_node*     node;
} aos_blist;

/* ring buffer */
typedef void (*aos_ring_buffer_ind_handl)(void);

typedef struct aos_ring_buffer_bsptr_st {
    /* buffer total length is sizeof(u8*) * uiBllockNum */
    /* read and write unit is sizeof(u8*) */
    u32  uiBllockNum;
    u8   **ppucBuffer;
    u32  uiWriteCount;
    u32  uiWriteIndex;
    u32  uiReadCount;
    u32  uiReadIndex;
} aos_ring_buffer_bsptr;

/* plmn freq */
typedef u32  aos_plmn_freq_tab;

typedef struct aos_plmn_freq_info_st {
    u32                             uiPlmn;
    u16                             usFreq;
    struct aos_plmn_freq_info_st    *pstNext;
    struct aos_plmn_freq_info_st    *pstPrev;
} aos_plmn_freq_info;

typedef struct aos_plmn_sort_st {
    u32                  uiPlmn;
    u32                  uiTabIndex;
} aos_plmn_sort;

typedef struct aos_freq_sort_st {
    u16                  usFreq;
    u32                  uiTabIndex;
} aos_freq_sort;

typedef struct aos_plmn_freq_st {
    u32                  uiMaxCount;
    u32                  uiUsedCount;
    aos_plmn_freq_info   *pstPlmnFreqInfo;
    aos_plmn_freq_info   *pstUsedHeadNode;
    aos_plmn_freq_info   *pstUsedTailNode;
    aos_plmn_freq_info   *pstFreeHeadNode;
    aos_plmn_sort        *puiPlmnSort;
    aos_freq_sort        *puiFreqSort;
} aos_plmn_freq;

/**---------------------------------------------------------------------------*/
/**                          function prototype                               */
/**---------------------------------------------------------------------------*/
/* hash table */
aos_hash_tbl *aos_create_hash_tbl(aos_hash_length, aos_hash_key_func, boolean);
aos_status aos_insert_hash_item(aos_hash_tbl *, aos_hash_key, aos_hash_item);
aos_status aos_find_hash_item(aos_hash_tbl *, aos_hash_key, aos_hash_item*);
aos_status aos_replace_hash_item(aos_hash_tbl *, aos_hash_key, aos_hash_item,
                                 aos_hash_item*);
aos_status aos_remove_hash_item(aos_hash_tbl *, aos_hash_key, aos_hash_item*);
aos_status aos_delete_hash_tbl(aos_hash_tbl *, aos_hash_free);

/* blist */
aos_blist *aos_create_blist(u16, boolean);
aos_status aos_add_tail_blist(aos_blist *, aos_blist_item);
aos_status aos_add_head_blist(aos_blist *, aos_blist_item);
aos_status aos_retrieve_tail_blist(aos_blist *, aos_blist_item *);
aos_status aos_retrieve_head_blist(aos_blist *, aos_blist_item *);
aos_blist_node *aos_head_blist_node(aos_blist *);
aos_blist_node *aos_next_blist_node(aos_blist *, aos_blist_node *);
aos_blist_node *aos_tail_blist_node(aos_blist *);
aos_blist_node *aos_prev_blist_node(aos_blist *, aos_blist_node *);
aos_blist_item aos_get_blist_item(aos_blist_node *);
aos_status aos_replace_blist_item(aos_blist *, aos_blist_node *,
                                  aos_blist_item, aos_blist_item *);
aos_status aos_insert_before_blist_node(aos_blist *, aos_blist_node *,
                                        aos_blist_item);
aos_status aos_insert_after_blist_node(aos_blist *, aos_blist_node *,
                                       aos_blist_item);
aos_status aos_remove_blist_node(aos_blist *, aos_blist_node *);
aos_status aos_delete_blist(aos_blist *, aos_blist_free);

/* ring buffer */
aos_ring_buffer_bsptr *aos_ring_buffer_bsptr_init(u32);
u32 aos_ring_buffer_bsptr_read(aos_ring_buffer_bsptr *, u8 **, u32);
aos_status aos_ring_buffer_bsptr_write(aos_ring_buffer_bsptr *,
                                       u8 *, boolean *);
void aos_ring_buffer_bsptr_delete(aos_ring_buffer_bsptr *);

/* plmn freq */
void aos_delete_plmn_freq_sub(aos_plmn_freq *pstTempTab,
                              aos_plmn_freq_info *pstPlmnFreqNode);
void aos_insert_plmn_freq_sub(aos_plmn_freq *pstTempTab, u32 uiPlmn,
                              u16 usFreq);
aos_status aos_find_plmn_sort(u32 uiPlmnValue, aos_plmn_sort *auiSort,
                              u32 uiFindLen,u32 *puiIndex);
aos_status aos_find_freq_sort(u16 uiFreqValue, aos_freq_sort *auiSort,
                              u32 uiFindLen,u32 *puiIndex);
aos_plmn_freq_tab aos_create_plmn_freq_tab(u32 uiMaxCount);
aos_status aos_insert_plmn_freq(aos_plmn_freq_tab pstPlmnFreqTab,
                                u32 uiPlmn, u16 usFreq);
aos_status aos_delete_plmn(aos_plmn_freq_tab pstPlmnFreqTab, u32 uiPlmn);
aos_status aos_delete_freq(aos_plmn_freq_tab pstPlmnFreqTab, u16 usFreq);
aos_status aos_delete_freq_area(aos_plmn_freq_tab pstPlmnFreqTab,
                                u16 usFreqMin, u16 usFreqMax);
aos_status aos_get_spec_plmn_freq(aos_plmn_freq_tab pstPlmnFreqTab, u32 uiPlmn,
                                  u16 *pauiOutInfo, u32 uiMaxCount,
                                  u32 *puiOutNum);
aos_status aos_get_all_freq(aos_plmn_freq_tab pstPlmnFreqTab,u16 *pauiOutInfo,
                            u32 uiMaxCount, u32 *puiOutNum);
aos_status aos_get_all_plmn(aos_plmn_freq_tab pstPlmnFreqTab,u32 *pauiOutInfo,
                            u32 uiMaxCount, u32 *puiOutNum);
aos_status aos_clean_plmn_freq(aos_plmn_freq_tab pstPlmnFreqTab);
aos_status aos_delete_plmn_freq(aos_plmn_freq_tab pstPlmnFreqTab);

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __AOS_LIST_H__ */

