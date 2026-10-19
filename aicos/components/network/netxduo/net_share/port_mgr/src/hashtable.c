/*******************************************************************************/
/*                                                                             */
/* Copyright (c) Microsoft Corporation. All rights reserved.                   */
/*                                                                             */
/* This software is licensed under the Microsoft Software License.             */
/* Full text of the license can be found in the LICENSE file at                */
/* https://aka.ms/AzureDefenderForIoT_EULA and in the root directory           */
/* of this software.                                                           */
/*                                                                             */
/*******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x138757
#define LOG_MODULE    AMOD_NET

#include <string.h>
#include <aic_osa.h>
#include "hashtable.h"

static unsigned int _hash_calc(hashtable_t *hashtable, const void *key);
static void *_hashtable_find(hashtable_t *hashtable, const void *key, unsigned int *index);

// API functions
bool hashtable_init(hashtable_t *hashtable, size_t size, hash_func_t hash_func,
    hash_equal_func_t hash_equal_func, hash_update_func_t hash_update_func)
{
    if (!hashtable || !size) {
        aicx_error(0x1c3864, "", "Wrong hashtable init parametrs");
        return false;
    }
    hashtable->ctrl.size = size;
    hashtable->ctrl.hash_func = hash_func;
    hashtable->ctrl.hash_equal_func = hash_equal_func ? hash_equal_func : hashtable_equal_default_func;
    hashtable->ctrl.hash_update_func = hash_update_func ? hash_update_func : hashtable_update_none_func;
    //memset(hashtable->table, 0, size * sizeof(void *));
    hashtable->ctrl.initialized = true;
    return true;
}

void *hashtable_insert(hashtable_t *hashtable, const void *key, void *item)
{
    collection_item_t *element = item;
    unsigned int index = 0;
    collection_item_t *existing_element;

    if (!hashtable || !hashtable->ctrl.initialized || !key || !item) {
        aicx_error(0x0f2af1, "", "Wrong parametrs");
        return NULL;
    }

    //alog_info("hashtable_insert: table inited\n");
    //collection_item_t *element = item;
    //unsigned int index = 0;
    //collection_item_t *existing_element = _hashtable_find(hashtable, key, &index);
    existing_element = _hashtable_find(hashtable, key, &index);

    if (existing_element == NULL) {
        collection_item_t *column = hashtable->table[index];
        element->previous = NULL;
        element->next = column;
        if (column != NULL) {
            column->previous = element;
        }
        hashtable->table[index] = element;
        //alog_info("hashtable_insert:index=%d,element=0x%x\n", index, element);
    } else {
        hashtable->ctrl.hash_update_func(existing_element, element);
    }

    return item;
}

void *hashtable_remove(hashtable_t *hashtable, const void *key, hash_free_func_t hash_free_func)
{
    unsigned int index = 0;
    collection_item_t *existing_element;
    collection_item_t *previous_element;
    collection_item_t *next_element;

    if (!hashtable || !hashtable->ctrl.initialized || !key) {
        aicx_error(0x0f2af1, "", "Wrong parametrs");
        return NULL;
    }
    //unsigned int index = 0;
    //collection_item_t *existing_element = _hashtable_find(hashtable, key, &index);
    existing_element = _hashtable_find(hashtable, key, &index);

    if (existing_element == NULL) {
        return NULL;
    }
    //collection_item_t *previous_element = existing_element->previous;
    //collection_item_t *next_element = existing_element->next;
    previous_element = existing_element->previous;
    next_element = existing_element->next;

    if (next_element != NULL) {
        next_element->previous = previous_element;
    }
    if (previous_element == NULL) {
        hashtable->table[index] = next_element;
    } else {
        previous_element->next = next_element;
    }
    if (hash_free_func) {
        hash_free_func(existing_element);
        return NULL;
    }
    return existing_element;
}

void *hashtable_find(hashtable_t *hashtable, const void *key)
{
    unsigned int index;

    if (!hashtable || !hashtable->ctrl.initialized || !key) {
        aicx_error(0x0f2af1, "", "Wrong parametrs");
        return NULL;
    }
    //unsigned int index;
    return _hashtable_find(hashtable, key, &index);
}

size_t hashtable_get_depth(hashtable_t *hashtable, size_t *count)
{
    size_t max = 0;
    size_t i;

    if (count) {
        *count = 0;
    }
    if (!hashtable || !hashtable->ctrl.initialized) {
        return 0;
    }
    for (i = 0; i < hashtable->ctrl.size; i++) {
        collection_item_t *current_element = NULL;
        size_t cnt = 0;
        hashtable_column_foreach(hashtable, current_element, i) {
            cnt++;
        }
        if (cnt > 1) {
            if (cnt > max) {
                max = cnt;
            }
            if (count) {
                (*count)++;
            }
        }
    }
    return max;
}

void hashtable_foreach(hashtable_t *hashtable, hash_action_func_t hash_action_func, void *ctx)
{
    unsigned int i;

    if (!hashtable || !hashtable->ctrl.initialized || !hash_action_func) {
        return;
    }
    for (i = 0; i < hashtable->ctrl.size; i++) {
        // not using hashtable_column_foreach() because of 'tmp = current_element->next for safe action'
        collection_item_t *current_element = hashtable->table[i]; // get column head
        while (current_element != NULL) {
            collection_item_t *tmp = current_element->next;
            hash_action_func(current_element, ctx);
            current_element = tmp;
        }
    }
}

void hashtable_flush(hashtable_t *hashtable, hash_free_func_t hash_free_func)
{
    size_t i;

    if (!hashtable || !hashtable->ctrl.initialized) {
        return;
    }
    if (!hash_free_func) {
        goto cleanup;
    }
    for (i = 0; i < hashtable->ctrl.size; i++) {
        // not using hashtable_column_foreach() because of 'tmp = current_element->next for safe free'
        collection_item_t *current_element = hashtable->table[i]; // get column head
        while (current_element != NULL) {
            collection_item_t *tmp = current_element->next;
            hash_free_func(current_element);
            current_element = tmp;
        }
    }
cleanup:
    memset(hashtable->table, 0, hashtable->ctrl.size * sizeof(void *));
}

bool hashtable_is_empty(hashtable_t *hashtable)
{
    size_t i;

    if (!hashtable || !hashtable->ctrl.initialized) {
        return true;
    }
    for (i = 0; i < hashtable->ctrl.size; i++) {
        collection_item_t *column = hashtable->table[i];
        if (column) {
            return false;
        }
    }
    return true;
}

// Service callbacks and functions
void hashtable_update_none_func(void *a, void *b)
{
    aicx_error(0x0fef6a, "", "Should never happens");
}

bool hashtable_equal_default_func(const void *key, void *item)
{
    return (key == item);
}

unsigned int hashtable_hash_str_default(const void *key)
{
    const char *buffer = key;
    return hashtable_buffer2hash(buffer, strlen(buffer));
}

unsigned int hashtable_buffer2hash(const char *buffer, size_t len)
{
    unsigned int hash = 5381;
    size_t i;

    for (i = 0 ; i < len; i++) {
        hash = ((hash << 5) + hash) + (unsigned int)buffer[i];
    }

    return hash;
}

// Internal functions
static unsigned int _hash_calc(hashtable_t *hashtable, const void *key)
{
    return hashtable->ctrl.hash_func ? (unsigned int)(hashtable->ctrl.hash_func(key) % hashtable->ctrl.size) :
        (unsigned int)((uintptr_t)key % hashtable->ctrl.size);
}

static void *_hashtable_find(hashtable_t *hashtable, const void *key, unsigned int *index)
{
    collection_item_t *current_element = NULL;

    *index = _hash_calc(hashtable, key);
    //collection_item_t *current_element = NULL;
    //alog_info("_hashtable_find: calc index=%d\n", *index);

    hashtable_column_foreach(hashtable, current_element, *index) {
        //alog_info("_hashtable_find:foreach:current_element=0x%x,index=%d\n",
                 //current_element, *index);
        if (hashtable->ctrl.hash_equal_func(key, current_element) != 0) {
            break;
        }
    }

    return current_element;
}

