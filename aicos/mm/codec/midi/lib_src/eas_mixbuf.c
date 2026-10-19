/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_mixbuf.c
 *
 * Contents and purpose:
 * Contains a data allocation for synthesizer
 *			
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 82 $
 *   $Date: 2006-07-10 14:45:19 -0400 (Mon, 10 Jul 2006) $
 *----------------------------------------------------------------------------
*/

// includes

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07b1f0
#define LOG_MODULE    AMOD_AUDIO
#include "eas_data.h"
#include "eas_mixer.h"

// globals
EAS_I32 eas_MixBuffer[BUFFER_SIZE_IN_MONO_SAMPLES * NUM_OUTPUT_CHANNELS];

