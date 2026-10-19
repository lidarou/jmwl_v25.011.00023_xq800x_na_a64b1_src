;***********************************************************
; Function:    WT_VoiceGain
; Processor:   ARM-E
; Description: the main synthesis function when fetching 
;			   wavetable samples.
;              C-callable.
;
; Usage: 
; Usage: 
;	WT_VoiceGain(
;			S_WT_VOICE *pWTVoice,
;			S_WT_FRAME *pWTFrame);
;
; Copyright (C) Sonic Network, Inc. 2004-2011
;****************************************************************
; Revision Control:
;   $Revision: 741 $
;   $Date: 2007-06-22 19:39:21 -0400 (Fri, 22 Jun 2007) $
;****************************************************************
;
;   where:
;	S_WT_VOICE *psVoice
;	PASSED IN: r0
;
;	S_WT_FRAME *pWTFrame
;	PASSED IN: r1
;****************************************************************



	INCLUDE		ARM_synth_constants.inc

	CODE32
	AREA ||.text-core||, CODE, READONLY

	EXPORT		WT_VoiceGain
	
; Register usage
; --------------
pWTVoice					RN	r0
pWTFrame					RN	r1
pInputBuffer				RN	r2
pMixBuffer					RN	r3

tmp0						RN	r4
tmp1						RN	r5
tmp2						RN	r1	; reuse register
tmp3						RN	r6

numSamples					RN	r9

	IF STEREO_OUTPUT = {TRUE}
gainIncLeft					RN	r7
gainIncRight				RN	r8
gainLeft					RN	r10
gainRight					RN	r11
	ELSE
gainIncrement				RN	r7
gain						RN	r8
	ENDIF


; register context for local variables
SaveRegs	RLIST	{r4-r11,lr}
;RestoreRegs	RLIST	{r4-r11,pc}

WT_VoiceGain PROC

	STMFD	sp!, SaveRegs

	LDR		pInputBuffer, [pWTFrame, #m_pAudioBuffer]
	LDR		pMixBuffer, [pWTFrame, #m_pMixBuffer]
	LDR		numSamples, [pWTFrame, #m_numSamples]
	
;----------------------------------------------------------------
; Stereo version
;----------------------------------------------------------------
; NOTE: instructions are reordered to reduce the effect of latency 
; due to storage and computational dependencies.
;----------------------------------------------------------------

	IF STEREO_OUTPUT = {TRUE}

	LDR		tmp0, [pWTFrame, #m_prevGain]
	LDR		tmp1, [pWTFrame, #m_gainTarget]
	
	LDRSH	gainLeft, [pWTVoice, #m_gainLeft]
	LDRSH	gainRight, [pWTVoice, #m_gainRight]
	
	MOV		gainIncLeft, gainLeft
	SMULBB	gainLeft, tmp0, gainLeft

	SMULBB	gainIncLeft, tmp1, gainIncLeft
	SUB		gainIncLeft, gainIncLeft, gainLeft
	MOV		gainLeft, gainLeft, ASR #(NUM_MIXER_GUARD_BITS - 2)
	MOV		gainIncLeft, gainIncLeft, ASR #(SYNTH_UPDATE_PERIOD_IN_BITS + NUM_MIXER_GUARD_BITS - 2)

	MOV		gainIncRight, gainRight
	SMULBB	gainRight, tmp0, gainRight

	SMULBB	gainIncRight, tmp1, gainIncRight
	SUB		gainIncRight, gainIncRight, gainRight
	MOV		gainRight, gainRight, ASR #(NUM_MIXER_GUARD_BITS - 2)
	MOV		gainIncRight, gainIncRight, ASR #(SYNTH_UPDATE_PERIOD_IN_BITS + NUM_MIXER_GUARD_BITS - 2)

	LDRSH		tmp0, [pInputBuffer], #2
	
StereoGainLoop
	LDR		tmp1, [pMixBuffer]

	ADD		gainLeft, gainLeft, gainIncLeft

	SMLAWB	tmp1, gainLeft, tmp0, tmp1

	LDR		tmp2, [pMixBuffer, #4]

	ADD		gainRight, gainRight, gainIncRight

	STR		tmp1, [pMixBuffer], #4

	SMLAWB	tmp2, gainRight, tmp0, tmp2

	SUBS	numSamples, numSamples, #1

	LDRGTSH	tmp0, [pInputBuffer], #2
	
	STR		tmp2, [pMixBuffer], #4

	BGT		StereoGainLoop

;----------------------------------------------------------------
; Mono version
;----------------------------------------------------------------
	ELSE

	LDR		gain, [pWTFrame, #m_prevGain]
	MOV		gain, gain, LSL #11
	LDR		gainIncrement, [pWTFrame, #m_gainTarget]
	MOV		gainIncrement, gainIncrement, LSL #(NUM_MIXER_GUARD_BITS + 4)
	SUB		gainIncrement, gainIncrement, gain
	MOV		gainIncrement, gainIncrement, ASR #SYNTH_UPDATE_PERIOD_IN_BITS
	
MonoGainLoop

	LDRSH	tmp0, [pInputBuffer], #NEXT_OUTPUT_PCM	; fetch voice output
	
	LDR		tmp1, [pMixBuffer]						; get left channel output sample
	ADD		gain, gain, gainIncrement				; gain step to eliminate zipper noise
	SMULWB	tmp0, gain, tmp0 						; sample * local gain

	MOV		tmp0, tmp0, ASR #1						; add 6dB headroom
	ADD 	tmp1, tmp0, tmp1
	STR		tmp1, [pMixBuffer], #4					; save and bump pointer
	
	SUBS	numSamples, numSamples, #1
	BGT		MonoGainLoop

	ENDIF	;end Mono version

	LDMFD	sp!,SaveRegs
	BX		lr
	
	ENDP
	END

