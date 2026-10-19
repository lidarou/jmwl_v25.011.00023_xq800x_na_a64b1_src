;***********************************************************
; Function:    SynthMasterGain
; Processor:   ARM-E
; Description: Copies 32-bit synth output to 16-bit buffer
;              with saturated gain control
;              C-callable.
;
; Usage: 
;	SynthMasterGain(
;			pInputBuffer
;			pOutputBuffer,
;			nGain,
;			nNumLoopSamples
;	);
;
; Copyright (C) Sonic Network, Inc. 2004-2011
;****************************************************************
; Revision Control:
;   $Revision: 481 $
;   $Date: 2006-11-28 02:57:46 -0500 (Tue, 28 Nov 2006) $
;****************************************************************
;
;   where:
;	long *pInputBuffer
;	PASSED IN: r0
;
;	EAS_PCM *pOutputBuffer
;	PASSED IN: r1
;
;	short nGain
;	PASSED IN: r2
;
;	EAS_U16	nNumLoopSamples
;	PASSED IN: r3
;
;****************************************************************

	INCLUDE		ARM_synth_constants.inc

	CODE32
	AREA ||.text-core||, CODE, READONLY

SynthMasterGain PROC

	EXPORT		SynthMasterGain	; allow other files to use this function
	




; Stack frame
; -----------
RET_ADDR_SZ			EQU		0			;return address
REG_SAVE_SZ			EQU		0			;save-on-entry registers saved
FRAME_SZ			EQU		(8)			;local variables
ARG_BLK_SZ			EQU		0			;argument block

PARAM_OFFSET		EQU		(ARG_BLK_SZ + FRAME_SZ + REG_SAVE_SZ + RET_ADDR_SZ)

; Register usage
; --------------
pnInputBuffer				RN	r0
pnOutputBuffer				RN	r1
nGain						RN	r2
nNumLoopSamples				RN	r3

	STMFD	sp!,{r4-r6,r14}					;Save any save-on-entry registers that are used

	LDR		r6, =0x7fff						;constant for saturation tests

loop
	LDR		r4, [pnInputBuffer], #4			;fetch 1st output sample
	
	LDR		r5, [pnInputBuffer], #4			;fetch 2nd output sample

	SMULWB	r4, r4, nGain					;output = gain * input
	
	CMP		r4, r6							;check for positive saturation
	MOVGT	r4, r6							;saturate
	CMN		r4, r6							;check for negative saturation
	MVNLT	r4, r6							;saturate
	
	SMULWB	r5, r5, nGain					;output = gain * input
	
	STRH	r4, [pnOutputBuffer], #NEXT_OUTPUT_PCM	;save 1st output sample
	
	CMP		r5, r6							;check for positive saturation
	MOVGT	r5, r6							;saturate
	CMN		r5, r6							;check for negative saturation
	MVNLT	r5, r6							;saturate
	STRH	r5, [pnOutputBuffer], #NEXT_OUTPUT_PCM	;save 2nd output sample

	SUBS	nNumLoopSamples, nNumLoopSamples, #2
	BGT		loop

;
; Return to calling function
;----------------------------------------------------------------

	LDMFD	sp!,{r4-r6, lr}				; return to calling function
	BX		lr

;*****************************************************************************

	ENDP		; end of function/procedure

	END		; end of assembly code

