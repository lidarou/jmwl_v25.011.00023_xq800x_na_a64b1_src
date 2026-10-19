/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_data.h
 *
 * Contents and purpose:
 * This header defines all types, to support dynamic allocation of the
 * memory resources needed for persistent EAS data.
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 686 $
 *   $Date: 2007-05-03 17:10:54 -0400 (Thu, 03 May 2007) $
 *----------------------------------------------------------------------------
*/

#ifndef _EAS_DATA_H
#define _EAS_DATA_H

#include "eas_types.h"
#include "eas_synthcfg.h"
#include "eas.h"
#include "eas_audioconst.h"
#include "eas_sndlib.h"
#include "eas_pcm.h"
#include "eas_pcmdata.h"
#include "eas_synth.h"
#include "eas_miditypes.h"
#include "eas_effects.h"

#ifdef AUX_MIXER
#include "eas_auxmixdata.h"
#endif

#ifdef JET_INTERFACE
#include "jet.h"
#endif

#ifdef _METRICS_ENABLED
#include "eas_perf.h"
#endif

#ifndef MAX_NUMBER_STREAMS
#define MAX_NUMBER_STREAMS			4
#endif

/* flags for S_EAS_STREAM */
#define STREAM_FLAGS_PARSED			1
#define STREAM_FLAGS_PAUSE			2
#define STREAM_FLAGS_LOCATE			4
#define STREAM_FLAGS_RESUME			8

/* structure for parsing a stream */
typedef struct s_eas_stream_tag
{
	void							*pParserModule;
	EAS_U32							time;
	EAS_U32							frameLength;
	EAS_I32							repeatCount;
	EAS_VOID_PTR					handle;
	EAS_U8							volume;
	EAS_BOOL8						streamFlags;
} S_EAS_STREAM;

/* amalgamated persistent data type */
typedef struct s_eas_data_tag
{
#ifdef _CHECKED_BUILD
	EAS_U32							handleCheck;
#endif
	EAS_HW_DATA_HANDLE				hwInstData;

	S_EFFECTS_MODULE				effectsModules[NUM_EFFECTS_MODULES];

#ifdef _METRICS_ENABLED	
	S_METRICS_INTERFACE				*pMetricsModule;
	EAS_VOID_PTR					pMetricsData;
#endif

	EAS_I32							*pMixBuffer;
	EAS_PCM							*pOutputAudioBuffer;

#ifdef AUX_MIXER
	S_EAS_AUX_MIXER					auxMixer;
#endif

#ifdef _MAXIMIZER_ENABLED
	EAS_VOID_PTR					pMaximizerData;
#endif

	S_EAS_STREAM					streams[MAX_NUMBER_STREAMS];
	
	S_PCM_STATE						*pPCMStreams;

	S_VOICE_MGR						*pVoiceMgr;

#ifdef JET_INTERFACE
	JET_DATA_HANDLE					jetHandle;
#endif
	
	EAS_U32							renderTime;
	EAS_I16							masterGain;
	EAS_U8							masterVolume;
	EAS_BOOL8						staticMemoryModel;
	EAS_BOOL8						searchHeaderFlag;
} S_EAS_DATA;

#endif

