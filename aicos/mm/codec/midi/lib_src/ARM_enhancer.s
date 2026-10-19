;***********************************************************
; Function:    SynthEnhancer
; Processor:   ARM
; Description: Filter, non-linear distortion, filter, mix with dry.
;			   Applied to all samples on all channels (global).
;              C-callable.
;
; Useage: 
; EAS_VOID 
; SynthEnhancer(
;					EAS_I16 nNumMonoSamplesToEnhance,
;					EAS_PCM *pInput,
;					EAS_PCM *pOutput,
;					S_ENHANCER_OBJECT	*psEnhancer);
;
; Inputs: lots
;			
; Outputs:
; none
;
; Copyright Sonic Network 2003-2005
;****************************************************************
; Revision Control:
;   $Revision: 481 $
;   $Date: 2006-11-28 02:57:46 -0500 (Tue, 28 Nov 2006) $
;****************************************************************
;
;   where:
;	EAS_I16 nNumMonoSamplesToEnhance
;	PASSED IN: r0
;
;	EAS_PCM *pInput
;	PASSED IN: r1
;
;	EAS_PCM *pOutput
;	PASSED IN: r2
;
;	S_ENHANCER_OBJECT *psEnhancer
;	PASSED IN: r3
;
;****************************************************************

	INCLUDE		ARM_synth_constants.inc

	CODE32
	AREA |.text-core|, CODE, READONLY
	

SynthEnhancer PROC

	EXPORT		SynthEnhancer	; allow other files to use this function
	

;	.if	($isdefed("_16_BIT_SAMPLES"))
;	; assemble this portion if supporting _16_BIT_SAMPLES


; The output samples are 2 bytes each (16-bit PCM output samples)
; However, the output buffer is interleaved, L then R
; Each L sample is therefore 4 bytes apart, as is each R sample
; This implementation of asynthenhancer first processes all of the
; left samples, then all of the right samples, hence the samples to
; be processed are 4 bytes apart (instead of 2 bytes apart). This
; assumes a stereo output, of course!

	IF STEREO_OUTPUT = {TRUE}
NEXT_ENHANCER_SAMPLE		EQU	4
	ELSE
NEXT_ENHANCER_SAMPLE		EQU	2
	ENDIF

; Stack frame
; -----------
RET_ADDR_SZ			EQU	0			;return address
REG_SAVE_SZ			EQU	0			;save-on-entry registers saved
FRAME_SZ			EQU	32			;local variables
ARG_BLK_SZ			EQU	0			;argument block

PARAM_OFFSET			EQU	(ARG_BLK_SZ + FRAME_SZ + REG_SAVE_SZ + RET_ADDR_SZ)

; Local variables
; --------------      
psEnhancerCopy			EQU	0
nNumMonoSamplesToEnhanceCopy	EQU	4
pInputCopy			EQU	8
pOutputCopy			EQU	12
nTemp1				EQU	16
nTemp2				EQU	20
nWet				EQU	24
nDry				EQU	28

; Register usage
; --------------
nNumLoopSamples			RN	r0
pInput				RN	r1
pOutput				RN	r2
psEnhancer			RN	r3

nFeedForward1			RN	r4
nFeedback1			RN	r5
nDriveCoef			RN	r6
nFeedback2			RN	r7

;/* state variables (filter delays) */
zF0				RN	r11		;	/* filter 1 pole/zero state var */
zF1				RN	r12		;	/* filter 2 zero state var */
zF2				RN	r8

tmp0				RN	r3
tmp1				RN	r14
tmp2				RN	r9


;
; C-variables that we want to be able to access in the asm func
;----------------------------------------------------------------


	STMFD		sp!,{r4-r9,r11,lr}		; Save any save-on-entry registers that are used
	SUB		sp,sp,#PARAM_OFFSET		; Allocate the local frame and argument block	
							; 1xxx dont we need to ensure frame boundary is aligned to 2 words (8 bytes)?

;
; Save any local copies
;----------------------------------------------------------------

	STR		psEnhancer, [sp,#psEnhancerCopy]

	STR		nNumLoopSamples, [sp,#nNumMonoSamplesToEnhanceCopy]
	
	STR		pInput, [sp,#pInputCopy]

	STR		pOutput, [sp,#pOutputCopy]

;
; Setup passed parameters in their destination registers
;----------------------------------------------------------------

	; load the local registers with their values

	LDRSB		nFeedForward1, [psEnhancer, #m_nEnhancerFeedForward1]

	LDRSB		nFeedback1, [psEnhancer, #m_nEnhancerFeedback1]

	LDRSB		nDriveCoef, [psEnhancer, #m_nDriveCoef]

	LDRSB		nFeedback2, [psEnhancer, #m_nEnhancerFeedback2]

	LDRSB		tmp1, [psEnhancer, #m_nWet]

	STR		tmp1, [sp, #nWet]

	LDRSB		tmp1, [psEnhancer, #m_nDry]

	STR		tmp1, [sp, #nDry]

;
; Start processing, left channel
;----------------------------------------------------------------

	; so we only need to init once (before the loop) 
	; and save once (after the loop)
	LDRSH		zF0, [psEnhancer, #m_zF0L]
									; shift left to account for filter coef frac bits
									; we accumulate the result assuming inputs are in
									; 2.(15 + filter frac bits) format
									; only the zero state variable is in this format
	MOV		zF0, zF0, LSL #NUM_ENHANCER_FILTER_COEF_FRAC_BITS

	LDRSH		zF1, [psEnhancer, #m_zF1L]			; pole state variable remains in 1.15 format

	LDRSH		zF2, [psEnhancer, #m_zF2L]			; load the 16-bit value

	; loop N times means execute when counter == N, N-1, N-2, N-3, ..., 2, 1
	; stop executing when counter <= 0
	; decrement counter at end of loop
	; Therefore, we assume that N >= 1 (if N == 0, then we have a problem!)


loop_start_L

	LDRSH		tmp0, [pInput]					; tmp0 = xL[n]
									; *pInput = xL[n]

	;;SMULBB	tmp1, zF1, nFeedback1				; tmp1 = zF1 * fbk, result in 2.(15 + frac)
	MUL		tmp1, zF1, nFeedback1				; tmp1 = zF1 * fbk, result in 2.(15 + frac)
									; expect pole state var, zF1, in 1.15 format
;			/* apply feedforward */

	;;SMLABB	tmp1, tmp0, nFeedForward1, tmp1			; tmp1 += xL[n] * ffd, result in 2.(15 + frac)
	MUL		tmp2, tmp0, nFeedForward1			; tmp2 =  xL[n] * ffd, result in 2.(15 + frac)
	ADD		tmp1, tmp1, tmp2				; tmp1 = tmp1 + tmp2, result in 2.(15 + frac)

	SUB		tmp1, tmp1, zF0					; tmp1 = tmp1 - zF0, expect zF0 in 2.(15 + frac)
									;      == filtered result in 2.(15 + frac)

	; update zF0 (zero) state variable
	;;SMULBB	zF0, tmp0, nFeedForward1			; zF0 = xL[n] * ffd, in 2.(15 + frac)
	MUL		zF0, tmp0, nFeedForward1			; zF0 = xL[n] * ffd, in 2.(15 + frac)

	; update zF1 (pole) state variable
	MOV		zF1, tmp1, ASR #NUM_ENHANCER_FILTER_COEF_FRAC_BITS
									; shift right to account for filter coef frac bits
									; zF1 = filtered result in 1.15 format

	;**************************************************
;			/* apply distortion */
;			/*
;			Use a drive value for distortion:
;			distortion	=	2 * input * (drive+1)/2 - drive * input * |input|
;			=	(input * drive) + input - (drive * input) * |input|
;			=	(input * drive) * (1 - |input|) + input
;			*/
;
;			/* (1 - |input|)	*/
;			nTempSample = DISTORTION_ONE - abs(nOutputSample);

	MOV		tmp1, zF1

	CMP		tmp1, #0

	RSBLT		tmp1, zF1, #0					; if input < 0, then tmp1 = -input == |input|

	RSB		tmp1, tmp1, #0xff				; tmp1 =  0xff - |input|

	ADD		tmp1, tmp1, #0x7f00				; tmp1 += 0x7F
									;      == 0x7FFF - |input|
		
;			/* (input * drive)	*/
;			nTempSample2 = MULT_AUDIO_DRIVE(nOutputSample, gpsSynthObject->m_sEnhancer.m_nDriveCoef);


;			/* computed as SATURATE((input * drive) * (1 - |input|) + input)	*/

	; tmp1 contains (1-|input|)
	; zF1 contains input
	MUL		tmp1, zF1, tmp1					; tmp1 = input * (1-|input|), discard acc3
	MOV		tmp1, tmp1, ASR #16				; rescale result back to 1
	MOV		tmp0, #3					; load drive amount
	MUL		tmp1, tmp0, tmp1				; multiply by drive, tmp1 = drive * input * (1-|input|)
	ADD		tmp0, zF1, tmp1					; add input, tmp1 = (input * drive) * (1 - |input|) + input
	
	MOV		tmp2, #0x8000					; now pin lower 16 bits
	ORR		tmp2, tmp2, #0x0FF0000
	ORR		tmp2, tmp2, #0x0FF000000
    	CMP		tmp0, tmp2								
    	MOVLT		tmp0, tmp2
    	MOV		tmp2, #0x7F00
    	ORR		tmp2, tmp2, #0x00FF
    	CMP		tmp0, tmp2
    	MOVGT		tmp0, tmp2


	; Instead of multiplying (input * drive) and then saturating, lets instead FIX
	; the drive value to be a constant. Weve used a drive value of 3.0 and it seems
	; to have worked fine so far, so lets fix the drive value to 3 and use the Qxxx
	; instructions to more efficiently calculate saturation.
	; input * 3 == input + (2 * input)

	;MOV		tmp2, zF1, LSL #16				; put input top 16 bits so we can use Qxxx
									; tmp2 = zF1 == input, placed into top 16 bits

	;QDADD		tmp0, tmp2, tmp2				; tmp0 = sat(input + sat(2*input))


|DoneSat_L|								; at this point, tmp0 = SAT(input * drive)
;			/* (input * drive) * (1 - |input|) + input	*/
;			/* nOutputSample += MULT_AUDIO_AUDIO(nTempSample2, nTempSample); */
;			nTempSample = MULT_AUDIO_AUDIO(nTempSample2, nTempSample);

;	Because we now use QDADD to perform the saturation, the result is now in the TOP 16-bits
;	of tmp0, so SMULBB should become SMULTB
	;SMULTB		tmp1, tmp0, tmp1				; tmp1 = (input * drive) * (1 - |input|)

;			nOutputSample = nOutputSample + nTempSample;
;			nOutputSample = SATURATE(nOutputSample);

;	NOTE: because we now use qdadd to implement multiplication by 3 and then saturation,
;	we placed the input into MS portion of tmp2 already, so no need to do that again here.
	;QADD	tmp0, tmp1, tmp2					; tmp0 = (input * drive) * (1 - |input|) + input
	
	; Combine this ASR instruction with the normalization (multiplication by 0.5) in filter 2
	; but defer this calculation until after we start calculating the feedforward value in
	; filter 2
;;	MOV		tmp0, tmp0, ASR	#16				; tmp0 = tmp0 >> 16, put into LS portion
;	MOV		tmp0, tmp0, ASR	#(16+1)				; tmp0 = tmp0 >> (16+1), put into LS portion
;									;      == 0.5 * tmp0, placed in LS portion

	;**************************************************
;			/* apply filter 2 */
;			/*
;			NOTE: f2 = is different than f1 --
;			f2 is a zero pole, single zero filter
;			Also note: even though the coefficients name is 
;			"gnEnhancerFeedback2" the filter topology is really a
;			feedforward (not feedback) multiplication 
;			*/
;			/* apply feedforward */
;			nOutputSample = nOutputSample >> 1;

	; combine this normalization (multiplication by 0.5) with the ASR instruction in distortion section
;	MOV		tmp0, tmp0, ASR #1				; tmp0 = (x[n] * 0.5) (normalize)

;			nTempSample = MULT_FILTER_AUDIO_COEF( *pzF2, gpsSynthObject->m_sEnhancer.m_nEnhancerFeedback2);

	;;SMULBB	tmp1, zF2, nFeedback2				; tmp1 = zF2 * fbk (note: fbk is really a feedforward)
	MUL		tmp1, zF2, nFeedback2				; tmp1 = zF2 * fbk (note: fbk is really a feedforward)

	MOV		tmp1, tmp1, ASR #NUM_ENHANCER_FILTER_COEF_FRAC_BITS
									; shift right to account for filter coef frac bits
									; tmp0 = filtered result in 1.15 format

;			/* update filter state */
;			*pzF2 = nInput >> 1
	; This  calculation is the last step of the distortion, combined with the normalization step of filter 2,
	; and was deferred to this stage so could start calculating the feedforward value
	MOV		zF2, tmp0, ASR	#1				; zF2 = tmp0 * 0.5  (tmp0 is already in LS portion)
										
;			/* generate filter output */
;			nTempSample2 = nOutputSample - nTempSample;

	SUB		tmp1, zF2, tmp1					; tmp1 = (x[n] * 0.5) - (zF2 * fbk) == filtered result
									; in 1.15 format

;			/* apply wet,dry */
;			nOutputSample  = MULT_AUDIO_WET_DRY_COEF( nOutputSample, gpsSynthObject->m_sEnhancer.m_nWet);
;			nOutputSample += MULT_AUDIO_WET_DRY_COEF( nInputSample,  gpsSynthObject->m_sEnhancer.m_nDry);
;
;			*pOut = SATURATE(nOutputSample);
;			pOut += NUM_OUTPUT_CHANNELS;

	LDR		tmp2, [sp, #nWet]				; tmp2 = nWet

;	avoid pipeline hazard: multiply after next instruction
;	SMULBB		tmp1, tmp1, tmp2				; tmp1 = filtered out * nWet

	LDRSH	tmp0, [pInput], #NEXT_ENHANCER_SAMPLE			; tmp0 = xL[n]
									; *pInput = xL[n+1]

;	avoiding pipeline hazard, this multiply was deferred
	;;SMULBB	tmp1, tmp1, tmp2				; tmp1 = filtered out * nWet
	MUL		tmp1, tmp2, tmp1				; tmp1 = filtered out * nWet

	LDR		tmp2, [sp, #nDry]				; tmp2 = nDry

	;MOV		tmp1, tmp1, LSL #(0+16-7)			; shift right (-7) to account for wet/dry coef frac bits
									; shift left (+16) to put result into MS portion
	MOV		tmp1, tmp1, ASR #7				;shift right (7) to account for wet/dry coef frac bits

	;;SMULBB	tmp0, tmp0, tmp2				; tmp0 = input * nDry
	MUL		tmp0, tmp2, tmp0				; tmp0 = input * nDry

	;MOV		tmp0, tmp0, LSL #(0+16-7)			; shift right (-7) to account for wet/dry coef frac bits
									; shift left (+16) to put result into MS portion
	MOV		tmp0, tmp0, ASR #7				; shift right (7) to account for wet/dry coef frac bits

	;QADD		tmp0, tmp0, tmp1				; tmp0 = SATURATE(input * nDry + filtered out * nWet)
	ADD		tmp0, tmp0, tmp1				; tmp0 = input * nDry + filtered out * nWet

	;MOV		tmp0, tmp0, ASR #16				; tmp0 = wet + dry result, in LS portion
	
	MOV		tmp2, #0x8000					; now pin lower 16 bits
	ORR		tmp2, tmp2, #0xFF0000
	ORR		tmp2, tmp2, #0xFF000000
	CMP		tmp0, tmp2								
	MOVLT		tmp0, tmp2
	MOV		tmp2, #0x7F00
	ORR		tmp2, tmp2, #0x00FF
	CMP		tmp0, tmp2
	MOVGT		tmp0, tmp2

	STRH		tmp0, [pOutput], #NEXT_ENHANCER_SAMPLE		; yL[n] = tmp0
									; *pOutput = yL[n+1]

;loop_end_L
;	B		loop_test_L

loop_test_L
	SUBS		nNumLoopSamples, nNumLoopSamples, #1

	BGT		loop_start_L

loop_done_L

; Clean up and store any changes that were caused during the loop
;----------------------------------------------------------------

	; reset the psEnhancer pointer because we re-used its register during the filter calculations
	LDR		psEnhancer, [sp,#psEnhancerCopy]

	; update and store
									; shift right to account for filter coef frac bits
									; we accumulated the result assuming inputs are in
									; 2.(15 + filter frac bits) format
									; only the zero state variable is in this format
	MOV		zF0, zF0, ASR #NUM_ENHANCER_FILTER_COEF_FRAC_BITS

	STRH		zF0, [psEnhancer, #m_zF0L]

	STRH		zF1, [psEnhancer, #m_zF1L]

	STRH		zF2, [psEnhancer, #m_zF2L]

	IF STEREO_OUTPUT = {TRUE}
;**************************************
; start right side

	; reset the pointers as required for the right side
	LDR		psEnhancer, [sp,#psEnhancerCopy]

	LDR		nNumLoopSamples, [sp,#nNumMonoSamplesToEnhanceCopy]

	LDR		pInput, [sp,#pInputCopy]

	LDR		pOutput, [sp,#pOutputCopy]

	; advance pointers to the right sample
	ADD		pInput, pInput, #(NEXT_ENHANCER_SAMPLE/2)

	ADD		pOutput, pOutput, #(NEXT_ENHANCER_SAMPLE/2)

;
; Start processing, right channel
;----------------------------------------------------------------

	; so we only need to init once (before the loop) 
	; and save once (after the loop)
	LDRSH		zF0, [psEnhancer, #m_zF0R]
									; shift left to account for filter coef frac bits
									; we accumulate the result assuming inputs are in
									; 2.(15 + filter frac bits) format
									; only the zero state variable is in this format
	MOV		zF0, zF0, LSL #NUM_ENHANCER_FILTER_COEF_FRAC_BITS

	LDRSH		zF1, [psEnhancer, #m_zF1R]			; pole state variable remains in 1.15 format

	LDRSH		zF2, [psEnhancer, #m_zF2R]			; load the 16-bit value

	; loop N times means execute when counter == N, N-1, N-2, N-3, ..., 2, 1
	; stop executing when counter <= 0
	; decrement counter at end of loop
	; Therefore, we assume that N >= 1 (if N == 0, then we have a problem!)

;loop_test_R
;	SUBS		nNumLoopSamples, nNumLoopSamples, #1
;
;	BLT		loop_done_R

loop_start_R

	LDRSH		tmp0, [pInput]					; tmp0 = xL[n]
									; *pInput = xL[n]

	;;SMULBB	tmp1, zF1, nFeedback1				; tmp1 = zF1 * fbk, result in 2.(15 + frac)
	MUL		tmp1, zF1, nFeedback1				; tmp1 = zF1 * fbk, result in 2.(15 + frac)
									; expect pole state var, zF1, in 1.15 format
;			/* apply feedforward */

	;;SMLABB	tmp1, tmp0, nFeedForward1, tmp1			; tmp1 += xL[n] * ffd, result in 2.(15 + frac)
	MUL		tmp2, tmp0, nFeedForward1			; tmp2 =  xL[n] * ffd, result in 2.(15 + frac)
	ADD		tmp1, tmp1, tmp2				; tmp1 = tmp1 + tmp2, result in 2.(15 + frac)

	SUB		tmp1, tmp1, zF0					; tmp1 = tmp1 - zF0, expect zF0 in 2.(15 + frac)
									;      == filtered result in 2.(15 + frac)

	; update zF0 (zero) state variable
	;;SMULBB	zF0, tmp0, nFeedForward1			; zF0 = xL[n] * ffd, in 2.(15 + frac)
	MUL		zF0, tmp0, nFeedForward1			; zF0 = xL[n] * ffd, in 2.(15 + frac)

	; update zF1 (pole) state variable
	MOV		zF1, tmp1, ASR #NUM_ENHANCER_FILTER_COEF_FRAC_BITS
									; shift right to account for filter coef frac bits
									; zF1 = filtered result in 1.15 format

	;**************************************************
;			/* apply distortion */
;			/*
;			Use a drive value for distortion:
;			distortion	=	2 * input * (drive+1)/2 - drive * input * |input|
;			=	(input * drive) + input - (drive * input) * |input|
;			=	(input * drive) * (1 - |input|) + input
;			*/
;
;			/* (1 - |input|)	*/
;			nTempSample = DISTORTION_ONE - abs(nOutputSample);

	MOV		tmp1, zF1

	CMP		tmp1, #0

	RSBLT		tmp1, zF1, #0					; if input < 0, then tmp1 = -input == |input|

	RSB		tmp1, tmp1, #0xff				; tmp1 =  0xff - |input|

	ADD		tmp1, tmp1, #0x7f00				; tmp1 += 0x7F
									;      == 0x7FFF - |input|
		
;			/* (input * drive)	*/
;			nTempSample2 = MULT_AUDIO_DRIVE(nOutputSample, gpsSynthObject->m_sEnhancer.m_nDriveCoef);
	
;			/* computed as SATURATE((input * drive) * (1 - |input|) + input)	*/

	; tmp1 contains (1-|input|)
	; zF1 contains input
	MUL		tmp1, zF1, tmp1					; tmp1 = input * (1-|input|), discard acc3
	MOV		tmp1, tmp1, ASR #16				; rescale result back to 1
	MOV		tmp0, #3					; load drive amount
	MUL		tmp1, tmp0, tmp1				; multiply by drive, tmp1 = drive * input * (1-|input|)
	ADD		tmp0, zF1, tmp1					; add input, tmp1 = (input * drive) * (1 - |input|) + input
	
	MOV		tmp2, #0x8000					; now pin lower 16 bits
	ORR		tmp2, tmp2, #0xFF0000
	ORR		tmp2, tmp2, #0xFF000000
 	CMP		tmp0, tmp2								
	MOVLT		tmp0, tmp2
	MOV		tmp2, #0x7F00
	ORR		tmp2, tmp2, #0x00FF
	CMP		tmp0, tmp2
	MOVGT		tmp0, tmp2


	; Instead of multiplying (input * drive) and then saturating, lets instead FIX
	; the drive value to be a constant. Weve used a drive value of 3.0 and it seems
	; to have worked fine so far, so lets fix the drive value to 3 and use the Qxxx
	; instructions to more efficiently calculate saturation.
	; input * 3 == input + (2 * input)

	;MOV		tmp2, zF1, LSL #16				; put input top 16 bits so we can use Qxxx
									; tmp2 = zF1 == input, placed into top 16 bits

	;QDADD		tmp0, tmp2, tmp2				; tmp0 = sat(input + sat(2*input))


|DoneSat_R|								; at this point, tmp0 = SAT(input * drive)
;			/* (input * drive) * (1 - |input|) + input	*/
;			/* nOutputSample += MULT_AUDIO_AUDIO(nTempSample2, nTempSample); */
;			nTempSample = MULT_AUDIO_AUDIO(nTempSample2, nTempSample);

;	Because we now use QDADD to perform the saturation, the result is now in the TOP 16-bits
;	of tmp0, so SMULBB should become SMULTB
	;SMULTB		tmp1, tmp0, tmp1				; tmp1 = (input * drive) * (1 - |input|)

;			nOutputSample = nOutputSample + nTempSample;
;			nOutputSample = SATURATE(nOutputSample);

;	NOTE: because we now use qdadd to implement multiplication by 3 and then saturation,
;	we placed the input into MS portion of tmp2 already, so no need to do that again here.
	;QADD		tmp0, tmp1, tmp2				; tmp0 = (input * drive) * (1 - |input|) + input
	
	; Combine this ASR instruction with the normalization (multiplication by 0.5) in filter 2
	; but defer this calculation until after we start calculating the feedforward value in
	; filter 2
;;	MOV		tmp0, tmp0, ASR	#16				; tmp0 = tmp0 >> 16, put into LS portion
;	MOV		tmp0, tmp0, ASR	#(16+1)				; tmp0 = tmp0 >> (16+1), put into LS portion
;									;      == 0.5 * tmp0, placed in LS portion

	;**************************************************
;			/* apply filter 2 */
;			/*
;			NOTE: f2 = is different than f1 --
;			f2 is a zero pole, single zero filter
;			Also note: even though the coefficients name is 
;			"gnEnhancerFeedback2" the filter topology is really a
;			feedforward (not feedback) multiplication 
;			*/
;			/* apply feedforward */
;			nOutputSample = nOutputSample >> 1;

	; combine this normalization (multiplication by 0.5) with the ASR instruction in distortion section
;	MOV		tmp0, tmp0, ASR #1				; tmp0 = (x[n] * 0.5) (normalize)

;			nTempSample = MULT_FILTER_AUDIO_COEF( *pzF2, gpsSynthObject->m_sEnhancer.m_nEnhancerFeedback2);

	;;SMULBB	tmp1, zF2, nFeedback2				; tmp1 = zF2 * fbk (note: fbk is really a feedforward)
	MUL		tmp1, zF2, nFeedback2				; tmp1 = zF2 * fbk (note: fbk is really a feedforward)

	MOV		tmp1, tmp1, ASR #NUM_ENHANCER_FILTER_COEF_FRAC_BITS
									; shift right to account for filter coef frac bits
									; tmp0 = filtered result in 1.15 format

;			/* update filter state */
;			*pzF2 = nInput >> 1
	; This  calculation is the last step of the distortion, combined with the normalization step of filter 2,
	; and was deferred to this stage so could start calculating the feedforward value
	MOV		zF2, tmp0, ASR	#1				; zF2 = tmp0 * 0.5 (ass0 is already in LS portion)

;			/* generate filter output */
;			nTempSample2 = nOutputSample - nTempSample;

	SUB		tmp1, zF2, tmp1					; tmp1 = (x[n] * 0.5) - (zF2 * fbk) == filtered result
									; in 1.15 format

;			/* apply wet,dry */
;			nOutputSample  = MULT_AUDIO_WET_DRY_COEF( nOutputSample, gpsSynthObject->m_sEnhancer.m_nWet);
;			nOutputSample += MULT_AUDIO_WET_DRY_COEF( nInputSample,  gpsSynthObject->m_sEnhancer.m_nDry);
;
;			*pOut = SATURATE(nOutputSample);
;			pOut += NUM_OUTPUT_CHANNELS;

	LDR		tmp2, [sp, #nWet]				; tmp2 = nWet

;	avoid pipeline hazard: multiply after next instruction
;	SMULBB		tmp1, tmp1, tmp2				; tmp1 = filtered out * nWet

	LDRSH		tmp0, [pInput], #NEXT_ENHANCER_SAMPLE		; tmp0 = xL[n]
									; *pInput = xL[n+1]

;	avoiding pipeline hazard, this multiply was deferred
	;;SMULBB	tmp1, tmp1, tmp2				; tmp1 = filtered out * nWet
	MUL		tmp1, tmp2, tmp1				; tmp1 = filtered out * nWet

	LDR		tmp2, [sp, #nDry]				; tmp2 = nDry

	;MOV		tmp1, tmp1, LSL #(0+16-7)			; shift right (-7) to account for wet/dry coef frac bits
									; shift left (+16) to put result into MS portion
	MOV		tmp1, tmp1, ASR #7				; shift right (7) to account for wet/dry coef frac bits

	;;SMULBB	tmp0, tmp0, tmp2				; tmp0 = input * nDry
	MUL		tmp0, tmp2, tmp0				; tmp0 = input * nDry

	;MOV		tmp0, tmp0, LSL #(0+16-7)			; shift right (-7) to account for wet/dry coef frac bits
									; shift left (+16) to put result into MS portion
	MOV		tmp0, tmp0, ASR #7				; shift right (7) to account for wet/dry coef frac bits

	;QADD		tmp0, tmp0, tmp1				; tmp0 = input * nDry + filtered out * nWet
	ADD		tmp0, tmp0, tmp1				; tmp0 = input * nDry + filtered out * nWet

	;MOV		tmp0, tmp0, ASR #16				; tmp0 = wet + dry result, in LS portion
	
	MOV		tmp2, #0x8000					; now pin lower 16 bits
	ORR		tmp2, tmp2, #0xFF0000
	ORR		tmp2, tmp2, #0xFF000000
	CMP		tmp0, tmp2								
	MOVLT		tmp0, tmp2
	MOV		tmp2, #0x7F00
	ORR		tmp2, tmp2, #0x00FF
	CMP		tmp0, tmp2
	MOVGT		tmp0, tmp2

	STRH		tmp0, [pOutput], #NEXT_ENHANCER_SAMPLE		; yL[n] = tmp0
									; *pOutput = yL[n+1]

;loop_end_R
;	B		loop_test_R

loop_test
	SUBS		nNumLoopSamples, nNumLoopSamples, #1

	BGT		loop_start_R

loop_done_R

; Clean up and store any changes that were caused during the loop
;----------------------------------------------------------------

	; reset the psEnhancer pointer because we re-used its register during the filter calculations
	LDR		psEnhancer, [sp,#psEnhancerCopy]

	; update and store
									; shift right to account for filter coef frac bits
									; we accumulated the result assuming inputs are in
									; 2.(15 + filter frac bits) format
									; only the zero state variable is in this format
	MOV		zF0, zF0, ASR #NUM_ENHANCER_FILTER_COEF_FRAC_BITS

	STRH		zF0, [psEnhancer, #m_zF0R]

	STRH		zF1, [psEnhancer, #m_zF1R]

	STRH		zF2, [psEnhancer, #m_zF2R]

; done right side
;**************************************
	ENDIF	; IF STEREO_OUTPUT = {TRUE}
	
;
; Return to calling function
;----------------------------------------------------------------

	ADD		sp,sp,#PARAM_OFFSET				; De-allocate the local frame and argument block

	LDMFD		sp!,{r4-r9,r11,lr}				; return to calling function
	BX			lr

;	.endif		; .if	($isdefed("_16_BIT_SAMPLES"))

;*****************************************************************************

PlusOne		DCD			0x00007fff
MinusOne	DCD			0xffff8000

	ENDP			; end of function/procedure

	END			; end of assembly code
