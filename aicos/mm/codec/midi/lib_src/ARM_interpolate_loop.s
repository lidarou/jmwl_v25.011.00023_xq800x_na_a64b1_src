;***********************************************************
; Function:    WT_Interpolate
; Processor:   ARM
; Description: the main synthesis function when fetching 
;			   wavetable samples.
;              C-callable.
;
; Usage: 
;	void WT_Interpolate(
;		S_SYNTH_VOICE *pVoice,
;		S_WT_PARAMS *pWTParams);
;
; Copyright (C) Sonic Network, Inc. 2004-2011
;****************************************************************
; Revision Control:
;   $Revision: 619 $
;   $Date: 2007-03-15 14:00:12 -0400 (Thu, 15 Mar 2007) $
;****************************************************************
;
;   where:
;	S_SYNTH_VOICE *pVoice
;	PASSED IN: r0
;
;	S_WT_PARAMS *pWTParams);
;	PASSED IN: r1
;****************************************************************

	INCLUDE		ARM_synth_constants.inc

	CODE32
	AREA |.text-core|, CODE, READONLY

	EXPORT		WT_Interpolate
	
; Register usage
; --------------
pVoice						RN	r0
pWTParams					RN	r1

numSamples					RN	r2
phaseIncrement				RN	r3
pOutputBuffer				RN	r4

tmp0						RN	r1	;reuse register
tmp1						RN	r5
tmp2						RN	r6

pLoopEnd					RN	r7
pLoopStart					RN	r8

pPhaseAccum					RN	r9
phaseFrac					RN	r10
phaseFracMask				RN	r11

SaveRegs	RLIST	{r4-r11,lr}
;RestoreRegs	RLIST	{r4-r11,pc}


WT_Interpolate PROC

	STMFD	sp!,SaveRegs

; Fetch parameters from structures

	LDR		pOutputBuffer, [pWTParams, #m_pAudioBuffer]
	LDR		numSamples, [pWTParams, #m_numSamples]

	LDR		phaseIncrement, [pWTParams, #m_phaseIncrement]
	LDR		pPhaseAccum, [pVoice, #m_pPhaseAccum]
	LDR		phaseFrac, [pVoice, #m_phaseFrac]
	LDR		phaseFracMask,=PHASE_FRAC_MASK

	LDR		pLoopStart, [pVoice, #m_pLoopStart]
	LDR		pLoopEnd, [pVoice, #m_pLoopEnd]
	ADD		pLoopEnd, pLoopEnd, #1					; need loop end to equal last sample + 1

InterpolationLoop
	SUBS	tmp0, pPhaseAccum, pLoopEnd		; check for loop end
	ADDGE	pPhaseAccum, pLoopStart, tmp0		; loop back to start

	IF	:DEF:SAMPLES_8_BIT
	LDRSB	tmp0, [pPhaseAccum]				; tmp0 = x0
	LDRSB	tmp1, [pPhaseAccum, #1]			; tmp1 = x1
	ELSE
	LDRSH	tmp0, [pPhaseAccum]				; tmp0 = x0
	LDRSH	tmp1, [pPhaseAccum, #2]			; tmp1 = x1
	ENDIF

; increment phase now to avoid stall and carry overflow from fraction to integer portion
	ADD	tmp2, phaseIncrement, phaseFrac
	ADD	pPhaseAccum, pPhaseAccum, tmp2, LSR #(NUM_PHASE_FRAC_BITS - NEXT_INPUT_PCM_SHIFT)

	SUB	tmp1, tmp1, tmp0					; tmp1 = x1 - x0
	MUL	tmp1, phaseFrac, tmp1				; tmp1 = phaseFrac * tmp1
	
; This section performs a gain adjustment of -12dB for 16-bit samples
; or +36dB for 8-bit samples. For a high quality synthesizer, the output
; can be set to full scale, however if the filter is used, it can overflow
; with certain coefficients and signal sources. In this case, either a
; saturation operation should take in the filter before scaling back to
; 16 bits or the signal path should be increased to 18 bits or more.

	IF	:DEF:SAMPLES_8_BIT
	MOV		tmp0, tmp0, LSL #6						; boost by 36dB
	ELSE
	MOV		tmp0, tmp0, ASR #2						; reduce by 6dB
	ENDIF															
	
	ADD		tmp1, tmp0, tmp1, ASR #(NUM_EG1_FRAC_BITS-6)	; tmp1 = tmp0 + (tmp1 >> (15-6))
															;   = x0 + f * (x1 - x0) == interpolated result
	
	STRH	tmp1, [pOutputBuffer], #NEXT_OUTPUT_PCM	; *pOutputBuffer++ = interpolated result

; mask integer phase from fraction	
	AND	phaseFrac, tmp2, phaseFracMask		; phaseFrac = frac part

	SUBS	tmp0, pPhaseAccum, pLoopEnd		; check for loop end
	ADDGE	pPhaseAccum, pLoopStart, tmp0		; loop back to start

	IF	:DEF:SAMPLES_8_BIT
	LDRSB	tmp0, [pPhaseAccum]				; tmp0 = x0
	LDRSB	tmp1, [pPhaseAccum, #1]			; tmp1 = x1
	ELSE
	LDRSH	tmp0, [pPhaseAccum]				; tmp0 = x0
	LDRSH	tmp1, [pPhaseAccum, #2]			; tmp1 = x1
	ENDIF

; increment phase now to avoid stall and carry overflow from fraction to integer portion
	ADD	tmp2, phaseIncrement, phaseFrac
	ADD	pPhaseAccum, pPhaseAccum, tmp2, LSR #(NUM_PHASE_FRAC_BITS - NEXT_INPUT_PCM_SHIFT)

	SUB	tmp1, tmp1, tmp0					; tmp1 = x1 - x0
	MUL	tmp1, phaseFrac, tmp1				; tmp1 = phaseFrac * tmp1
	
; This section performs a gain adjustment of -12dB for 16-bit samples
; or +36dB for 8-bit samples. For a high quality synthesizer, the output
; can be set to full scale, however if the filter is used, it can overflow
; with certain coefficients and signal sources. In this case, either a
; saturation operation should take in the filter before scaling back to
; 16 bits or the signal path should be increased to 18 bits or more.

	IF	:DEF:SAMPLES_8_BIT
	MOV		tmp0, tmp0, LSL #6						; boost by 36dB
	ELSE
	MOV		tmp0, tmp0, ASR #2						; reduce by 6dB
	ENDIF															
	
	ADD		tmp1, tmp0, tmp1, ASR #(NUM_EG1_FRAC_BITS-6)	; tmp1 = tmp0 + (tmp1 >> (15-6))
															;   = x0 + f * (x1 - x0) == interpolated result
	
	STRH	tmp1, [pOutputBuffer], #NEXT_OUTPUT_PCM	; *pOutputBuffer++ = interpolated result

; mask integer phase from fraction	
	AND	phaseFrac, tmp2, phaseFracMask		; phaseFrac = frac part

	SUBS	numSamples, numSamples, #2
	BGT		InterpolationLoop
	
; update and store phase
	STR		pPhaseAccum, [pVoice, #m_pPhaseAccum]
	STR		phaseFrac, [pVoice, #m_phaseFrac]

	LDMFD	sp!,SaveRegs
	BX		lr

	ENDP
	END

