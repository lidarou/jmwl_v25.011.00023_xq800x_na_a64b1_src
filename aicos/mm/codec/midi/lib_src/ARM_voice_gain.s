;***********************************************************
; Function:    WT_VoiceGain
; Processor:   ARM
; Description: 
;	Scales a 16-bit input buffer by a gain factor using
;	anti-zipper filter plus left/right gain (for stereo)
;	and mixes into a 32-bit buffer.
;
; Usage: 
;	WT_VoiceGain(
;			S_WT_VOICE *pWTVoice,
;			S_WT_FRAME *pWTFrame);
;
; Copyright (C) Sonic Network, Inc. 2004-2011
;****************************************************************
; Revision Control:
;   $Revision: 817 $
;   $Date: 2007-08-02 16:46:22 -0400 (Thu, 02 Aug 2007) $
;****************************************************************
;
;   where:
;	S_WT_VOICE *pWTVoice
;	PASSED IN: r0
;
;	S_WT_FRAME *pWTFrame
;	PASSED IN: r1
;****************************************************************
;

	INCLUDE		ARM_synth_constants.inc

	CODE32
	AREA |.text-core|, CODE, READONLY
	

	EXPORT		WT_VoiceGain
	
; Register usage
; --------------
pWTVoice					RN	r0
pWTFrame					RN	r1
pInputBuffer				RN	r2
pMixBuffer					RN	r3

gainIncrement				RN	r4
gain						RN	r5
numSamples					RN	r6

tmp0						RN	r1	; reuse register
tmp1						RN	r7
tmp2						RN	r8
tmp3						RN	r9
tmp4						RN	r10

	IF STEREO_OUTPUT = {TRUE}
gainLeft					RN	r11
gainRight					RN	r12
	ENDIF


; register context for local variables
SaveRegs	RLIST	{r4-r12,lr}
;RestoreRegs	RLIST	{r4-r12,pc}

WT_VoiceGain PROC

	STMFD	sp!, SaveRegs

	LDR		pInputBuffer, [pWTFrame, #m_pAudioBuffer]
	LDR		pMixBuffer, [pWTFrame, #m_pMixBuffer]
	LDR		numSamples, [pWTFrame, #m_numSamples]

	IF STEREO_OUTPUT = {TRUE}
	LDRSH	gainLeft, [pWTVoice, #m_gainLeft]
	LDRSH	gainRight, [pWTVoice, #m_gainRight]
	ENDIF
	
	LDR		gain, [pWTFrame, #m_prevGain]
	MOV		gain, gain, LSL #16
	LDR		gainIncrement, [pWTFrame, #m_gainTarget]
	MOV		gainIncrement, gainIncrement, LSL #16
	SUB		gainIncrement, gainIncrement, gain

	ANDS	tmp0, numSamples, #1
	BNE		OddCount

;****************************************************************
; This version only works for even sample counts
;****************************************************************

	MOV		gainIncrement, gainIncrement, ASR #(SYNTH_UPDATE_PERIOD_IN_BITS-1)

GainLoopFast

	IF STEREO_OUTPUT = {TRUE}
	LDRSH	tmp0, [pInputBuffer], #NEXT_OUTPUT_PCM		; fetch sample
	ADD		gain, gain, gainIncrement					; gain step to eliminate zipper noise
	MOV		tmp1, gain, ASR #15							; shift to low bits

	MUL		tmp3, gainLeft, tmp1						; multiply to get left gain
	MOV		tmp3, tmp3, ASR #15							; shift to low bits

	MUL		tmp4, gainRight, tmp1						; multiply to get right gain
	MOV		tmp4, tmp4, ASR #15							; shift to low bits
	
;****************************************************************
; stereo output
;****************************************************************
	LDR		tmp1, [pMixBuffer]							; get left channel output sample
	MULS	tmp2, tmp0, tmp3	 						; sample * left gain
	ADD		tmp1, tmp1, tmp2, ASR #NUM_MIXER_GUARD_BITS	; mix with buffer
	STR		tmp1, [pMixBuffer], #4						; save and bump pointer

	LDR		tmp1, [pMixBuffer]							; get right channel output sample
	MULS	tmp2, tmp0, tmp4	 						; sample * right gain
	ADD		tmp1, tmp1, tmp2, ASR #NUM_MIXER_GUARD_BITS	; mix with buffer
	STR		tmp1, [pMixBuffer], #4						; save and bump pointer

	LDRSH	tmp0, [pInputBuffer], #NEXT_OUTPUT_PCM		; fetch next sample

	LDR		tmp1, [pMixBuffer]							; get left channel output sample
	MULS	tmp2, tmp0, tmp3	 						; sample * left gain
	ADD		tmp1, tmp1, tmp2, ASR #NUM_MIXER_GUARD_BITS	; mix with buffer
	STR		tmp1, [pMixBuffer], #4						; save and bump pointer

	LDR		tmp1, [pMixBuffer]							; get right channel output sample
	MULS	tmp2, tmp0, tmp4	 						; sample * right gain
	ADD		tmp1, tmp1, tmp2, ASR #NUM_MIXER_GUARD_BITS	; mix with buffer
	STR		tmp1, [pMixBuffer], #4						; save and bump pointer
	
;****************************************************************
; mono	 output
;****************************************************************
	ELSE
	; NEED TO ADD MONO CODE!!!
	ENDIF

	SUBS	numSamples, numSamples, #2
	BGT		GainLoopFast

	LDMFD	sp!,SaveRegs
	BX		lr

;****************************************************************
; This version works for odd or even sample counts
;****************************************************************
	
OddCount
	MOV		gainIncrement, gainIncrement, ASR #SYNTH_UPDATE_PERIOD_IN_BITS

GainLoop

	LDRSH	tmp0, [pInputBuffer], #NEXT_OUTPUT_PCM		; fetch sample
	ADD		gain, gain, gainIncrement					; gain step to eliminate zipper noise
	MOV		tmp2, gain, ASR #16							; shift to add guard bits
	
;****************************************************************
; stereo output
;****************************************************************
	IF STEREO_OUTPUT = {TRUE}
	LDR		tmp1, [pMixBuffer]							; get left channel output sample
	MULS	tmp2, tmp0, tmp2	 						; sample * local gain
	MOV		tmp2, tmp2, ASR #14							; shift to lower bits
	
	MULS	tmp0, gainLeft, tmp2						; left output += sample * left gain
	MOV		tmp0, tmp0, ASR #NUM_MIXER_GUARD_BITS		; shift to lower bits
	ADD		tmp1, tmp0, tmp1							; sum with other voices
	STR		tmp1, [pMixBuffer], #4						; save and bump pointer

	LDR		tmp1, [pMixBuffer]							; get right channel output sample
	MULS	tmp0, gainRight, tmp2						; right output += sample * left gain
	MOV		tmp0, tmp0, ASR #NUM_MIXER_GUARD_BITS		; shift to lower bits
	ADD		tmp1, tmp0, tmp1							; sum with other voices
	STR		tmp1, [pMixBuffer], #4						; save and bump pointer
	
;****************************************************************
; mono	 output
;****************************************************************
	ELSE
	LDR		tmp1, [pMixBuffer]							; get left channel output sample
	MULS	tmp2, tmp0, tmp2	 						; sample * local gain
	
	MOV		tmp2, tmp2, ASR #6							; shift to lower bits
	ADD		tmp1, tmp2, tmp1							; sum with other voices
	STR		tmp1, [pMixBuffer], #4						; save and bump pointer
	ENDIF
	
	SUBS	numSamples, numSamples, #1
	BGT		GainLoop

	LDMFD	sp!,SaveRegs
	BX		lr

	ENDP

	END
