;***********************************************************
; Function:    WT_VoiceFilter
; Processor:   ARM
; Description:
; Implements a 2-pole low-pass filter with resonanance
;
; Usage: 
;	void WT_VoiceFilter(
;		S_FILTER CONTROL *pFilter,
;		S_WT_FRAME *pWTFrame);
;
; Copyright 2005 Sonic Network, Inc.
;****************************************************************
; Revision Control:
;   $Revision: 481 $
;   $Date: 2006-11-28 02:57:46 -0500 (Tue, 28 Nov 2006) $
;****************************************************************
;
;   where:
;	S_FILTER_CONTROL *pFilter
;	PASSED IN: r0
;
;	S_WT_FRAME *pWTFrame
;	PASSED IN: r1
;****************************************************************

	INCLUDE		ARM_synth_constants.inc

	CODE32
	AREA ||.text-core||, CODE, READONLY


	EXPORT		WT_VoiceFilter
	

; Register usage
; --------------
pFilter						RN	r0
pWTFrame					RN	r1
pBuffer						RN	r2
numSamples					RN	r3

z1							RN	r4
z2							RN	r5
b1							RN	r6
b2							RN	r7
K							RN	r8

tmp0						RN	r1	; reuse register
tmp1						RN	r9
tmp2						RN	r10


SaveRegs	RLIST	{r4-r10, lr}
;RestoreRegs	RLIST	{r4-r10, pc}


WT_VoiceFilter PROC

	STMFD	sp!, SaveRegs

;
; Setup passed parameters in their destination registers
;----------------------------------------------------------------

	LDR		pBuffer, [pWTFrame, #m_pAudioBuffer]
	LDR		numSamples, [pWTFrame, #m_numSamples]

	;load state variables from pFilter structure
	LDRSH	z1, [pFilter, #m_z1]
	LDRSH	z2, [pFilter, #m_z2]

	;load coefficients from pWTFrame structure
	LDR		K, [pWTFrame, #m_k]
	LDR		b1, [pWTFrame, #m_b1]
	LDR		b2, [pWTFrame, #m_b2]
	
	RSB	b1, b1, #0						; b1 = -b1
	RSB	b2, b2, #0						; b2 = -b2
	MOV	b2, b2, ASR #1					; b2 = b2 >> 1
	MOV	K, K, ASR #1					; K = K >> 1

;
; Start processing
;----------------------------------------------------------------

	LDRSH	tmp0, [pBuffer]					; fetch sample

FilterLoop
	SMULBB	tmp2, z1, b1					; tmp2 = z1 * -b1
	SMLABB	tmp2, z2, b2, tmp2				; tmp2 = (-b1 * z1) + (-b2 * z2)

	MOV		z2, z1							; delay line

	SMLABB	tmp0, tmp0, K, tmp2				; tmp1 = (K * x[n]) + (-b1 * z1) + (-b2 * z2)
	
	LDRSH	tmp1, [pBuffer, #NEXT_OUTPUT_PCM]	; fetch next sample
	
	MOV		z1, tmp0, ASR #14				; shift result to low word
	STRH	z1, [pBuffer], #NEXT_OUTPUT_PCM	; write back to buffer

	SMULBB	tmp2, z1, b1					; tmp2 = z1 * -b1
	
	SUBS	numSamples, numSamples, #2		; unroll loop once

	SMLABB	tmp2, z2, b2, tmp2				; tmp2 = (-b1 * z1) + (-b2 * z2)

	SMLABB	tmp1, tmp1, K, tmp2				; tmp1 = (K * x[n]) + (-b1 * z1) + (-b2 * z2)

	MOV		z2, z1							; delay line

	MOV		z1, tmp1, ASR #14				; shift result to low word
	
	LDRGTSH	tmp0, [pBuffer, #NEXT_OUTPUT_PCM]	; fetch next sample

	STRH	z1, [pBuffer], #NEXT_OUTPUT_PCM	; write back to buffer

	BGT		FilterLoop
; save z terms
;----------------------------------------------------------------

	STRH	z1, [pFilter, #m_z1]
	STRH	z2, [pFilter, #m_z2]

; Return to calling function
;----------------------------------------------------------------

	LDMFD	sp!,SaveRegs
	BX		lr

	ENDP
	END

