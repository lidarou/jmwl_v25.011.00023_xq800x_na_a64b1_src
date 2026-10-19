/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
/****************************************************************************************
Portions of this file are derived from the following 3GPP standard:

    3GPP TS 26.173
    ANSI-C code for the Adaptive Multi-Rate - Wideband (AMR-WB) speech codec
    Available from http://www.3gpp.org

(C) 2007, 3GPP Organizational Partners (ARIB, ATIS, CCSA, ETSI, TTA, TTC)
Permission to distribute, modify and use this file under the standard license
terms listed above has been obtained from the copyright holder.
****************************************************************************************/
/*
------------------------------------------------------------------------------

 Name: amrwb_codec-api.h

     Date: 05/02/2007

------------------------------------------------------------------------------
 REVISION HISTORY


 Description:

------------------------------------------------------------------------------
 INCLUDE DESCRIPTION

 Main header file for the Packet Video AMR Wide  Band  decoder library. The
 constants, structures, and functions defined within this file, along with
 a basic data types header file, is all that is needed to use and communicate
 with the library. The internal data structures within the library are
 purposely hidden.

 ---* Need description of the input buffering. *-------

 ---* Need an example of calling the library here *----

------------------------------------------------------------------------------
 REFERENCES

  (Normally header files do not have a reference section)

------------------------------------------------------------------------------
*/

/*----------------------------------------------------------------------------
; CONTINUE ONLY IF NOT ALREADY DEFINED
----------------------------------------------------------------------------*/
#ifndef _AMR_CODEC_API_H
#define _AMR_CODEC_API_H


/*----------------------------------------------------------------------------
; INCLUDES
----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

#include "aicxtek_audio_api.h"

    /*----------------------------------------------------------------------------
    ; MACROS
    ; Define module specific macros here
    ----------------------------------------------------------------------------*/

    /*----------------------------------------------------------------------------
    ; DEFINES
    ; Include all pre-processor statements here.
    ----------------------------------------------------------------------------*/

#define AMR_TX_WMF 0
#define AMR_TX_IF2 1
#define AMR_TX_ETS 2

/*!* the frame format the codec supports*/
typedef enum {
    VOAMRWB_DEFAULT = 0,	/*!< the frame type is the header (defined in RFC3267) + rawdata*/
    /*One word (2-byte) for sync word (0x6b21)*/
    /*One word (2-byte) for frame length N.*/
    /*N words (2-byte) containing N bits (bit 0 = 0x007f, bit 1 = 0x0081).*/
    VOAMRWB_ITU = 1,
    /*One word (2-byte) for sync word (0x6b21).*/
    /*One word (2-byte) to indicate the frame type.*/
    /*One word (2-byte) to indicate the mode.*/
    /*N words  (2-byte) containing N bits (bit 0 = 0xff81, bit 1 = 0x007f).*/
    VOAMRWB_RFC3267 = 2,	/* see RFC 3267 */
    VOAMRWB_TMAX
}VOAMRWBFRAMETYPE;




typedef enum {
    AMR_CMD_GET_FROMAT = 1,
    AMR_CMD_GET_CHANNELS,
    AMR_CMD_GET_SAMPLERATE,
    AMR_CMD_SET_FRAMETYPE,
    AMR_CMD_SET_MODE,
    AMR_CMD_SET_DTX,
    AMR_CMD_SET_OUTFORMAT,
    AMR_CMD_SET_DECMODE,
    AMR_CMD_NB_SET_BIT_FORMAT,
}amr_control_cmd_t;

typedef struct enc_init_param {
    int enc_mode;
    union {
        int frameType;
        int output_format;
    };
    int allow_dtx;
    void* scratch_ptr;
    void* log_func;
}amr_enc_initparam_t;

enum {
    AMR_CODEC_SUCCESS = 0,
    AMR_CODEC_MODE_ERR = -1,
    AMR_CODEC_OUT_BUFF_SMALL = -2,
    AMR_CODEC_IN_BUFF_SMALL = -3,
    AMR_CODEC_OUT_OF_SCRATCH = -4,
    AMR_CODEC_PROC_ERR = -5,
    AMR_CODEC_CMD_ERR = -6,
    AMR_CODEC_NULL_POINTER = -7,
    AMR_CODEC_ENC_FORMAT_ERR = -8,
};

enum {
    AMR_DEC_MODE_NORMAL,
    AMR_DEC_MODE_FRAME,
    AMR_DEC_MODE_MAX,
};

typedef struct dec_init_param {
    int dec_mode;
    void* scratch_ptr;
    void* log_func;
}amr_dec_initparam_t;

void * amrwb_dec_create(memory_api_t *p_memory, void *init_param, unsigned int param_size32);
void   amrwb_dec_delete(void *handle);
int    amrwb_dec_process(void *handle, char *in_buf, unsigned int in_size8, char *out_buf, unsigned int *io_out_size8);
int    amrwb_dec_control(void *handle, int param_code, int *p_param, unsigned int param_size32);

void* amrwb_enc_create(memory_api_t* p_memory, void* init_param, unsigned int param_size32);
void  amrwb_enc_delete(void* handle);
int   amrwb_enc_process(void* handle, char* in_buf, unsigned int in_size8, char* out_buf, unsigned int* io_out_size8);
int   amrwb_enc_control(void* handle, int param_code, int* p_param, unsigned int param_size32);


void* amrnb_dec_create(memory_api_t* p_memory, void* init_param, unsigned int param_size32);
void   amrnb_dec_delete(void* handle);
int    amrnb_dec_process(void* handle, char* in_buf, unsigned int in_size8, char* out_buf, unsigned int* io_out_size8);
int    amrnb_dec_control(void* handle, int param_code, int* p_param, unsigned int param_size32);

void* amrnb_enc_create(memory_api_t* p_memory, void* init_param, unsigned int param_size32);
void   amrnb_enc_delete(void* handle);
int    amrnb_enc_process(void* handle, char* in_buf, unsigned int in_size8, char* out_buf, unsigned int* io_out_size8);
int    amrnb_enc_control(void* handle, int param_code, int* p_param, unsigned int param_size32);

#ifdef __cplusplus
}
#endif


#endif  /* PVMP4AUDIODECODER_API_H */


