#
# Auto-generated sample makefile
#
# This makefile is intended for use with GNU make.
# Set the paths to the tools (CC, AR, LD, etc.)
#

vpath %.c host_src

CC = C:\Program Files\ARM\ADSv1_2\Bin\armcc.exe
AS = C:\Program Files\ARM\ADSv1_2\Bin\armasm.exe
LD = C:\Program Files\ARM\ADSv1_2\Bin\armlink.exe
AR = C:\Program Files\ARM\ADSv1_2\Bin\armar.exe

%.o: %.c
	$(CC) -c -fy -li -cpu ARM7TDMI -O1 -apcs /noswstackcheck/interwork -o $@ -Ihost_src -DUNIFIED_DEBUG_MESSAGES -DEAS_WT_SYNTH -D_XMF_PARSER -D_ENHANCER_ENABLED -DMMAPI_SUPPORT $<

%.o: %.s
	$(AS) -o $@ -apcs /noswst/interwork -li -cpu ARM7TDMI -I lib_src -predefine CHECK_STACK SETL {FALSE} -predefine REVERB SETL {FALSE} -predefine CHORUS SETL {FALSE} -predefine STEREO_OUTPUT SETL {TRUE} -predefine SAMPLE_RATE_32000 SETL {TRUE} -predefine SAMPLES_8_BIT SETL {TRUE} -predefine FILTER_ENABLED SETL {TRUE} $<

OBJS = eas_main.o eas_report.o eas_wave.o eas_hostmm.o eas_config.o

arm7-400kb-8b-32khz.axf: $(OBJS)
	$(LD) -o $@ $(OBJS) arm7-400kb-8b-32khz.a

