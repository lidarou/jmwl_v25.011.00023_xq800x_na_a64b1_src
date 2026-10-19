#
# Auto-generated sample makefile
#
# This makefile is intended for use with GNU make.
# Set the paths to the tools (CC, AR, LD, etc.)
#

vpath %.c lib_src

CC = C:\Program Files\ARM\ADSv1_2\Bin\armcc.exe
AS = C:\Program Files\ARM\ADSv1_2\Bin\armasm.exe
LD = C:\Program Files\ARM\ADSv1_2\Bin\armlink.exe
AR = C:\Program Files\ARM\ADSv1_2\Bin\armar.exe

%.o: %.c
	$(CC) -c -fy -li -cpu ARM7TDMI -O1 -apcs /noswstackcheck/interwork -o $@ -Ilib_src -Ihost_src -DNUM_OUTPUT_CHANNELS=2 -D_SAMPLE_RATE_32000 -DMAX_SYNTH_VOICES=64 -DEAS_WT_SYNTH -D_8_BIT_SAMPLES -D_FILTER_ENABLED -D_XMF_PARSER -D_ENHANCER_ENABLED -DDLS_SYNTHESIZER -DMMAPI_SUPPORT -DNATIVE_EAS_KERNEL $<

%.o: %.s
	$(AS) -o $@ -apcs /noswst/interwork -li -cpu ARM7TDMI -I lib_src -predefine CHECK_STACK SETL {FALSE} -predefine REVERB SETL {FALSE} -predefine CHORUS SETL {FALSE} -predefine STEREO_OUTPUT SETL {TRUE} -predefine SAMPLE_RATE_32000 SETL {TRUE} -predefine SAMPLES_8_BIT SETL {TRUE} -predefine FILTER_ENABLED SETL {TRUE} $<

OBJS = eas_mididata.o eas_pan.o ARM_interpolate_noloop.o eas_xmf.o eas_smfdata.o eas_mdls.o ARM_interpolate_loop.o eas_math.o eas_enhancer.o eas_enhdata.o eas_voicemgt.o ARM_voice_gain.o eas_public.o eas_dlssynth.o eas_wtengine.o eas_midi.o ARM_filter.o ARM_enhancer.o eas_flog.o eas_mixbuf.o eas_tcdata.o eas_wtsynth.o eas_smf.o eas_tonecontrol.o eas_pcm.o eas_mixer.o 400kb_8b_32khz.o eas_xmfdata.o eas_pcmdata.o eas_data.o ARM_mastergain.o

arm7-400kb-8b-32khz.a: $(OBJS)
	$(AR) -create $@ $(OBJS)

