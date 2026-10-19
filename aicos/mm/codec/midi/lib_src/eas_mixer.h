/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_mixer.h
 *
 * Contents and purpose:
 * This file contains the critical components of the mix engine that
 * must be optimized for best performance.
 *			
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 706 $
 *   $Date: 2007-05-31 20:22:51 -0400 (Thu, 31 May 2007) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_MIXER_H
#define _EAS_MIXER_H

//3 dls: This module is in the midst of being converted from a synth
//3 specific module to a general purpose mix engine

#define MIX_FLAGS_STEREO_SOURCE		1
#define MIX_FLAGS_STEREO_OUTPUT		2
#define NUM_MIXER_GUARD_BITS		4

#include "eas_effects.h"

extern void SynthMasterGain( long *pInputBuffer, EAS_PCM *pOutputBuffer, EAS_U16 nGain, EAS_U16 nNumLoopSamples);

/*----------------------------------------------------------------------------
 * EAS_MixEngineInit()
 *----------------------------------------------------------------------------
 * Purpose:
 * Prepares the mix engine for work, allocates buffers, locates effects modules, etc.
 *
 * Inputs:
 * pEASData		 	- instance data
 * pInstData		- pointer to variable to receive instance data handle
 *
 * Outputs:
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
*/
EAS_RESULT EAS_MixEngineInit (EAS_DATA_HANDLE pEASData);

/*----------------------------------------------------------------------------
 * EAS_MixEnginePrep()
 *----------------------------------------------------------------------------
 * Purpose:
 * Performs prep before synthesize a buffer of audio, such as clearing
 * audio buffers, etc.
 *
 * Inputs:
 * psEASData - pointer to overall EAS data structure
 *
 * Outputs:
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
*/
void EAS_MixEnginePrep (EAS_DATA_HANDLE pEASData, EAS_I32 nNumSamplesToAdd);

/*----------------------------------------------------------------------------
 * EAS_MixEnginePost
 *----------------------------------------------------------------------------
 * Purpose: 
 * This routine does the post-processing after all voices have been
 * synthesized. It calls any sweeteners and does the final mixdown to
 * the output buffer.
 * 
 * Inputs: 
 *			
 * Outputs:
 *
 * Notes:
 *----------------------------------------------------------------------------
*/
void EAS_MixEnginePost (EAS_DATA_HANDLE pEASData, EAS_I32 nNumSamplesToAdd);

/*----------------------------------------------------------------------------
 * EAS_MixEngineShutdown()
 *----------------------------------------------------------------------------
 * Purpose:
 * Shuts down effects modules and deallocates memory
 *
 * Inputs:
 * pEASData		 	- instance data
 * pInstData		- instance data handle
 *
 * Outputs:
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
*/
EAS_RESULT EAS_MixEngineShutdown (EAS_DATA_HANDLE pEASData);

#ifdef UNIFIED_MIXER
/*----------------------------------------------------------------------------
 * EAS_MixStream
 *----------------------------------------------------------------------------
 * Mix a 16-bit stream into a 32-bit buffer
 *
 * pInputBuffer	16-bit input buffer
 * pMixBuffer	32-bit mix buffer
 * numSamples	number of samples to mix
 * gainLeft		initial gain left or mono
 * gainRight	initial gain right
 * gainLeft		left gain increment per sample
 * gainRight	right gain increment per sample
 * flags		bit 0 = stereo source
 * 				bit 1 = stereo output
 *----------------------------------------------------------------------------
*/
void EAS_MixStream (EAS_PCM *pInputBuffer, EAS_I32 *pMixBuffer, EAS_I32 numSamples, EAS_I32 gainLeft, EAS_I32 gainRight, EAS_I32 gainIncLeft, EAS_I32 gainIncRight, EAS_I32 flags);
#endif

#endif /* #ifndef _EAS_MIXER_H */

