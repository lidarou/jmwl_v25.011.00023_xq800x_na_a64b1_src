

#ifndef __BLACKLIST_H__
#define __BLACKLIST_H__
#define BLACKLIST_NUMBER_LEN    30


#define BLACKLIST_FLASH_MAX_COUNT 50

#if 0
typedef struct _BLACKLIST_HEAD_H_ {
    uint32_t        flag;
    uint16_t        count;
    uint16_t        used;

} blacklist_head_t;


typedef struct _BLACKLIST_PARAM_H_ {
    uint8_t         number[BLACKLIST_NUMBER_LEN + 2];
    uint16_t        index;
} blacklist_param_t;


#endif


enum {
    BLACKLIST_RET_FS = -2,
    BLACKLIST_RET_ERROR = -1,
    BLACKLIST_RET_OK,
};


typedef struct _BLACKLIST_PARAM_H_ {
    uint8_t         number[BLACKLIST_NUMBER_LEN + 2];
} blacklist_param_t;


typedef struct _BLACKLIST_HEAD_H_ {
    uint32_t        flag;
    uint32_t        count;

} blacklist_head_t;


typedef struct _BLACKLIST_DATA_H_ {
    blacklist_head_t head;
    blacklist_param_t num_array[0];

} blacklist_data_t;


#define BLACKLIST_PATH          "/C/phonebook/blacklist.bin"
#define BLACKLIST_HEAD_FLAG     0x12345679


blacklist_data_t * blacklist_get_flash_data(void);
int32_t blacklist_wirte_flash_data(blacklist_data_t * p_blacklist_data,uint32_t size);
int32_t blacklist_add_flash_data(blacklist_param_t * p_blacklist_param,uint32_t size);
int32_t blacklist_have_this_number(blacklist_data_t * *pp_blacklist_data,uint32_t * p_index,uint8_t * p_number);

#endif

