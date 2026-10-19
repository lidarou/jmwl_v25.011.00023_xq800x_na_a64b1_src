;***********************************************************
; File:			ARM_synth_constants.inc
; Processor:	ARM
; Description:	Contains constants and defines, most of which
;			  	are mirrored in synth.h
;
; Copyright Sonic Network Inc. 2004
;****************************************************************
; Revision Control:
;   $Revision: 741 $
;   $Date: 2007-06-22 19:39:21 -0400 (Fri, 22 Jun 2007) $
;****************************************************************


	IF	:DEF:SAMPLE_RATE_8000
SYNTH_UPDATE_PERIOD_IN_BITS		EQU	5
BUFFER_SIZE_IN_MONO_SAMPLES		EQU	32
	ENDIF

	IF	:DEF:SAMPLE_RATE_16000
SYNTH_UPDATE_PERIOD_IN_BITS		EQU	6
BUFFER_SIZE_IN_MONO_SAMPLES		EQU	64
	ENDIF

	IF	:DEF:SAMPLE_RATE_20000
SYNTH_UPDATE_PERIOD_IN_BITS		EQU	7
BUFFER_SIZE_IN_MONO_SAMPLES		EQU	128
	ENDIF

	IF	:DEF:SAMPLE_RATE_22050
SYNTH_UPDATE_PERIOD_IN_BITS		EQU	7
BUFFER_SIZE_IN_MONO_SAMPLES		EQU	128
	ENDIF

	IF	:DEF:SAMPLE_RATE_24000
SYNTH_UPDATE_PERIOD_IN_BITS		EQU	7
BUFFER_SIZE_IN_MONO_SAMPLES		EQU	128
	ENDIF

	IF	:DEF:SAMPLE_RATE_32000
SYNTH_UPDATE_PERIOD_IN_BITS		EQU	7
BUFFER_SIZE_IN_MONO_SAMPLES		EQU	128
	ENDIF

	IF	:DEF:SAMPLE_RATE_44100
SYNTH_UPDATE_PERIOD_IN_BITS		EQU	8
BUFFER_SIZE_IN_MONO_SAMPLES		EQU	256
	ENDIF

	IF	:DEF:SAMPLE_RATE_48000
SYNTH_UPDATE_PERIOD_IN_BITS		EQU	8
BUFFER_SIZE_IN_MONO_SAMPLES		EQU	256
	ENDIF


; if the OUTPUT PCM sample is 16-bits, then when using indexed addressing,
; the next sample is this many bytes away
NEXT_OUTPUT_PCM				EQU	2

;****************************************************************************
;/* macros for fractional phase accumulator */
NUM_PHASE_FRAC_BITS			EQU	15

PHASE_FRAC_MASK				EQU	0x7FFF

; shift for phase accumulator when fraction carries over
	IF	:DEF:SAMPLES_8_BIT
NEXT_INPUT_PCM_SHIFT			EQU	0
	ENDIF

	IF	:DEF:SAMPLES_16_BIT
NEXT_INPUT_PCM_SHIFT			EQU	1
	ENDIF

;****************************************************************************
;/* Envelope 1 (EG1) calculation macros */
NUM_EG1_FRAC_BITS			EQU	15

;****************************************************************************
NUM_MIXER_GUARD_BITS			EQU	4

;****************************************************************************

NUM_ENHANCER_FILTER_COEF_FRAC_BITS	EQU	5

;****************************************************************************

;
; I've temporarily given up on the idea of getting ADS/RV and gcc to
; handle a struct in a compatible fashion. Switching to old fashion EQU
;

	IF FILTER_ENABLED = {TRUE}
;**************************************
; typedef struct s_filter_tag
m_z1			EQU	0
m_z2			EQU	2
	ENDIF

;**************************************
; typedef struct s_wt_frame_tag
m_gainTarget		EQU	0
m_phaseIncrement	EQU	4

	IF FILTER_ENABLED = {TRUE}
m_k			EQU	8
m_b1			EQU	12
m_b2			EQU	16
m_pAudioBuffer		EQU	20
m_pMixBuffer		EQU	24
m_numSamples		EQU	28
m_prevGain		EQU	32
	ELSE
m_pAudioBuffer		EQU	8
m_pMixBuffer		EQU	12
m_numSamples		EQU	16
m_prevGain		EQU	20
	ENDIF


;**************************************
; typedef struct s_wt_voice_tag
m_pLoopEnd		EQU	0	; /* points to last PCM sample (not 1 beyond last) */
m_pLoopStart		EQU	4	; /* points to first sample at start of loop */
m_pPhaseAccum		EQU	8	; /* points to first sample at start of loop */
m_phaseFrac		EQU	12	; /* points to first sample at start of loop */

	IF STEREO_OUTPUT = {TRUE}
m_gainLeft		EQU	16 	; /* current gain, left ch  */
m_gainRight		EQU	18	; /* current gain, right ch */
	ENDIF


;****************************************************************************
; enhancer
m_nEnhancerFeedForward1	EQU	0
m_nEnhancerFeedback1	EQU	1
m_nDriveCoef		EQU	2
m_nEnhancerFeedback2	EQU	3
m_nWet			EQU	4
m_nDry			EQU	5

m_zF0L			EQU	6	; filter 1 zero state var, left
m_zF1L			EQU	8	; filter 1 pole state var, left
m_zF2L			EQU	10	; filter 2 zero state var, left
m_zF0R			EQU	12	; filter 1 zero state var, right
m_zF1R			EQU	14	; filter 1 pole state var, right
m_zF2R			EQU	16	; filter 2 zero state var, right

;****************************************************************************

	END

