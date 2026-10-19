;***********************************************************
; Function:    SynthVoiceFilter
; Processor:   ARM
; Description:
; Implements a 2-pole low-pass filter with resonanance
;
; Usage: 
;	void SynthVoiceFilter(
;		S_FILTER CONTROL *pFilter,
;		S_WT_PARAMS *pFilter);
;
; Copyright 2005 Sonic Network, Inc.
;****************************************************************
; Revision Control:
;   $Revision: 619 $
;   $Date: 2007-03-15 14:00:12 -0400 (Thu, 15 Mar 2007) $
;****************************************************************
;
;   where:
;	S_FILTER_CONTROL *pFilter
;	PASSED IN: r0
;
;	S_WT_PARAMS *pWTParams
;	PASSED IN: r1
;****************************************************************

	INCLUDE		ARM_synth_constants.inc

	CODE32
	AREA |.text-core|, CODE, READONLY

	
	EXPORT		WT_VoiceFilter

; Register usage
; --------------
pFilter						RN	r0
pWTParams					RN	r1
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


WT_VoiceFilter PROC

	STMFD	sp!, SaveRegs
	
;
; Setup passed parameters in their destination registers
;----------------------------------------------------------------

	LDR		pBuffer, [pWTParams, #m_pAudioBuffer]
	LDR		numSamples, [pWTParams, #m_numSamples]

	;load state variables from pFilter structure
	LDRSH	z1, [pFilter, #m_z1]
	LDRSH	z2, [pFilter, #m_z2]

	;load coefficients from pWTParams structure
	LDRSH	K, [pWTParams, #m_k]
	LDRSH	b1, [pWTParams, #m_b1]
	LDRSH	b2, [pWTParams, #m_b2]
	
	RSB	b1, b1, #0						; b1 = -b1
	RSB	b2, b2, #0						; b2 = -b2
	MOV	b2, b2, ASR #1					; b2 = b2 >> 1
	MOV	K, K, ASR #1					; K = K >> 1

;
; Start processing
;----------------------------------------------------------------

FilterLoop
	LDRSH	tmp1, [pBuffer]							; fetch sample
	MUL		tmp2, z1, b1							; tmp2 = z1 * -b1
	MLA		tmp2, z2, b2, tmp2						; tmp2 = (-b1 * z1) + (-b2 * z2)
	MLA		tmp1, K, tmp1, tmp2						; tmp1 = (K * x[n]) + (-b1 * z1) + (-b2 * z2)
	MOV		z2, z1									; delay line
	MOV		z1, tmp1, ASR #14						; shift result to low word
	STRH	z1, [pBuffer], #NEXT_OUTPUT_PCM			; write back to buffer

	LDRSH	tmp1, [pBuffer]							; fetch sample
	MUL		tmp2, z1, b1							; tmp2 = z1 * -b1
	MLA		tmp2, z2, b2, tmp2						; tmp2 = (-b1 * z1) + (-b2 * z2)
	MLA		tmp1, K, tmp1, tmp2						; tmp1 = (K * x[n]) + (-b1 * z1) + (-b2 * z2)
	MOV		z2, z1									; delay line
	MOV		z1, tmp1, ASR #14						; shift result to low word
	STRH	z1, [pBuffer], #NEXT_OUTPUT_PCM			; write back to buffer
	
	SUBS	numSamples, numSamples, #2
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
	
