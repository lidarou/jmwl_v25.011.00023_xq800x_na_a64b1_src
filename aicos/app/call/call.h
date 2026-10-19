/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the APIs of call.
 */

#ifndef __CALL_H__
#define __CALL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
*      INCLUDES
*********************/

/*********************
*      DEFINES
*********************/
#define CONTACT_NAME_LEN_MAX                    (10)
#define TELE_PHONE_NUMBER_LEN_MAX               (32)
/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    CALL_ORIGIN_DIALDER,         /* 0, dialer */
    CALL_ORIGIN_GLOBAL_INCOMING, /* 1, global call */
    CALL_ORIGIN_CALLLOG,         /* 2, calllog */
    CALL_ORIGIN_CONTACT,         /* 3, contact */
}call_origin_e;

/* TODO call_enter_param_t can be used by other app */
typedef struct {
    call_origin_e call_origin;                     /* call origin */
    char phone_num[TELE_PHONE_NUMBER_LEN_MAX];     /* phone number */
    char contact_name[CONTACT_NAME_LEN_MAX];       /* contact name*/
    uint8_t phone_num_type;                        /* 0:normal number;1:service number;3:emergency number */
    /* TODO: add contact image id;ring type;number location; */
}call_enter_param_t;

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
* GLOBAL PROTOTYPES
**********************/

/**********************
*   GLOBAL FUNCTIONS
**********************/

/**
 * app_call_init() - the init of call.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_call_init(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __CALL_H__ */
