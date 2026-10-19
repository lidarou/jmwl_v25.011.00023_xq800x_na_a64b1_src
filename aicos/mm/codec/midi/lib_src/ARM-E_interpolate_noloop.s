;***********************************************************
; Function:    WT_InterpolateNoLoop
; Processor:   ARM-E
; Description: the main synthesis function when fetching 
;			   wavetable samples.
;              C-callable.
;
; Usage: 
;	void WT_InterpolateNoLoop(
;		S_WT_VOICE *pWTVoice,
;		S_WT_FRAME *pWTFrame);
;
; Copyright (C) Sonic Network, Inc. 2004-2011
;****************************************************************
; Revision Control:
;   $Revision: 481 $
;   $Date: 2006-11-28 02:57:46 -0500 (Tue, 28 Nov 2006) $
;****************************************************************
;
;   where:
;	S_WT_VOICE *pWTVoice
;	PASSED IN: r0
;
;	S_WT_FRAME *pWTFrame;
;	PASSED IN: r1
;****************************************************************

	INCLUDE		ARM_synth_constants.inc

	CODE32
	AREA |.text-core|, CODE, READONLY

	
	EXPORT		WT_InterpolateNoLoop
	

; Register usage
; --------------
pWTVoice					RN	r0
pWTFrame					RN	r1
pOutputBuffer				RN	r2
numSamples					RN	r3

phaseIncrement				RN	r4
pPhaseAccum					RN	r5
phaseFrac					RN	r6
phaseFracMask				RN	r7

tmp0						RN	r1	; reuse register
tmp1						RN	r8
tmp2						RN	r9


SaveRegs	RLIST	{r4-r9,lr}
RestoreRegs	RLIST	{r4-r9,pc}

WT_InterpolateNoLoop PROC

	STMFD	sp!, SaveRegs

;
; Fetch parameters from structures
;----------------------------------------------------------------

	LDR		pOutputBuffer, [pWTFrame, #m_pAudioBuffer]
	LDR		numSamples, [pWTFrame, #m_numSamples]
	
	LDR		phaseIncrement, [pWTFrame, #m_phaseIncrement]
	LDR		pPhaseAccum, [pWTVoice, #m_pPhaseAccum]
	LDR		phaseFrac, [pWTVoice, #m_phaseFrac]
	LDR		phaseFracMask,=PHASE_FRAC_MASK

InterpolationLoop

	IF	:DEF:SAMPLES_8_BIT
	LDRSB	tmp0, [pPhaseAccum]				; tmp0 = x0
	LDRSB	tmp1, [pPhaseAccum, #1]			; tmp1 = x1
	ELSE
	LDRSH	tmp0, [pPhaseAccum]				; tmp0 = x0
	LDRSH	tmp1, [pPhaseAccum, #2]			; tmp1 = x1
	ENDIF

	ADD		tmp2, phaseIncrement, phaseFrac	; increment pointer here to avoid pipeline stall

	SUB		tmp1, tmp1, tmp0				; tmp1 = x1 - x0
	SMULBB	tmp1, phaseFrac, tmp1			; tmp1 = phaseFrac * tmp2

; This section performs a gain adjustment of -12dB for 16-bit samples
; or +36dB for 8-bit samples. For a high quality synthesizer, the output
; can be set to full scale, however if the filter is used, it can overflow
; with certain coefficients and signal sources. In this case, either a
; saturation operation should take in the filter before scaling back to
; 16 bits or the signal path should be increased to 18 bits or more.

	IF	:DEF:SAMPLES_8_BIT
	MOV		tmp0, tmp0, LSL #6							; boost 8-bit signal by 36dB
	ELSE
	MOV		tmp0, tmp0, ASR #2							; reduce 16-bit signal by 12dB
	ENDIF															
	
	ADD		tmp1, tmp0, tmp1, ASR #(NUM_EG1_FRAC_BITS-6)	; tmp1 = tmp0 + (tmp1 >> (15-6))
															;	   = x0 + f * (x1 - x0) == interpolated result
	
	STRH	tmp1, [pOutputBuffer], #NEXT_OUTPUT_PCM	; *pOutputBuffer++ = interpolated result
	
; carry overflow from fraction to integer portion
	ADD	pPhaseAccum, pPhaseAccum, tmp2, LSR #(NUM_PHASE_FRAC_BITS - NEXT_INPUT_PCM_SHIFT)
	AND	phaseFrac, tmp2, phaseFracMask		; nphaseFrac = frac part

	SUBS	numSamples, numSamples, #1
	BGT		InterpolationLoop

; Clean up and store any changes that were caused during the loop
;----------------------------------------------------------------

	; update and store phase
	STR		pPhaseAccum, [pWTVoice, #m_pPhaseAccum]
	STR		phaseFrac, [pWTVoice, #m_phaseFrac]

;
; Return to calling function
;----------------------------------------------------------------

	LDMFD	sp!,{r4-r9,lr}
	BX		lr

	ENDP
	END
	
