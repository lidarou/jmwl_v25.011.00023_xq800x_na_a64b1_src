/*----------------------------------------------------------------------------
 *
 * File: 
 * eas_enhancer.c 
 *
 * Contents and purpose:
 * Contains the implementation of the enhancer effect.
 *
 *
 * Copyright (C) Sonic Network, Inc. 2004-2011
 *
 *----------------------------------------------------------------------------
 * Revision Control:
 *   $Revision: 502 $
 *   $Date: 2006-12-12 00:42:02 -0500 (Tue, 12 Dec 2006) $
 *----------------------------------------------------------------------------
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1dbc17
#define LOG_MODULE    AMOD_AUDIO

#include "eas_data.h"
#include "eas_effects.h"
#include "eas_math.h"
#include "eas_enhdata.h"
#include "eas_enhancer.h"
#include "eas_config.h"
#include "eas_host.h"
#include "eas_report.h"

/*----------------------------------------------------------------------------
 * Defines
 *----------------------------------------------------------------------------
*/

/* default enhancer parameter values */
#define		DEFAULT_ENHANCER_FEEDBACK1		112		/* fo(112/128) = fo(28/32) =  467 Hz */
#define		DEFAULT_ENHANCER_FEEDBACK2		124		/* fo(  8/128) = fo( 2/32) = 5074 Hz */	/* use this when filter 2 is 0p1z */
#define		DEFAULT_DRIVE_COEF				48		/* 3.0 * 2^NUM_DRIVE_COEF_FRAC_BITS == 3 * 16 == 48 */

#define		DEFAULT_WET						61		/* 0.316 * 2^NUM_WET_DRY_FRAC_BITS */
#define		DEFAULT_DRY						121		/* 0.948 * 2^NUM_WET_DRY_FRAC_BITS */

#define		DEFAULT_BYPASS_F1				EAS_FALSE
#define		DEFAULT_BYPASS_F2				EAS_FALSE
#define		DEFAULT_BYPASS_DISTORTION		EAS_FALSE
#define		DEFAULT_NON_LINEARITY			0	/* select this non linearity as default */
#define		MAX_NUM_NON_LINEARITIES			4
#define		DEFAULT_APPLY_DITHER			EAS_TRUE

/* Macros for the distortion part of the enhancer */
/*
	expect 0<= enhancer filter coef < 1
	1.0 is actually 1.0 - 1 LSbit == NUM_ENHANCER_FILTER_COEF_FRAC_BITS ones
*/
#define	NUM_ENHANCER_FILTER_COEF_FRAC_BITS	5
#define ENHANCER_FILTER_ONE					(EAS_I32) (0x1L << NUM_ENHANCER_FILTER_COEF_FRAC_BITS)

#define MULT_FILTER_AUDIO_COEF(audio,coef) 		/*lint -e704 <avoid divide for performance>*/ \
			(EAS_I32)(									\
			(											\
				((EAS_I32)(audio)) * ((EAS_I32)(coef))	\
			)											\
			>> NUM_ENHANCER_FILTER_COEF_FRAC_BITS		\
										)				\
												/* lint +e704 <restore checking>*/

/* prototypes for effects interface */
static EAS_RESULT EnhancerInit (EAS_DATA_HANDLE pEASData, EAS_VOID_PTR *pInstData);
static void EnhancerProcess (EAS_VOID_PTR pInstData, EAS_PCM *pSrc, EAS_PCM *pDst, EAS_I32 numSamples);
static EAS_RESULT EnhancerShutdown (EAS_DATA_HANDLE pEASData, EAS_VOID_PTR pInstData);
static EAS_RESULT EnhancerGetParam (EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 *pValue);
static EAS_RESULT EnhancerSetParam (EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 value);

/* common effects interface for configuration module */
const S_EFFECTS_INTERFACE EAS_Enhancer = 
{
	EnhancerInit,
	EnhancerProcess,
	EnhancerShutdown,
	EnhancerGetParam,
	EnhancerSetParam
};


/*----------------------------------------------------------------------------
 * EnhancerInit()
 *----------------------------------------------------------------------------
 * Purpose: 
 * 
 * Inputs: 
 * psEASData - pointer to overall EAS data structure
 *			
 * Outputs:
 *
 *----------------------------------------------------------------------------
*/
static EAS_RESULT EnhancerInit (EAS_DATA_HANDLE pEASData, EAS_VOID_PTR *pInstData)
{
	S_ENHANCER_OBJECT *pEnhData;

	/* check Configuration Module for data allocation */
	if (pEASData->staticMemoryModel)
		pEnhData = EAS_CMEnumFXData(EAS_MODULE_ENHANCER);

	/* allocate dynamic memory */
	else
		pEnhData = EAS_HWMalloc(pEASData->hwInstData, sizeof(S_ENHANCER_OBJECT));

	if (pEnhData == NULL)
	{
		EAS_ReportEx(_EAS_SEVERITY_FATAL, 0x93b9328a, 0x00000001 );
		return EAS_ERROR_MALLOC_FAILED;
	}

	/* clear the structure */
	EAS_HWMemSet(pEnhData, 0, sizeof(S_ENHANCER_OBJECT));

	/* set some default values */
	pEnhData->m_nEnhancerFeedback1
		= (EAS_I8) ( DEFAULT_ENHANCER_FEEDBACK1 >> (7 - NUM_ENHANCER_FILTER_COEF_FRAC_BITS) );
		
	pEnhData->m_nEnhancerFeedForward1
		= (EAS_I8) ( (EAS_I8) ( (DEFAULT_ENHANCER_FEEDBACK1 >> (7 - NUM_ENHANCER_FILTER_COEF_FRAC_BITS)) + ENHANCER_FILTER_ONE) >> 1);
	
	pEnhData->m_nEnhancerFeedback2
		= (EAS_I8) ( DEFAULT_ENHANCER_FEEDBACK2 >> (7 - NUM_ENHANCER_FILTER_COEF_FRAC_BITS) );
		
	pEnhData->m_nDriveCoef
		= (EAS_I8) ( DEFAULT_DRIVE_COEF );
		
	pEnhData->m_nWet
		= (EAS_I8) ( DEFAULT_WET );

	pEnhData->m_nDry
		= (EAS_I8) ( DEFAULT_DRY );

	*pInstData = pEnhData;
	return EAS_SUCCESS;
} /* end EnhancerInit */

#ifndef _NATIVE_ENHANCER
/*----------------------------------------------------------------------------
 * SynthEnhancer()
 *----------------------------------------------------------------------------
 * Purpose: 
 * Do a block of processing of the enhancer effect.
 * The inputs and outputs must be in interleaved buffers where
 * the left channel sample precedes the right channel sample.
 *
 * Inputs:
 * pInstData		- instance data handle
 * pSrc				- pointer to source buffer
 * pDst				- pointer to destination buffer
 * numSamples		- number of samples or sample pairs to process
 *
 * Outputs:
 *
 * Side Effects:
 * if pInput == pOutput, then we overwrite the input samples (in-place calc)
 *----------------------------------------------------------------------------
*/
static void EnhancerProcess (EAS_VOID_PTR pInstData, EAS_PCM *pSrc, EAS_PCM *pDst, EAS_I32 numSamples)
{
	S_ENHANCER_OBJECT *pEnhData;
	EAS_I32 nChannelNumber;
	EAS_I32 i;
	EAS_I32 nTempSample;
	EAS_I32 nTempSample2;
	EAS_PCM nInputSample;
	EAS_I32 nOutputSample;
	EAS_I16 *pzF1, *pzF2;
	EAS_PCM *pIn;
	EAS_PCM *pOut;

	pEnhData = (S_ENHANCER_OBJECT*) pInstData;

	if (pEnhData->bypass)
	{
		if (pSrc != pDst)
			EAS_HWMemCpy(pSrc, pDst, numSamples * NUM_OUTPUT_CHANNELS * (EAS_I32) sizeof(EAS_PCM));
		return;
	}
	
	for (nChannelNumber = 0; nChannelNumber < NUM_OUTPUT_CHANNELS; nChannelNumber++)
	{
		
		pIn = pSrc + nChannelNumber;
		pOut = pDst + nChannelNumber;
		
		if (0 == nChannelNumber)
		{
			/* set ptrs to left channel state vars */
			pzF1 = &(pEnhData->m_zF1L);
			pzF2 = &(pEnhData->m_zF2L);
		}
		else
		{
			/* set ptrs to right channel state vars */
			pzF1 = &(pEnhData->m_zF1R);
			pzF2 = &(pEnhData->m_zF2R);
		}	/* end if (0 == nChannelNumber) */
		
		for (i=0; i < numSamples; i++)
		{
			nInputSample = *pIn;
			pIn += NUM_OUTPUT_CHANNELS;

			/* apply filter 1 */
			/* apply feedforward */
			nTempSample = MULT_FILTER_AUDIO_COEF( nInputSample, pEnhData->m_nEnhancerFeedForward1);
			
			/* apply feedback */
			nTempSample += MULT_FILTER_AUDIO_COEF( *pzF1, pEnhData->m_nEnhancerFeedback1);
			
			/* generate filter output */
			nOutputSample = nTempSample - *pzF1;
			nOutputSample = SATURATE(nOutputSample);
			
			/* update filter state */
//			*pzF1 = nTempSample;
			*pzF1 = (EAS_I16)SATURATE(nTempSample);

			/* apply distortion */
			/*
			Use a drive value for distortion:
			distortion	=	2 * input * (drive+1)/2 - drive * input * |input|
			=	(input * drive) + input - (drive * input) * |input|
			=	(input * drive) * (1 - |input|) + input
			*/

			/* (1 - |input|)	*/
			nTempSample = ((nOutputSample < 0) ? 
				(DISTORTION_ONE + nOutputSample) : (DISTORTION_ONE - nOutputSample));
			/*		nTempSample = SATURATE(nTempSample); */
			
			/* (input * drive)	*/
			nTempSample2 = MULT_AUDIO_DRIVE(nOutputSample, pEnhData->m_nDriveCoef);
			nTempSample2 = SATURATE(nTempSample2);
			
			/* (input * drive) * (1 - |input|) + input	*/
			/* nOutputSample += MULT_AUDIO_AUDIO(nTempSample2, nTempSample); */
			nTempSample = MULT_AUDIO_AUDIO(nTempSample2, nTempSample);
			/*		nTempSample = SATURATE(nTempSample); */
			
			nOutputSample = nOutputSample + nTempSample;
			nOutputSample = SATURATE(nOutputSample);

			/* apply filter 2 */
			/*
			NOTE: f2 = is different than f1 --
			f2 is a zero pole, single zero filter
			Also note: even though the coefficients name is 
			"gnEnhancerFeedback2" the filter topology is really a
			feedforward (not feedback) multiplication 
			*/
			/* apply feedforward */
			/*lint -e{702} <avoid divide for performance>*/
			nOutputSample = nOutputSample >> 1;
			nTempSample = MULT_FILTER_AUDIO_COEF( *pzF2, pEnhData->m_nEnhancerFeedback2);
			nTempSample2 = nOutputSample - nTempSample;
			
			/* update filter state */
			*pzF2 = (EAS_I16)nOutputSample;
			
			/* generate filter output */
			nOutputSample = SATURATE(nTempSample2);
				
			/* apply wet,dry */
			nOutputSample  = MULT_AUDIO_WET_DRY_COEF( nOutputSample, pEnhData->m_nWet);
			nOutputSample += MULT_AUDIO_WET_DRY_COEF( nInputSample,  pEnhData->m_nDry);

			*pOut = (EAS_I16)SATURATE(nOutputSample);
			pOut += NUM_OUTPUT_CHANNELS;

		}	/* end for (i=0; i < nNumMonoSamplesToEnhance; i++) */
			
	}	/* end for (nChannelNumber = 0; nChannelNumber < NUM_OUTPUT_CHANNELS; nChannelNumber++) */

	return;

} /* end EnhancerProcess */
#endif

#ifdef _NATIVE_ENHANCER
/* prototype for external ARM assembly code */
extern void SynthEnhancer ( EAS_I16 nNumMonoSamplesToEnhance, EAS_PCM *pInput, EAS_PCM *pOutput, S_ENHANCER_OBJECT *m_sEnhancer);
/*----------------------------------------------------------------------------
 * EnhancerProcess()
 *----------------------------------------------------------------------------
 * Purpose: 
 * Do a block of processing of the enhancer effect. This version is a
 * temporary veneer to the ARM assembly code.
 *
 * Inputs: 
 * pInstData		- instance data handle
 * pSrc				- pointer to source buffer
 * pDst				- pointer to destination buffer
 * numSamples		- number of samples or sample pairs to process
 *
 * Outputs:
 *
 * Side Effects:
 * if pInput == pOutput, then we overwrite the input samples (in-place calc)
 *----------------------------------------------------------------------------
*/
static void EnhancerProcess (EAS_VOID_PTR pInstData, EAS_PCM *pSrc, EAS_PCM *pDst, EAS_I32 numSamples)
{
	SynthEnhancer((EAS_I16) numSamples, pSrc, pDst, (S_ENHANCER_OBJECT*) pInstData);
} /* end EnhancerProcess */
#endif

/*----------------------------------------------------------------------------
 * EnhancerShutdown()
 *----------------------------------------------------------------------------
 * Purpose: 
 * Initializes the enhancer effect.
 *
 * Inputs:
 * pInstData		- handle to instance data
 *		
 * Outputs:
 * 
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
*/
static EAS_RESULT EnhancerShutdown (EAS_DATA_HANDLE pEASData, EAS_VOID_PTR pInstData)
{
	/* check Configuration Module for static memory allocation */
	if (!pEASData->staticMemoryModel)
		if (pInstData)
			EAS_HWFree(pEASData->hwInstData, pInstData);
	return EAS_SUCCESS;
} /* end EnhancerShutdown */

/*----------------------------------------------------------------------------
 * EnhancerGetParam()
 *----------------------------------------------------------------------------
 * Purpose: 
 * Get an Enhancer parameter
 *
 * Inputs:
 * pInstData		- handle to instance data
 * param			- parameter index
 * *pValue			- pointer to variable to hold retrieved value
 *		
 * Outputs:
 * 
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
*/
static EAS_RESULT EnhancerGetParam (EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 *pValue)
{
	S_ENHANCER_OBJECT *p;

	p = (S_ENHANCER_OBJECT*) pInstData;
	switch (param)
	{
		case EAS_PARAM_ENHANCER_BYPASS:
			*pValue = (EAS_I32) p->bypass;
			break;
		default:
			return EAS_ERROR_INVALID_PARAMETER;
	}
	return EAS_SUCCESS;
} /* end EnhancerGetParam */

/*----------------------------------------------------------------------------
 * EnhancerSetParam()
 *----------------------------------------------------------------------------
 * Purpose: 
 * Set a Enhancer parameter
 *
 * Inputs:
 * pInstData		- handle to instance data
 * param			- parameter index
 * *pValue			- new paramter value
 *		
 * Outputs:
 * 
 *
 * Side Effects:
 *
 *----------------------------------------------------------------------------
*/
static EAS_RESULT EnhancerSetParam (EAS_VOID_PTR pInstData, EAS_I32 param, EAS_I32 value)
{
	S_ENHANCER_OBJECT *p;

	p = (S_ENHANCER_OBJECT*) pInstData;
	switch (param)
	{
		case EAS_PARAM_ENHANCER_BYPASS:
			p->bypass = (EAS_BOOL) value;
			break;
		default:
			return EAS_ERROR_INVALID_PARAMETER;
	}
	return EAS_SUCCESS;
} /* end EnhancerSetParam */


