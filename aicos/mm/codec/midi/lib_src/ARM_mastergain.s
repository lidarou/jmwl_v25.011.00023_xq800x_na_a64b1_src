;***********************************************************
; Function:    SynthMasterGain
; Processor:   ARM
; Description: Copies 32-bit synth output to 16-bit buffer
;              with saturated gain control
;              C-callable.
;
; Usage: 
;	asynth_mastergain(
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
	AREA |.text-core|, CODE, READONLY

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

SaveRegs	RLIST	{r4-r6,r14}
;RestoreRegs	RLIST	{r4-r6, pc}

	STMFD	sp!,SaveRegs					;Save any save-on-entry registers that are used

	LDR	r5, =0x7fff							;constant for saturation tests
	
; NOTE: The gain control (nGain) allows for tweaking to get as many significant output
; bits through the entire gain path as possible. It is expected that the value will be
; tweaked based on the number of voices and how hot the signal needs to be leaving the
; synthesizer (in case it needs to be mixed with other audio). The gain multiplier
; should remain in the range of 0x400-0x7ff to insure maximum signal quality. If the
; gain cannot be scaled within that range, then adjustments should be made in the
; shifts to prevent numerical truncation in the results.


loop_start
	LDR		r6, [pnInputBuffer], #4			;get output sample
	MOV		r6, r6, ASR#7					;add some guard bits
	MULS	r4, r6, nGain					;output = gain * input
	MOV		r4, r4, ASR#9					;shift to low bits

; new saturation code to try, saves one cycle
;	MOV		r6, r4, ASR#15					;sign bit of 16-bit value
;	TEQ		r4, r6, ASR#31					;check against sign of 32-bit value
;	EORNE	r6, r5, r6, ASR#31				;extend sign to all bits
;	STRH	r6, [pnOutputBuffer], #NEXT_OUTPUT_PCM
	
	CMP		r4, r5							;check for positive saturation
	MOVGT	r4, r5							;saturate
	CMN		r4, r5							;check for negative saturation
	MVNLT	r4, r5							;saturate
	STRH	r4, [pnOutputBuffer], #NEXT_OUTPUT_PCM

; replace to here
	
	SUBS	nNumLoopSamples, nNumLoopSamples, #1
	BGT		loop_start

loop_done

;
; Return to calling function
;----------------------------------------------------------------

	LDMFD	sp!,SaveRegs				; return to calling function
	BX		lr

;*****************************************************************************

	ENDP		; end of function/procedure

	END		; end of assembly code

