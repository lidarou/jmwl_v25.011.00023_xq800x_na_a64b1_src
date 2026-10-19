/*----------------------------------------------------------------------------
 *
 * File:
 * eas_synthcfg.h
 *
 * Contents and purpose:
 * Defines for various synth configurations
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 664 $
 *   $Date: 2007-04-25 16:11:22 -0400 (Wed, 25 Apr 2007) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_SYNTHCFG_H
#define _EAS_SYNTHCFG_H

#include "audio_config.h"

#if defined(EAS_WT_SYNTH)
#define _WT_SYNTH

/* FM on MCU */
#elif defined(EAS_FM_SYNTH)
#define _FM_SYNTH

/* wavetable drums and FM melodic on MCU */
#elif defined(EAS_HYBRID_SYNTH)
#define _WT_SYNTH
#define _FM_SYNTH
#define _SECONDARY_SYNTH
#define _HYBRID_SYNTH

/* wavetable drums on MCU, wavetable melodic on DSP */
#elif defined(EAS_SPLIT_WT_SYNTH)
#define _WT_SYNTH
#define _SPLIT_ARCHITECTURE

/* wavetable drums on MCU, FM melodic on DSP */
#elif defined(EAS_SPLIT_HYBRID_SYNTH)
#define _WT_SYNTH
#define _FM_SYNTH
#define _SECONDARY_SYNTH
#define _SPLIT_ARCHITECTURE
#define _HYBRID_SYNTH

/* FM synth on DSP */
#elif defined(EAS_SPLIT_FM_SYNTH)
#define _FM_SYNTH
#define _SPLIT_ARCHITECTURE

#else
#error "Unrecognized architecture option"
#endif

#endif

